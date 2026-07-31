# Makefile

ROMNAME = rom.nds
BUILDROM = test.nds

define n


endef

# Check if user cloned git repository correctly first thing to prevent excessive user enquiries
# add an exception for the path to hg-engine that is normally in the docker container
ifneq ($(shell pwd),/hg-engine)
ifneq ($(shell git rev-parse --is-inside-work-tree 2>/dev/null), true)
$(error Current directory is not a git repository. Please follow the instructions in the README: https://github.com/BluRosie/hg-engine)
endif
endif

ifneq ($(shell pwd | grep -i 'onedrive'),)
$(error "Do not put files into OneDrive.  Please clone the repository in a different folder." )
endif

DESIRED_GAMECODE := IPKE
GAMECODE = $(shell dd bs=1 skip=12 count=4 if=$(ROMNAME) status=none)
VALID_GAMECODE = $(shell echo $(GAMECODE) | grep -i -q $(DESIRED_GAMECODE); echo $$?)

ifneq ($(VALID_GAMECODE), 0)
# invalid rom detected based on gamecode.  this primarily catches other-language roms
$(error ROM Code read from $(ROMNAME) ($(GAMECODE)) does not match valid ROM Code ($(DESIRED_GAMECODE)).$(n)Please use a valid US HeartGold ROM.$(n)hg-engine does not work with non-USA ROM files)
endif

MAC = $(shell uname -s | grep -i -q 'darwin'; echo $$?)

ifneq ($(MAC), 0)
# Current MSYS2 identifies itself as MINGW*_NT in uname; older releases used
# MSYS_NT. Check uname instead of /proc/version so both forms are supported.
MSYS2 = $(shell uname -s | grep -E -i -q 'msys|mingw|cygwin'; echo $$?)
else
MSYS2 = 1
endif

# environment setting
# get rid of devkitpro:  if devkitpro is installed, can still use it.  otherwise, default to arm-none-eabi tools
ifeq ($(shell echo $$DEVKITARM),)
ifeq ($(MSYS2), 0)
# Current MSYS2 installs the ARM embedded toolchain into the active MinGW
# environment (normally /ucrt64). Fall back to /mingw64 for legacy shells.
MSYS2_MINGW_PREFIX = $(if $(MINGW_PREFIX),$(MINGW_PREFIX),/mingw64)
PREFIX = $(MSYS2_MINGW_PREFIX)/bin/arm-none-eabi-
AS = $(PREFIX)as
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
else
PREFIX = arm-none-eabi-
AS = $(PREFIX)as
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
endif
else
# support legacy devkitpro instructions
PREFIX = bin/arm-none-eabi-
AS = $(DEVKITARM)/$(PREFIX)as
CC = $(DEVKITARM)/$(PREFIX)gcc
LD = $(DEVKITARM)/$(PREFIX)ld
OBJCOPY = $(DEVKITARM)/$(PREFIX)objcopy
endif

# Use ccache automatically when it is installed. Builds fall back to the
# compiler directly when ccache is unavailable.
CCACHE := $(shell command -v ccache 2>/dev/null)
ifneq ($(CCACHE),)
CC := $(CCACHE) $(CC)
endif

PYTHON_NO_VENV = python3
VENV = .venv
PYTHON_VENV_VERSION := $(shell $(PYTHON_NO_VENV) -m ensurepip 2>&1 | grep -i -q 'No module named'; echo $$?)

ifneq ($(PYTHON_VENV_VERSION), 0)
# we can use a virtual environment because ensurepip is packaged with the python install
VENV_ACTIVATE = $(VENV)/bin/activate
PYTHON = . $(VENV_ACTIVATE); $(PYTHON_NO_VENV)
REQUIREMENTS = requirements.txt
else
# there is no need to use a virtual environment because python does not have the requirements installed
PYTHON = $(PYTHON_NO_VENV)
VENV_ACTIVATE =
endif

.PHONY: clean all code dumprom full-rom move_narc quick-rom refresh_base_code

move_narc clean restore: NOSCAN = 1

NOSCAN ?= 0


default: all

ifneq ($(PYTHON_VENV_VERSION), 0)
# only set up venv if we need to
venv: $(VENV_ACTIVATE)

# divorce this python3 from venv so that it works
$(VENV_ACTIVATE):
	$(PYTHON_NO_VENV) -m venv $(VENV)
ifeq ($(MSYS2), 0)
	$(PYTHON) -m pip install ndspy==4.1.0
else
	$(PYTHON) -m pip install -r $(REQUIREMENTS)
endif

endif

####################### Tools #######################
ifeq ($(MSYS2), 0)
HOST_EXE := .exe
else
HOST_EXE :=
endif

ADPCMXQ := tools/adpcm-xq$(HOST_EXE)
ARMIPS := tools/armips$(HOST_EXE)
BLZ := tools/blz$(HOST_EXE)
BTX := tools/btx$(HOST_EXE)
ENCODEPWIMG := tools/ENCODE_IMG$(HOST_EXE)
GFX := tools/nitrogfx$(HOST_EXE)
MSGENC := tools/msgenc$(HOST_EXE)
MOVEDATAGEN := tools/movedatagen$(HOST_EXE)
POKEDEXDATAGEN := tools/pokedexdatagen$(HOST_EXE)
SPECIESDATAGEN := tools/speciesdatagen$(HOST_EXE)
TRAINERDATAGEN := tools/trainerdatagen$(HOST_EXE)
NARCHIVE := $(PYTHON) tools/narcpy.py
NDSTOOL := tools/ndstool$(HOST_EXE)
NTRWAVTOOL := $(PYTHON) tools/ntrWavTool.py
O2NARC := tools/o2narc$(HOST_EXE)
SDATTOOL := $(PYTHON) tools/SDATTool.py

# Compiler/Assembler/Linker settings
LDFLAGS = rom.ld -T $(C_SUBDIR)/linker.ld
ASFLAGS = -mthumb
CFLAGS = -mthumb -mno-thumb-interwork -mcpu=arm7tdmi -mtune=arm7tdmi -mno-long-calls -march=armv4t -Wall -Wextra -Wno-builtin-declaration-mismatch -Wno-sequence-point -Wno-address-of-packed-member -Os -fira-loop-pressure -fipa-pta
ARMIPS_FLAGS = -equ DEBUG_BATTLE_SCENARIOS 0

ifeq ($(AUTO_TEST),Y)
    CFLAGS += -DDEBUG_BATTLE_SCENARIOS -DDEBUG_AUTO_CONTINUE_GAME
    ARMIPS_FLAGS = -equ DEBUG_BATTLE_SCENARIOS 1
endif

####################### Output #######################
C_SUBDIR = src
ASM_SUBDIR = asm
INCLUDE_SUBDIR = include
BUILD := build
BUILD_NARC := $(BUILD)/narc
BASE := base
FILESYS := $(BASE)/root
BASE_EXTRACTION_STAMP := $(BASE)/.extracted
ARMIPS_CONFIG := $(BUILD)/armips_config.s
ARMIPS_CONFIG_GENERATOR := tools/generate_armips_config.py

LINK = $(BUILD)/linked.o
OUTPUT = $(BUILD)/output.bin

INCLUDE_SRCS := $(wildcard $(INCLUDE_SUBDIR)/*.h)

C_SRCS := $(wildcard $(C_SUBDIR)/*.c)
ALL_C_SRCS += $(C_SRCS)
C_OBJS := $(patsubst $(C_SUBDIR)/%.c,$(BUILD)/%.o,$(C_SRCS))

ASM_SRCS := $(wildcard $(ASM_SUBDIR)/*.s)
ALL_ASM_SRCS += $(ASM_SRCS)
ASM_OBJS := $(patsubst $(ASM_SUBDIR)/%.s,$(BUILD)/%.o,$(ASM_SRCS))
OBJS     := $(C_OBJS) $(ASM_OBJS)

REQUIRED_DIRECTORIES += $(BASE) $(BUILD) $(BUILD_NARC)

$(ARMIPS_CONFIG): include/config.h $(ARMIPS_CONFIG_GENERATOR) $(VENV_ACTIVATE)
	@mkdir -p $(dir $@)
	$(PYTHON) $(ARMIPS_CONFIG_GENERATOR) $< $@


## includes
include data/graphics/pokegra.mk
include data/graphics/itemgra.mk
include data/itemdata/itemdata.mk
include data/codetables.mk
include narcs.mk
include overlays.mk
include dump.mk

####################### Build Tools #######################
MSGENC_SOURCES := $(wildcard tools/source/msgenc/*.cpp) $(wildcard tools/source/msgenc/*.h)
$(MSGENC): $(MSGENC_SOURCES)
	cd tools/source/msgenc ; $(MAKE)
	mv tools/source/msgenc/msgenc $(MSGENC)

TOOLS += $(MSGENC)

$(NDSTOOL):
ifeq (,$(wildcard $(NDSTOOL)))
	rm -r -f tools/source/ndstool
	cd tools/source ; git clone https://github.com/devkitPro/ndstool.git
	cd tools/source/ndstool ; git checkout fa6b6d01881363eb2cd6e31d794f51440791f336
	@# do not need to account for subdirectories here because ndstool does not have any .sh in subdirs
	cd tools/source/ndstool ; chmod +x *.sh
	cd tools/source/ndstool ; ./autogen.sh
	cd tools/source/ndstool ; ./configure && $(MAKE)
	mv tools/source/ndstool/ndstool $(NDSTOOL)
	rm -r -f tools/source/ndstool
endif

TOOLS += $(NDSTOOL)

$(ARMIPS):
ifeq (,$(wildcard $(ARMIPS)))
	rm -r -f tools/source/armips
	cd tools/source ; git clone --recursive https://github.com/BluRosie/armips.git
	mkdir -p tools/source/armips/build
	cd tools/source/armips/build; cmake .. -DCMAKE_BUILD_TYPE=Release ..
	cmake --build tools/source/armips/build --config Release --target armips-bin --parallel
ifeq ($(MSYS2), 0)
	mv tools/source/armips/build/armips.exe $(ARMIPS)
else
	mv tools/source/armips/build/armips $(ARMIPS)
endif
	rm -r -f tools/source/armips
endif

TOOLS += $(ARMIPS)

$(ADPCMXQ):
ifeq (,$(wildcard $(ADPCMXQ)))
	rm -r -f tools/source/adpcm-xq
	cd tools/source ; git clone https://github.com/dbry/adpcm-xq.git
	cd tools/source/adpcm-xq ; gcc -O2 *.c -o adpcm-xq -lm
	mv tools/source/adpcm-xq/adpcm-xq $(ADPCMXQ)
	rm -r -f tools/source/adpcm-xq
endif

TOOLS += $(ADPCMXQ)

tools/ntrWavTool.py:
ifeq (,$(wildcard tools/ntrWavTool.py))
	rm -r -f tools/source/ntrWavTool
	cd tools/source ; git clone https://github.com/turtleisaac/ntrWavTool.git
	mv tools/source/ntrWavTool/ntrWavTool.py tools/ntrWavTool.py
	rm -r -f tools/source/ntrWavTool
endif

TOOLS += tools/ntrWavTool.py

NITROGFX_SOURCES := $(wildcard tools/source/nitrogfx/*.c) $(wildcard tools/source/nitrogfx/*.h)
$(GFX): $(NITROGFX_SOURCES)
	cd tools/source/nitrogfx ; $(MAKE)
	mv tools/source/nitrogfx/nitrogfx $(GFX)

TOOLS += $(GFX)

$(MOVEDATAGEN): $(wildcard tools/source/movedatagen/*.c) data/Moves.c include/move_data.h include/data_config.h
	cd tools/source/movedatagen ; $(MAKE)

TOOLS += $(MOVEDATAGEN)

$(POKEDEXDATAGEN): $(wildcard tools/source/pokedexdatagen/*.c) data/PokedexSort.c data/PokedexArea.c include/pokedex_archive_data.h include/constants/pokedex.h
	cd tools/source/pokedexdatagen ; $(MAKE)

TOOLS += $(POKEDEXDATAGEN)

$(SPECIESDATAGEN): $(wildcard tools/source/speciesdatagen/*.c) data/Species.c include/species_data.h include/data_config.h
	cd tools/source/speciesdatagen ; $(MAKE)

TOOLS += $(SPECIESDATAGEN)

$(TRAINERDATAGEN): $(wildcard tools/source/trainerdatagen/*.c) data/Trainers.c include/trainer_data.h include/constants/trainerclass.h include/constants/pokemon.h
	cd tools/source/trainerdatagen ; $(MAKE)

TOOLS += $(TRAINERDATAGEN)

$(O2NARC): $(wildcard tools/source/o2narc/*.cpp) $(wildcard tools/source/o2narc/*.h)
	cd tools/source/o2narc ; $(MAKE)
	mv tools/source/o2narc/o2narc $(O2NARC)

TOOLS += $(O2NARC)

$(ENCODEPWIMG):
	cd tools/source/DECODEIMG ; $(MAKE)
	mv tools/source/DECODEIMG/ENCODE_IMG $(ENCODEPWIMG)

TOOLS += $(ENCODEPWIMG)

$(BTX):
	cd tools/source/btx ; $(MAKE)
	mv tools/source/btx/btx $(BTX)

TOOLS += $(BTX)

####################### Build #######################
$(BUILD)/rom_gen.ld:$(LINK) $(OUTPUT) rom.ld
	cp rom.ld $(BUILD)/rom_gen.ld
	$(PYTHON) scripts/generate_ld.py $(BUILD)/rom_gen.ld $(LINK)

# create output folders if they do not exist
$(CODE_BUILD_DIRS):
	mkdir -p $@

# generate .d dependency files that are included as part of compiling if it does not exist
define SRC_OBJ_INC_DEFINE
# this generates the objects as part of generating the dependency list which will just be massive files of rules
$1: $2 | $(dir $1)
	$(CC) -MMD -MF $(basename $1).d $(CFLAGS) -c $2 -o $1
	@#printf "\t$(CC) $(CFLAGS) -c $2 -o $1" >> $(basename $1).d

-include $(basename $1).d
endef

ifneq (1,$(NOSCAN))
$(foreach src, $(ALL_C_SRCS), $(eval $(call SRC_OBJ_INC_DEFINE,$(patsubst $(C_SUBDIR)/%.c,$(BUILD)/%.o, $(src)),$(src))))
endif

# Generated headers only invalidate the source files that include them.
$(BUILD)/field/move_tutor.o \
$(BUILD)/field/script_commands.o \
$(BUILD)/individual/PartyMenu_HandleUseItemOnMon.o \
$(BUILD)/pokemon.o: $(LEARNSETS_HEADER)

$(BUILD)/test_battle.o: $(BATTLETESTS_HEADER)

define ASM_OBJ_INC_DEFINE
# these should have similar dependency scanning, but we do not currently use them in a way conducive to it
$1: $2 | $(dir $1)
	$(AS) $(ASFLAGS) -c $2 -o $1
endef

ifneq (1,$(NOSCAN))
$(foreach src, $(ALL_ASM_SRCS), $(eval $(call ASM_OBJ_INC_DEFINE,$(patsubst $(ASM_SUBDIR)/%.s,$(BUILD)/%.o, $(src)),$(src))))
endif

$(LINK):$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(OUTPUT):$(LINK)
	$(OBJCOPY) -O binary $< $@

# Only reextract when the source ROM changes. The separate stamp is not
# modified by later executable refreshes, so it is safe for dependency checks.
$(BASE_EXTRACTION_STAMP): $(ROMNAME) $(NDSTOOL) $(VENV_ACTIVATE)
	rm -rf $(BASE)
	@mkdir -p $(REQUIRED_DIRECTORIES)
	$(NDSTOOL) -x $(ROMNAME) -9 $(BASE)/arm9.bin -7 $(BASE)/arm7.bin -y9 $(BASE)/overarm9.bin -y7 $(BASE)/overarm7.bin -d $(FILESYS) -y $(BASE)/overlay -t $(BASE)/banner.bin -h $(BASE)/header.bin
	$(NARCHIVE) extract $(FILESYS)/a/0/2/8 -o $(BUILD)/a028/ -nf
	@touch $@

# Binary patches are conditional and are not all reversible. In particular, an
# AUTO_TEST build leaves debug hooks behind if a later normal build patches the
# same extracted files in place. Restore the executable inputs from rom.nds
# before every patch pass while preserving the generated NitroFS in base/root.
refresh_base_code: $(BASE_EXTRACTION_STAMP) $(NDSTOOL)
	rm -rf $(BASE)/overlay
	@mkdir -p $(BASE)/overlay
	$(NDSTOOL) -x $(ROMNAME) -9 $(BASE)/arm9.bin -y9 $(BASE)/overarm9.bin -y $(BASE)/overlay

all: $(BUILDROM)

# Compile and link injected code without modifying base/ or packaging a ROM.
code: $(OUTPUT) $(OVERLAY_OUTPUTS)

# This named entry point uses the safe incremental graph. It never skips stale
# data; code-only changes naturally take the short path.
quick-rom full-rom: $(BUILDROM)


####################### Restore clean base ################
NEWFILE = romOld-`date +%d%b%y`.nds
CLEANROM = romClean.nds
restore:
	mv $(ROMNAME) $(NEWFILE)
	cp $(CLEANROM) $(ROMNAME)

####################### Restore and build ################
restore_build: | restore all

####################### Clean #######################
clean:
	rm -rf $(BUILD) $(BASE) $(BUILD)/rom_gen.ld $(BUILD)/rom_gen_battle.ld
	rm -rf $(shell find . -type d -name "generated")
	@echo "Build artifacts removed."

clean_tools:
	rm -rf $(TOOLS) $(VENV)

ALL_CODE_OBJS := $(patsubst $(C_SUBDIR)/%.c,$(BUILD)/%.o,$(ALL_C_SRCS)) \
 $(patsubst $(ASM_SUBDIR)/%.s,$(BUILD)/%.o,$(ALL_ASM_SRCS)) \
 $(patsubst $(C_SUBDIR)/%.c,$(BUILD)/%.d,$(ALL_C_SRCS))

clean_code:
	rm -f $(ALL_CODE_OBJS) $(LINKED_OUTPUTS) $(OUTPUT) $(OVERLAY_OUTPUTS) $(BUILD)/rom_gen.ld $(BUILD)/rom_gen_battle.ld

####################### Final ROM Build #######################
CODE_ADDON_ARTIFACTS := $(wildcard $(BUILD)/a028/9_*) $(wildcard $(BUILD)/a028/8_1*) $(wildcard build/$(BUILD)/8_2*) $(BUILD)/a028/8_07 $(BUILD)/a028/8_08 $(BUILD)/a028/8_09
CODE_ADDON_ARTIFACTS := $(filter-out $(BUILD)/a028/8_1 $(BUILD)/a028/8_2 $(BUILD)/a028/8_3 $(BUILD)/a028/8_4 $(BUILD)/a028/8_5 $(BUILD)/a028/8_6, $(CODE_ADDON_ARTIFACTS))
DATA_INSTALL_STAMP := $(BUILD)/.data_installed

move_narc $(DATA_INSTALL_STAMP): $(NARC_FILES) $(BASE_EXTRACTION_STAMP)
	@echo "battle hud layout:"
	cp $(BATTLEHUD_NARC) $(BATTLEHUD_TARGET)

	@echo "move data:"
	cp $(MOVEDATA_NARC) $(MOVEDATA_TARGET)

	@echo "move particles:"
	cp $(MOVEPARTICLES_NARC) $(MOVEPARTICLES_TARGET)

	@echo "item data files:"
	cp $(ITEMDATA_NARC) $(ITEMDATA_TARGET)

	@echo "mon sprite data:"
	cp $(POKEGRA_NARC) $(POKEGRA_TARGET)
	cp $(POKEGRA_NARC) $(PBR_POKEGRA_TARGET)

	@echo "opening demo files:"
	cp $(OPENDEMO_NARC) $(OPENDEMO_TARGET)

	@echo "mon personal data:"
	cp $(MONDATA_NARC) $(MONDATA_TARGET)

	@echo "sprite offsets:"
	cp $(SPRITEOFFSETS_NARC) $(SPRITEOFFSETS_TARGET)

	@echo "mon height offsets (a005):"
	cp $(HEIGHT_NARC) $(HEIGHT_TARGET)

	@echo "dex area data:"
	cp $(DEXAREA_NARC) $(DEXAREA_TARGET)

	@echo "pokedex sort lists:"
	cp $(DEXSORT_NARC) $(DEXSORT_TARGET)

	@echo "evolution data:"
	cp $(EVOS_NARC) $(EVOS_TARGET)

	@echo "regional dex order:"
	cp $(REGIONALDEX_NARC) $(REGIONALDEX_TARGET)

	@echo "trainer data:"
	cp $(TRAINERDATA_NARC) $(TRAINERDATA_TARGET)
	cp $(TRAINERDATA_NARC_2) $(TRAINERDATA_TARGET_2)

	@echo "trainer text:"
	cp $(TRAINERTEXT_NARC) $(TRAINERTEXT_TARGET)
	cp $(TRAINERTEXT_NARC_2) $(TRAINERTEXT_TARGET_2)

	@echo "footprints:"
	cp $(FOOTPRINTS_NARC) $(FOOTPRINTS_TARGET)

	@echo "move anims:"
	cp $(MOVEANIM_NARC) $(MOVEANIM_TARGET)

	@echo "move sub animations:"
	cp $(MOVESUBANIM_NARC) $(MOVESUBANIM_TARGET)

	@echo "move battle scripts:"
	cp $(MOVE_SEQ_NARC) $(MOVE_SEQ_TARGET)

	@echo "move battle scripts:"
	cp $(BATTLE_EFF_NARC) $(BATTLE_EFF_TARGET)

	@echo "battle sub effects:"
	cp $(BATTLE_SUB_NARC) $(BATTLE_SUB_TARGET)

	@echo "bag gfx:"
	cp $(BAGGFX_NARC) $(BAGGFX_TARGET)

	@echo "item gfx:"
	cp $(ITEMGFX_NARC) $(ITEMGFX_TARGET)

	@echo "dex gfx for fairy:"
	cp $(DEXGFX_NARC) $(DEXGFX_TARGET)

	@echo "battle gfx for fairy:"
	cp $(BATTLEGFX_NARC) $(BATTLEGFX_TARGET)

	@echo "otherpoke gfx for fairy:"
	cp $(OTHERPOKE_NARC) $(OTHERPOKE_TARGET)

	@echo "pokemon icons:"
	cp $(ICONGFX_NARC) $(ICONGFX_TARGET)

	@echo "wild encounters:"
	cp $(ENCOUNTER_NARC) $(ENCOUNTER_TARGET)

	@echo "safari zone encounters:"
	cp $(SAFARI_ENCOUNTER_NARC) $(SAFARI_ENCOUNTER_TARGET)

	@echo "pokemon overworlds:"
	cp $(OVERWORLDS_NARC) $(OVERWORLDS_TARGET)

	@echo "pokemon overworld data:"
	cp $(OVERWORLD_DATA_NARC) $(OVERWORLD_DATA_TARGET)

	@echo "move an updated gs_sound_data.sdat:"
	cp $(SDAT_BUILD) $(SDAT_TARGET)

	@echo "text data:"
	cp $(MSGDATA_NARC) $(MSGDATA_TARGET)

	@echo "ball spa files:"
	cp $(BALL_SPA_NARC) $(BALL_SPA_TARGET)

	@echo "pokewalker sprites:"
	cp $(PW_POKEGRA_NARC) $(PW_POKEGRA_TARGET)

	@echo "pokewalker icons:"
	cp $(PW_POKEICON_NARC) $(PW_POKEICON_TARGET)

	@echo "font:"
	if [ $$(grep -i -c "//#define IMPLEMENT_TRANSPARENT_TEXTBOXES" $(INCLUDE_SUBDIR)/config.h) -eq 0 ]; then cp $(FONT_NARC) $(FONT_TARGET); fi

	@echo "textbox:"
	if [ $$(grep -i -c "//#define IMPLEMENT_TRANSPARENT_TEXTBOXES" $(INCLUDE_SUBDIR)/config.h) -eq 0 ]; then cp $(TEXTBOX_NARC) $(TEXTBOX_TARGET); fi

	@echo "scripts:"
	cp $(SCR_SEQ_NARC) $(SCR_SEQ_TARGET)

	@echo "headbutt trees:"
	cp $(HEADBUTT_NARC) $(HEADBUTT_TARGET)

	@echo "trainer gfx:"
	cp $(TRAINER_GFX_NARC) $(TRAINER_GFX_TARGET)

	@echo "trainer back gfx:"
	cp $(TRAINER_GFX_BACK_NARC) $(TRAINER_GFX_BACK_TARGET)

	@echo "levelup learnset:"
	cp $(LEVELUPLEARNSET_NARC) $(LEVELUPLEARNSET_TARGET)

	@echo "egg moves:"
	cp $(EGGLEARNSET_NARC) $(EGGLEARNSET_TARGET)



	@echo "baby mons:"
	cp $(BABYMONS_BIN) $(BABYMONS_TARGET)

	@if test -s build/a028/8_00; then \
		rm -rf build/a028/8_0 build/a028/8_1 build/a028/8_2 build/a028/8_3 build/a028/8_4 build/a028/8_5 build/a028/8_6 build/a028/8_7 build/a028/8_8 build/a028/8_9; \
	fi
	@if test -s build/a028/8_7; then \
		rm -rf build/a028/8_7 build/a028/8_8 build/a028/8_9; \
	fi
	@rm -rf $(CODE_ADDON_ARTIFACTS)

	@echo "hidden ability table:"
	cp $(HIDDEN_ABILITY_TABLE_BIN) $(HIDDEN_ABILITY_TABLE_TARGET)

	@echo "base experience table:"
	cp $(BASE_EXPERIENCE_TABLE_BIN) $(BASE_EXPERIENCE_TABLE_TARGET)

	@echo "icon palette table:"
	cp $(ICON_PALETTE_TABLE_BIN) $(ICON_PALETTE_TABLE_TARGET)

	@echo "species to ow female table:"
	cp $(SPECIES_TO_OW_FEMALE_BIN) $(SPECIES_TO_OW_FEMALE_TARGET)

	@echo "form data table:"
	cp $(POKEFORMDATATBL_BIN) $(POKEFORMDATATBL_TARGET)

	@echo "form to species mapping table:"
	cp $(FORMTOSPECIES_BIN) $(FORMTOSPECIES_TARGET)

	@echo "form reversion mapping table:"
	cp $(FORMREVERSION_BIN) $(FORMREVERSION_TARGET)

	@echo "machine moves:"
	cp $(MACHINELEARNSET_BIN) $(MACHINELEARNSET_TARGET)

	@echo "tutor moves:"
	cp $(TUTORLEARNSET_BIN) $(TUTORLEARNSET_TARGET)

	@echo "battle tests:"
	cp $(BATTLETESTS_BIN) $(BATTLETESTS_TARGET)

	@echo "background gfx ids:"
	cp $(BACKGROUND_GFX_IDS_BIN) $(BACKGROUND_GFX_IDS_TARGET)

	@echo "hidden item params:"
	cp $(HIDDEN_ITEM_PARAMS_BIN) $(HIDDEN_ITEM_PARAMS_TARGET)

	@echo "level cap rewards:"
	cp $(LEVEL_CAP_REWARDS_BIN) $(LEVEL_CAP_REWARDS_TARGET)
	@touch $(DATA_INSTALL_STAMP)

####################### Incremental ROM assembly #######################
PATCH_STAMP := $(BUILD)/.base_patched
CODE_NARC_STAMP := $(BUILD)/.code_narc_installed
ARMIPS_INPUTS := $(shell find armips -type f)
PATCH_INPUTS := scripts/make.py hooks armhooks bytereplacement repoints routinepointers $(ARMIPS_INPUTS) $(ARMIPS_CONFIG)

# Many archive generators extract an existing file from base/root. Ensure the
# source ROM has been extracted before any of them can run in parallel.
$(NARC_FILES): | $(BASE_EXTRACTION_STAMP)

# Restore pristine executable inputs immediately before every actual patch
# pass. A true no-op build skips this entire stage.
$(PATCH_STAMP): $(OUTPUT) $(OVERLAY_OUTPUTS) $(TOOLS) $(DATA_INSTALL_STAMP) $(PATCH_INPUTS)
	rm -rf $(BASE)/overlay
	@mkdir -p $(BASE)/overlay
	$(NDSTOOL) -x $(ROMNAME) -9 $(BASE)/arm9.bin -y9 $(BASE)/overarm9.bin -y $(BASE)/overlay
	$(PYTHON) scripts/make.py $(CFLAGS)
	$(ARMIPS) armips/global.s $(ARMIPS_FLAGS)
	@touch $@

$(CODE_NARC_STAMP): $(PATCH_STAMP)
	$(NARCHIVE) create $(FILESYS)/a/0/2/8 $(BUILD)/a028/ -nf
	@touch $@

$(BUILDROM): $(CODE_NARC_STAMP)
	@echo "Making ROM..."
	$(NDSTOOL) -c $@ -9 $(BASE)/arm9.bin -7 $(BASE)/arm7.bin -y9 $(BASE)/overarm9.bin -y7 $(BASE)/overarm7.bin -d $(FILESYS) -y $(BASE)/overlay -t $(BASE)/banner.bin -h $(BASE)/header.bin
	@echo "Done.  See output $@."

update_machine_moves: $(VENV_ACTIVATE)
	$(PYTHON) scripts/update_machine_moves.py --descriptions --sprites
	@echo "Updated item descriptions and sprites. Double check formatting"


# needed to keep the $(SDAT_OBJ_DIR)/WAVE_ARC_PV%/00.swav from being detected as an intermediate file
.SECONDARY:

####################### Debug #######################
print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
