CC := gcc
OBJCOPY := objcopy
CFLAGS := -std=c11 -O2 -Wall -Wextra -I../../..
TARGET := ../../../tools/trainerdatagen
HOST_EXE := $(if $(findstring MINGW,$(shell uname -s)),.exe,)

SRCS := trainer_data_gen.c
DATA_SRC := ../../../data/Trainers.c
DATA_OBJ := TrainersData.o
OBJS := $(SRCS:%.c=%.o) $(DATA_OBJ)

.PHONY: all clean

all: $(TARGET)
	@:

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	# Current MinGW binutils can place the non-runtime .comment section above
	# the PE image range for this large generated-data executable. Windows then
	# rejects an otherwise valid host tool. Removing that section is lossless.
	$(OBJCOPY) --remove-section=.comment $@$(HOST_EXE)

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
$(DEPDIR): ; @mkdir -p $@

%.o: %.c
%.o: %.c $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(DATA_OBJ): $(DATA_SRC) ../../../include/trainer_data.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(TARGET) trainerdatagen.exe $(OBJS)

DEPFILES := $(SRCS:%.c=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
