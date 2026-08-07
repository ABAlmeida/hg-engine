#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../include/trainer_data.h"
#include "../../../include/constants/file.h"
#include "../../../include/constants/item.h"

enum {
    RAWTEXT_PATH_LENGTH = 128,
    TRAINER_DATA_MEMBER_PATH_LENGTH = 32,
    TRAINER_TEXT_OFFSET_MEMBER_PATH_LENGTH = 48,
    TRAINER_TEXT_ENTRY_SIZE = 4,
    TRAINER_REWARD_LINE_LENGTH = 256,
    TRAINER_OFFER_TEXT_LENGTH = 1024,
    TRAINER_OFFER_LINE_WIDTH = 32,
};

typedef struct TrainerReward {
    uint16_t item;
    uint16_t quantity;
    int configured;
} TrainerReward;

static uint16_t EncodeNicknameChar(char c) {
    if (c >= '0' && c <= '9') {
        return 0x0121 + (uint16_t)(c - '0');
    }

    if (c >= 'A' && c <= 'Z') {
        return 0x012B + (uint16_t)(c - 'A');
    }

    if (c >= 'a' && c <= 'z') {
        return 0x0145 + (uint16_t)(c - 'a');
    }

    return 0;
}

static void BuildRawTextPath(char *path, size_t pathSize, const char *dir, int index) {
    size_t dirLen;

    if (index < 0 || index > 9999) {
        fprintf(stderr, "Rawtext file index out of range: %d\n", index);
        exit(EXIT_FAILURE);
    }

    dirLen = strlen(dir);
    if (dirLen + sizeof("/0000.txt") > pathSize) {
        fprintf(stderr, "Rawtext output path too long: %s\n", dir);
        exit(EXIT_FAILURE);
    }

    memcpy(path, dir, dirLen);
    path[dirLen] = '/';
    path[dirLen + 1] = (char)('0' + ((index / 1000) % 10));
    path[dirLen + 2] = (char)('0' + ((index / 100) % 10));
    path[dirLen + 3] = (char)('0' + ((index / 10) % 10));
    path[dirLen + 4] = (char)('0' + (index % 10));
    memcpy(path + dirLen + 5, ".txt", sizeof(".txt"));
}

static int IsPartyMonDefined(const TrainerPokemonData *mon) {
    return mon->species != SPECIES_NONE;
}

static int GetTrainerPartyMonCount(const TrainerData *entry) {
    int count = 0;
    int i;
    int sawGap = 0;

    for (i = 0; i < TRAINER_SOURCE_MAX_PARTY_SIZE; i++) {
        if (IsPartyMonDefined(&entry->party[i])) {
            if (sawGap) {
                fprintf(stderr, "Trainer \"%s\" has a gap in its party definition\n", entry->name);
                exit(EXIT_FAILURE);
            }
            count++;
        } else if (count > 0) {
            sawGap = 1;
        }
    }

    return count;
}

static int IsTextEntryDefined(const TrainerMessageEntry *entry) {
    return entry->text != NULL;
}

static int GetTrainerTextEntryCount(const TrainerData *entry) {
    int count = 0;
    int i;
    int sawGap = 0;

    for (i = 0; i < TRAINER_SOURCE_MAX_TEXT_ENTRY_COUNT; i++) {
        if (IsTextEntryDefined(&entry->text[i])) {
            if (sawGap) {
                fprintf(stderr, "Trainer \"%s\" has a gap in its text definition\n", entry->name);
                exit(EXIT_FAILURE);
            }
            count++;
        } else if (count > 0) {
            sawGap = 1;
        }
    }

    return count;
}

static int HasTrainerTextType(const TrainerData *entry, uint16_t type) {
    int textCount = GetTrainerTextEntryCount(entry);
    int i;

    for (i = 0; i < textCount; i++) {
        if (entry->text[i].type == type) {
            return 1;
        }
    }
    return 0;
}

static int HasTrainerIntroText(const TrainerData *entry) {
    return HasTrainerTextType(entry, TRMSG_INTRO) || HasTrainerTextType(entry, TRMSG_DBL_INTRO_1) ||
           HasTrainerTextType(entry, TRMSG_DBL_INTRO_2);
}

static int GetTrainerTextOffsetCount(void) {
    int i;

    for (i = (int)sTrainerDataCount - 1; i >= 0; i--) {
        if (GetTrainerTextEntryCount(&sTrainerData[i]) > 0) {
            return i + 1;
        }
    }

    return 0;
}

static void WriteLe16(uint8_t *dst, uint16_t value) {
    dst[0] = (uint8_t)(value & 0xFF);
    dst[1] = (uint8_t)(value >> 8);
}

static void WriteLe32(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value & 0xFF);
    dst[1] = (uint8_t)((value >> 8) & 0xFF);
    dst[2] = (uint8_t)((value >> 16) & 0xFF);
    dst[3] = (uint8_t)((value >> 24) & 0xFF);
}

static FILE *OpenBinaryForWrite(const char *path) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    }
    return file;
}

static FILE *OpenTextForWrite(const char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    }
    return file;
}

static char *TrimWhitespace(char *text) {
    char *end;

    while (isspace((unsigned char)*text)) {
        text++;
    }
    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';
    return text;
}

static unsigned long ParseUnsigned(const char *text, const char *field, int lineNumber) {
    char *end;
    unsigned long value = strtoul(text, &end, 0);

    if (text[0] == '\0' || *end != '\0') {
        fprintf(stderr, "Invalid %s on trainer reward line %d: %s\n", field, lineNumber, text);
        exit(EXIT_FAILURE);
    }
    return value;
}

static TrainerReward *LoadTrainerRewards(const char *path) {
    TrainerReward *rewards = calloc(sTrainerDataCount, sizeof(*rewards));
    char line[TRAINER_REWARD_LINE_LENGTH];
    FILE *file;
    int lineNumber = 0;

    if (rewards == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    file = fopen(path, "r");
    if (file == NULL) {
        perror(path);
        free(rewards);
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *comment;
        char *trainerText;
        char *itemText;
        char *quantityText;
        char *extra;
        unsigned long trainerId;
        unsigned long itemId;
        unsigned long quantity;

        lineNumber++;
        if (strchr(line, '\n') == NULL && !feof(file)) {
            fprintf(stderr, "Trainer reward line %d is too long\n", lineNumber);
            fclose(file);
            free(rewards);
            exit(EXIT_FAILURE);
        }
        comment = strchr(line, '#');
        if (comment != NULL) {
            *comment = '\0';
        }
        trainerText = TrimWhitespace(line);
        if (*trainerText == '\0' || strcmp(trainerText, "trainer_id,item_id,quantity") == 0) {
            continue;
        }

        itemText = strchr(trainerText, ',');
        if (itemText == NULL) {
            fprintf(stderr, "Trainer reward line %d must have three columns\n", lineNumber);
            fclose(file);
            free(rewards);
            exit(EXIT_FAILURE);
        }
        *itemText++ = '\0';
        quantityText = strchr(itemText, ',');
        if (quantityText == NULL) {
            fprintf(stderr, "Trainer reward line %d must have three columns\n", lineNumber);
            fclose(file);
            free(rewards);
            exit(EXIT_FAILURE);
        }
        *quantityText++ = '\0';
        extra = strchr(quantityText, ',');
        if (extra != NULL) {
            fprintf(stderr, "Trainer reward line %d has too many columns\n", lineNumber);
            fclose(file);
            free(rewards);
            exit(EXIT_FAILURE);
        }

        trainerText = TrimWhitespace(trainerText);
        itemText = TrimWhitespace(itemText);
        quantityText = TrimWhitespace(quantityText);
        trainerId = ParseUnsigned(trainerText, "trainer ID", lineNumber);
        itemId = ParseUnsigned(itemText, "item ID", lineNumber);
        quantity = ParseUnsigned(quantityText, "quantity", lineNumber);

        if (trainerId >= sTrainerDataCount) {
            fprintf(stderr, "Trainer reward line %d has out-of-range trainer ID %lu\n", lineNumber, trainerId);
            fclose(file);
            free(rewards);
            exit(EXIT_FAILURE);
        }
        if (rewards[trainerId].configured) {
            fprintf(stderr, "Trainer reward line %d duplicates trainer ID %lu\n", lineNumber, trainerId);
            fclose(file);
            free(rewards);
            exit(EXIT_FAILURE);
        }
        if ((itemId == ITEM_NONE) != (quantity == 0)) {
            fprintf(stderr, "Trainer reward line %d must use item 0 and quantity 0 together\n", lineNumber);
            fclose(file);
            free(rewards);
            exit(EXIT_FAILURE);
        }
        if (itemId > MAX_TOTAL_ITEM_NUM || quantity > BAG_SLOT_QUANTITY_MAX) {
            fprintf(stderr, "Trainer reward line %d has an invalid item or quantity\n", lineNumber);
            fclose(file);
            free(rewards);
            exit(EXIT_FAILURE);
        }

        rewards[trainerId].item = (uint16_t)itemId;
        rewards[trainerId].quantity = (uint16_t)quantity;
        rewards[trainerId].configured = 1;
    }

    fclose(file);
    return rewards;
}

static void ReadIndexedTextFile(const char *dir, int index, char *text, size_t textSize) {
    char path[RAWTEXT_PATH_LENGTH];
    FILE *file;

    BuildRawTextPath(path, sizeof(path), dir, index);
    file = fopen(path, "r");
    if (file == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    }
    if (fgets(text, (int)textSize, file) == NULL) {
        fprintf(stderr, "Unable to read text from %s\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    if (strchr(text, '\n') == NULL && !feof(file)) {
        fprintf(stderr, "Text in %s exceeds the supported length\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);
    TrimWhitespace(text);
}

static void ReadTextArchiveLine(const char *textDir, int archiveId, int index, char *text, size_t textSize) {
    char path[RAWTEXT_PATH_LENGTH];
    FILE *file;
    int current = 0;

    snprintf(path, sizeof(path), "%s/%03d.txt", textDir, archiveId);
    file = fopen(path, "r");
    if (file == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    }
    while (current <= index && fgets(text, (int)textSize, file) != NULL) {
        current++;
    }
    if (current <= index) {
        fprintf(stderr, "Text index %d is missing from %s\n", index, path);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    if (strchr(text, '\n') == NULL && !feof(file)) {
        fprintf(stderr, "Text line %d in %s exceeds the supported length\n", index, path);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);
    TrimWhitespace(text);
}

static void StripTextControls(char *text) {
    char *src = text;
    char *dst = text;

    while (*src != '\0') {
        if (*src == '{') {
            while (*src != '\0' && *src != '}') {
                src++;
            }
            if (*src == '}') {
                src++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

static void AppendText(char *dst, size_t dstSize, const char *text) {
    size_t used = strlen(dst);
    size_t remaining = dstSize - used;

    if (strlen(text) + 1 > remaining) {
        fprintf(stderr, "Generated trainer offer exceeds %zu bytes\n", dstSize);
        exit(EXIT_FAILURE);
    }
    memcpy(dst + used, text, strlen(text) + 1);
}

static void WrapTrainerOffer(const char *sentence, char *offer, size_t offerSize) {
    char copy[TRAINER_OFFER_TEXT_LENGTH];
    char *word;
    int lineLength = 0;
    int lineOnPage = 1;

    if (strlen(sentence) >= sizeof(copy)) {
        fprintf(stderr, "Generated trainer offer sentence is too long\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, sentence);
    offer[0] = '\0';

    word = strtok(copy, " ");
    while (word != NULL) {
        int wordLength = (int)strlen(word);

        if (lineLength != 0 && lineLength + 1 + wordLength > TRAINER_OFFER_LINE_WIDTH) {
            AppendText(offer, offerSize, lineOnPage == 1 ? "\\n" : "\\r");
            lineOnPage = lineOnPage == 1 ? 2 : 1;
            lineLength = 0;
        }
        if (lineLength != 0) {
            AppendText(offer, offerSize, " ");
            lineLength++;
        }
        AppendText(offer, offerSize, word);
        lineLength += wordLength;
        word = strtok(NULL, " ");
    }
    AppendText(offer, offerSize, "\\rDo you want to battle?");
}

static void BuildTrainerTeamText(const TrainerData *entry, const char *speciesNameDir, char *team, size_t teamSize) {
    int partyCount = GetTrainerPartyMonCount(entry);
    int i;

    team[0] = '\0';
    for (i = 0; i < partyCount; i++) {
        char speciesName[RAWTEXT_PATH_LENGTH];

        ReadIndexedTextFile(speciesNameDir, entry->party[i].species, speciesName, sizeof(speciesName));
        if (i > 0) {
            AppendText(team, teamSize, i == partyCount - 1 ? (partyCount == 2 ? " and " : ", and ") : ", ");
        }
        AppendText(team, teamSize, speciesName);
    }
}

static void BuildTrainerOfferText(const TrainerData *entry, const TrainerReward *reward, const char *speciesNameDir,
                                  const char *itemTextDir, char *offer, size_t offerSize) {
    char team[TRAINER_OFFER_TEXT_LENGTH / 2];
    char sentence[TRAINER_OFFER_TEXT_LENGTH];
    int sentenceLength;

    BuildTrainerTeamText(entry, speciesNameDir, team, sizeof(team));
    if (reward->item == ITEM_NONE) {
        sentenceLength = snprintf(sentence, sizeof(sentence), "Can you beat my team of %s?", team);
    } else {
        enum ItemGeneration generation = ITEM_GENERATION(reward->item);
        int archiveId;
        char itemName[RAWTEXT_PATH_LENGTH];

        if (generation == CUSTOM) {
            archiveId = reward->quantity == 1 ? MSG_DATA_ITEM_NAME_ARTICLE_CUSTOM : MSG_DATA_ITEM_NAME_PLURAL_CUSTOM;
        } else {
            int base = reward->quantity == 1 ? MSG_DATA_ITEM_NAME_ARTICLE_GEN4 : MSG_DATA_ITEM_NAME_PLURAL_GEN4;
            archiveId = MSG_DATA_ITEM_FILE(base, generation);
        }
        ReadTextArchiveLine(itemTextDir, archiveId, ITEM_MSG_OFFSET(reward->item), itemName, sizeof(itemName));
        StripTextControls(itemName);
        sentenceLength =
            snprintf(sentence, sizeof(sentence), "I’ll give you %s if you can beat my team of %s.", itemName, team);
    }
    if (sentenceLength < 0 || (size_t)sentenceLength >= sizeof(sentence)) {
        fprintf(stderr, "Generated trainer offer sentence was truncated\n");
        exit(EXIT_FAILURE);
    }
    WrapTrainerOffer(sentence, offer, offerSize);
}

static void WriteTrainerRewardScript(const char *path, const TrainerReward *rewards) {
    FILE *file = OpenTextForWrite(path);
    u32 trainerId;

    fputs("// Generated from data/trainer_rewards.csv. Do not edit.\n", file);
    fputs("_hg_load_trainer_reward:\n", file);
    fputs("\tsetvar VAR_SPECIAL_x8005, 0\n", file);
    for (trainerId = 0; trainerId < sTrainerDataCount; trainerId++) {
        if (rewards[trainerId].item != ITEM_NONE) {
            fprintf(file, "\tcompare VAR_SPECIAL_x8004, %u\n", trainerId);
            fprintf(file, "\tgoto_if_eq _hg_trainer_reward_%u\n", trainerId);
        }
    }
    fputs("\treturn\n", file);
    for (trainerId = 0; trainerId < sTrainerDataCount; trainerId++) {
        if (rewards[trainerId].item != ITEM_NONE) {
            fprintf(file, "_hg_trainer_reward_%u:\n", trainerId);
            fprintf(file, "\tsetvar VAR_SPECIAL_x8004, %u\n", rewards[trainerId].item);
            fprintf(file, "\tsetvar VAR_SPECIAL_x8005, %u\n", rewards[trainerId].quantity);
            fputs("\treturn\n", file);
        }
    }
    fclose(file);
}

static void WriteTrainerNameTextFile(const char *dir, int index, const char *name) {
    char path[RAWTEXT_PATH_LENGTH];
    FILE *file;

    BuildRawTextPath(path, sizeof(path), dir, index);
    file = OpenTextForWrite(path);
    fputs("{TRNAME}", file);
    fputs(name, file);
    fclose(file);
}

static void WriteTextFile(const char *dir, int index, const char *text) {
    char path[RAWTEXT_PATH_LENGTH];
    FILE *file;

    BuildRawTextPath(path, sizeof(path), dir, index);
    file = OpenTextForWrite(path);
    fputs(text, file);
    fclose(file);
}

static void WriteTrainerHeaderMember(const char *dir, int index, const TrainerData *entry) {
    char path[TRAINER_DATA_MEMBER_PATH_LENGTH];
    uint8_t data[20] = {0};
    FILE *file;
    int partyCount;
    int i;

    snprintf(path, sizeof(path), "%s/5_%03d", dir, index);
    file = OpenBinaryForWrite(path);
    partyCount = GetTrainerPartyMonCount(entry);

    data[0x00] = entry->data.trainerType;
    WriteLe16(&data[0x01], entry->data.trainerClass);
    data[0x03] = (uint8_t)((entry->data.partySize & TRAINER_DATA_RANDOM_PARTY_ORDER) | partyCount);
    for (i = 0; i < 4; i++) {
        WriteLe16(&data[0x04 + (i * 2)], entry->data.items[i]);
    }
    WriteLe32(&data[0x0C], entry->data.aiFlags);
    WriteLe32(&data[0x10], entry->data.battleType);

    if (fwrite(data, sizeof(data), 1, file) != 1) {
        perror(path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

static void WriteTrainerPartyMember(const char *dir, int index, const TrainerData *entry) {
    char path[TRAINER_DATA_MEMBER_PATH_LENGTH];
    FILE *file;
    uint8_t data[512] = {0};
    size_t size = 0;
    int partyCount;
    int i;
    int j;

    snprintf(path, sizeof(path), "%s/6_%03d", dir, index);
    file = OpenBinaryForWrite(path);
    partyCount = GetTrainerPartyMonCount(entry);

    if (partyCount == 0) {
        uint8_t placeholder[8] = {0};

        if (fwrite(placeholder, sizeof(placeholder), 1, file) != 1) {
            perror(path);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        fclose(file);
        return;
    }

    for (i = 0; i < partyCount; i++) {
        const TrainerPokemonData *mon = &entry->party[i];

        data[size++] = mon->ivs;
        data[size++] = mon->abilitySlot;
        WriteLe16(&data[size], mon->level);
        size += 2;

        WriteLe16(&data[size], mon->species);
        size += 2;

        if (entry->data.trainerType & TRAINER_DATA_TYPE_ITEMS) {
            WriteLe16(&data[size], mon->item);
            size += 2;
        }

        if (entry->data.trainerType & TRAINER_DATA_TYPE_MOVES) {
            for (j = 0; j < 4; j++) {
                WriteLe16(&data[size], mon->moves[j]);
                size += 2;
            }
        }

        if (entry->data.trainerType & TRAINER_DATA_TYPE_ABILITY) {
            WriteLe16(&data[size], mon->ability);
            size += 2;
        }

        if (entry->data.trainerType & TRAINER_DATA_TYPE_BALL) {
            WriteLe16(&data[size], mon->ball);
            size += 2;
        }

        if (entry->data.trainerType & TRAINER_DATA_TYPE_IV_EV_SET) {
            data[size++] = mon->setIvs.hp;
            data[size++] = mon->setIvs.attack;
            data[size++] = mon->setIvs.defense;
            data[size++] = mon->setIvs.speed;
            data[size++] = mon->setIvs.spAttack;
            data[size++] = mon->setIvs.spDefense;
            data[size++] = mon->setEvs.hp;
            data[size++] = mon->setEvs.attack;
            data[size++] = mon->setEvs.defense;
            data[size++] = mon->setEvs.speed;
            data[size++] = mon->setEvs.spAttack;
            data[size++] = mon->setEvs.spDefense;
        }

        if (entry->data.trainerType & TRAINER_DATA_TYPE_NATURE_SET) {
            data[size++] = mon->nature;
        }

        if (entry->data.trainerType & TRAINER_DATA_TYPE_SHINY_LOCK) {
            data[size++] = mon->shinyLock;
        }

        if (entry->data.trainerType & TRAINER_DATA_TYPE_ADDITIONAL_FLAGS) {
            WriteLe32(&data[size], mon->additionalFlags);
            size += 4;

            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_STATUS) {
                WriteLe32(&data[size], mon->status);
                size += 4;
            }
            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_HP) {
                WriteLe16(&data[size], mon->hp);
                size += 2;
            }
            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_ATK) {
                WriteLe16(&data[size], mon->attack);
                size += 2;
            }
            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_DEF) {
                WriteLe16(&data[size], mon->defense);
                size += 2;
            }
            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_SPEED) {
                WriteLe16(&data[size], mon->speed);
                size += 2;
            }
            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_SP_ATK) {
                WriteLe16(&data[size], mon->spAttack);
                size += 2;
            }
            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_SP_DEF) {
                WriteLe16(&data[size], mon->spDefense);
                size += 2;
            }
            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_PP_COUNTS) {
                for (j = 0; j < 4; j++) {
                    data[size++] = mon->ppCounts[j];
                }
            }
            if (mon->additionalFlags & TRAINER_DATA_EXTRA_TYPE_NICKNAME) {
                if (mon->nicknameStr != NULL) {
                    int nicknameLen = strlen(mon->nicknameStr);
                    if (nicknameLen > 10) {
                        fprintf(stderr, "Nickname \"%s\" is too long; max is 10 characters\n", mon->nicknameStr);
                        exit(EXIT_FAILURE);
                    }

                    for (j = 0; j < 11; j++) {
                        if (j < nicknameLen) {
                            WriteLe16(&data[size], EncodeNicknameChar(mon->nicknameStr[j]));
                            size += 2;
                        } else if (j == nicknameLen) {
                            WriteLe16(&data[size], 0xFFFF);
                            size += 2;
                        } else {
                            WriteLe16(&data[size], 0);
                            size += 2;
                        }
                    }
                } else {
                    for (j = 0; j < 11; j++) {
                        WriteLe16(&data[size], mon->nickname[j]);
                        size += 2;
                    }
                }
            }
        }

        WriteLe16(&data[size], mon->ballSeal);
        size += 2;
    }

    if (size > 0 && fwrite(data, size, 1, file) != 1) {
        perror(path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

static void WriteTrainerTextData(const char *mapDir, const char *offsetDir, const char *rawTextDir,
                                 const char *speciesNameDir, const char *itemTextDir, const TrainerReward *rewards) {
    char mapPath[TRAINER_TEXT_OFFSET_MEMBER_PATH_LENGTH];
    char offsetPath[TRAINER_TEXT_OFFSET_MEMBER_PATH_LENGTH];
    FILE *mapFile;
    FILE *offsetFile;
    uint16_t *offsets;
    uint16_t offset = 0;
    int textFileIndex = 0;
    int trainerTextOffsetCount;
    int i;
    int j;
    u32 orderIndex;

    offsets = calloc(sTrainerDataCount, sizeof(*offsets));
    if (offsets == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    trainerTextOffsetCount = GetTrainerTextOffsetCount();

    snprintf(mapPath, sizeof(mapPath), "%s/7_0", mapDir);
    snprintf(offsetPath, sizeof(offsetPath), "%s/1_0", offsetDir);
    mapFile = OpenBinaryForWrite(mapPath);
    offsetFile = OpenBinaryForWrite(offsetPath);

    for (orderIndex = 0; orderIndex < sTrainerTextOrderCount; orderIndex++) {
        const TrainerData *entry;
        uint16_t trainerId = sTrainerTextOrder[orderIndex];
        int textCount;

        if (trainerId >= sTrainerDataCount) {
            fprintf(stderr, "Trainer text order contains out-of-range trainer id %u\n", trainerId);
            free(offsets);
            exit(EXIT_FAILURE);
        }

        entry = &sTrainerData[trainerId];
        textCount = GetTrainerTextEntryCount(entry);
        if (textCount == 0) {
            continue;
        }

        offsets[trainerId] = offset;
        for (j = 0; j < textCount; j++) {
            uint8_t data[TRAINER_TEXT_ENTRY_SIZE];

            WriteLe16(&data[0], trainerId);
            WriteLe16(&data[2], entry->text[j].type);
            if (fwrite(data, sizeof(data), 1, mapFile) != 1) {
                perror(mapPath);
                fclose(mapFile);
                fclose(offsetFile);
                free(offsets);
                exit(EXIT_FAILURE);
            }
            WriteTextFile(rawTextDir, textFileIndex++, entry->text[j].text);
            offset = (uint16_t)(offset + TRAINER_TEXT_ENTRY_SIZE);
        }
        if (HasTrainerIntroText(entry)) {
            uint8_t data[TRAINER_TEXT_ENTRY_SIZE];
            char offer[TRAINER_OFFER_TEXT_LENGTH];

            BuildTrainerOfferText(entry, &rewards[trainerId], speciesNameDir, itemTextDir, offer, sizeof(offer));
            WriteLe16(&data[0], trainerId);
            WriteLe16(&data[2], TRMSG_BATTLE_OFFER);
            if (fwrite(data, sizeof(data), 1, mapFile) != 1) {
                perror(mapPath);
                fclose(mapFile);
                fclose(offsetFile);
                free(offsets);
                exit(EXIT_FAILURE);
            }
            WriteTextFile(rawTextDir, textFileIndex++, offer);
            offset = (uint16_t)(offset + TRAINER_TEXT_ENTRY_SIZE);
        }
    }

    for (i = 0; i < trainerTextOffsetCount; i++) {
        uint8_t data[2];

        WriteLe16(data, offsets[i]);
        if (fwrite(data, sizeof(data), 1, offsetFile) != 1) {
            perror(offsetPath);
            fclose(mapFile);
            fclose(offsetFile);
            free(offsets);
            exit(EXIT_FAILURE);
        }
    }

    fclose(mapFile);
    fclose(offsetFile);
    free(offsets);
}

int main(int argc, char **argv) {
    const char *trainerDataDir;
    const char *trainerPartyDir;
    const char *trainerTextMapDir;
    const char *trainerTextOffsetDir;
    const char *rawTextRoot;
    const char *rewardDataPath;
    const char *speciesNameDir;
    const char *itemTextDir;
    const char *rewardScriptPath;
    TrainerReward *rewards;
    char trainerNamesDir[RAWTEXT_PATH_LENGTH];
    char trainerTextDir[RAWTEXT_PATH_LENGTH];
    int i;

    if (argc != 10) {
        fprintf(stderr,
                "Usage: %s <a055-dir> <a056-dir> <a057-dir> <a131-dir> <rawtext-root> <rewards.csv> "
                "<species-names-dir> <item-text-dir> <reward-script>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    trainerDataDir = argv[1];
    trainerPartyDir = argv[2];
    trainerTextMapDir = argv[3];
    trainerTextOffsetDir = argv[4];
    rawTextRoot = argv[5];
    rewardDataPath = argv[6];
    speciesNameDir = argv[7];
    itemTextDir = argv[8];
    rewardScriptPath = argv[9];
    rewards = LoadTrainerRewards(rewardDataPath);

    snprintf(trainerNamesDir, sizeof(trainerNamesDir), "%s/729", rawTextRoot);
    snprintf(trainerTextDir, sizeof(trainerTextDir), "%s/728", rawTextRoot);

    for (i = 0; i < (int)sTrainerDataCount; i++) {
        WriteTrainerHeaderMember(trainerDataDir, i, &sTrainerData[i]);
        WriteTrainerPartyMember(trainerPartyDir, i, &sTrainerData[i]);
        WriteTrainerNameTextFile(trainerNamesDir, i, sTrainerData[i].name);
    }

    for (i = 0; i < (int)sTrainerDataCount; i++) {
        if (rewards[i].item != ITEM_NONE &&
            (!HasTrainerIntroText(&sTrainerData[i]) || GetTrainerPartyMonCount(&sTrainerData[i]) == 0)) {
            fprintf(stderr, "Trainer reward %d does not target a trainer with an introductory message and party\n", i);
            free(rewards);
            return EXIT_FAILURE;
        }
    }

    WriteTrainerTextData(trainerTextMapDir, trainerTextOffsetDir, trainerTextDir, speciesNameDir, itemTextDir, rewards);
    WriteTrainerRewardScript(rewardScriptPath, rewards);
    free(rewards);
    return EXIT_SUCCESS;
}
