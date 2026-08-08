#include "../include/types.h"

#include "../include/summary.h"
#include "../include/battle.h"


// file is from LheaRachel on github who adapted it from Bubble
// i just adapted it to hgss and added the +/- handling and such


static void UpdatePokemonData(struct SummaryState *summary, u8 mode)
{
    void *potentialBoxMon = Summary_GetPokemonData(summary);
    void *pokemon;

    // Use a different data accessor for summary screens from a PC box
    if (summary->baseData->dataType == 2) {
        pokemon = AllocMonZeroed(19); // heap id 19
        CopyBoxPokemonToPokemon(potentialBoxMon, pokemon);
    } else {
        pokemon = potentialBoxMon;
    }

    int paramStart = MON_DATA_MAXHP;

    // Pokemon struct orders current HP before max HP, so need to handle mode == 0 as a special case
    if (mode == 0) {
        summary->pokemonData.hp = (u16) GetMonData(pokemon, MON_DATA_HP, NULL);
    } else {
        if (mode == 1) {
            paramStart = MON_DATA_HP_EV;
        } else {
            paramStart = MON_DATA_HP_IV;
        }

        summary->pokemonData.hp = (u16) GetMonData(pokemon, paramStart, NULL);
    }

    // Pokemon struct orders all data parameters as such:
    // Attack -> Defense -> Speed -> SpAttack -> SpDefense
    summary->pokemonData.attack    = (u16) GetMonData(pokemon, paramStart + 1, NULL);
    summary->pokemonData.defense   = (u16) GetMonData(pokemon, paramStart + 2, NULL);
    summary->pokemonData.speed     = (u16) GetMonData(pokemon, paramStart + 3, NULL);
    summary->pokemonData.spAttack  = (u16) GetMonData(pokemon, paramStart + 4, NULL);
    summary->pokemonData.spDefense = (u16) GetMonData(pokemon, paramStart + 5, NULL);


    if (summary->baseData->dataType == 2) { // free it if it was allocated
        sys_FreeMemoryEz(pokemon);
    }
}

// Components here are indices into the system font palette.
//   l == index of the color to use for the letter itself
//   s == index of the color to use for the shadow of the letter
//   g == index of the color to use for the background of the letter (0 == transparent)
#define COLOR(l, s, g) ((u32)(((l & 0xFF) << 16) | ((s & 0xFF) << 8) | ((g & 0xFF) << 0)))
#define BLACK          (COLOR(1,  2, 0))
#define BLUE_INVERT    (COLOR(4,  3, 0))
#define RED_INVERT     (COLOR(6,  5, 0))
#define BLUE           (COLOR(3,  4, 0))
#define RED            (COLOR(5,  6, 0))
#define PINK           (COLOR(7,  8, 0))
#define GREEN          (COLOR(9, 10, 0))
#define WHITE          (COLOR(0xE, 0xF, 0))

// The original Skills-page renderer reserves definition window 2 for its heading.
#define SUMMARY_WINDOW_SKILLS_HEADER 2
#define SUMMARY_MSG_SKILLS           109
#define SUMMARY_MSG_FRIENDSHIP       208

static s32 Summary_GetNatureStatEffect(u32 nature, u8 windowIdx)
{
    u32 stat;
    u32 raisedStat;
    u32 loweredStat;

    if (windowIdx == 0) {
        return 0;
    }

    // Nature ordering is Attack, Defense, Speed, Sp. Atk, Sp. Def. The
    // Summary windows display Speed last, so translate the window index.
    stat = windowIdx - 1;
    if (stat >= 2) {
        stat = stat == 4 ? 2 : stat + 1;
    }

    raisedStat = nature / 5;
    loweredStat = nature % 5;
    if (raisedStat == loweredStat) {
        return 0;
    }
    if (stat == raisedStat) {
        return 1;
    }
    if (stat == loweredStat) {
        return -1;
    }
    return 0;
}

static void Summary_PrintFriendship(struct SummaryState *summary, struct BoxPokemon *pokemon)
{
    GF_BGL_BMPWIN *window = &summary->defnWindows[SUMMARY_WINDOW_SKILLS_HEADER];

    FillWindowPixelBuffer(window, 0);

    // Eggs store remaining hatch cycles in the friendship field.
    if (summary->pokemonData.isEgg) {
        Summary_PrintStringGeneric(
            summary,
            SUMMARY_WINDOW_SKILLS_HEADER,
            SUMMARY_MSG_SKILLS,
            WHITE,
            JUSTIFY_LEFT
        );
    } else {
        // PartyPokemon begins with BoxPokemon, so this accessor supports both
        // party and PC summary sources without allocating a temporary mon.
        u32 friendship = GetBoxMonData(pokemon, MON_DATA_FRIENDSHIP, NULL);

        Summary_NumberToString(summary, SUMMARY_MSG_FRIENDSHIP, friendship, 3, 0);
        Summary_PrintString(summary, window, WHITE, JUSTIFY_LEFT);
    }

    CopyWindowToVram(window);
}

static void PrintStatNumberWithColor(struct SummaryState *summary, u8 windowIdx, u32 justify, u32 nature)
{
    u32 color = BLACK;
    s32 effect = Summary_GetNatureStatEffect(nature, windowIdx);

    if (effect > 0) {
        color = RED;
    } else if (effect < 0) {
        color = BLUE;
    }

    Summary_PrintString(summary, &summary->addlWindows[windowIdx], color, justify);
}

static void Summary_ColorizeStatScreen(
    struct SummaryState *summary,
    u32 mode,
    struct BoxPokemon *pokemon,
    u32 nature
)
{
    Summary_PrintFriendship(summary, pokemon);

    Summary_NumberToString(summary, 120, summary->pokemonData.attack, 3, 0);
    PrintStatNumberWithColor(summary, 1, JUSTIFY_RIGHT, nature);
    Summary_NumberToString(summary, 121, summary->pokemonData.defense, 3, 0);
    PrintStatNumberWithColor(summary, 2, JUSTIFY_RIGHT, nature);
    Summary_NumberToString(summary, 122, summary->pokemonData.spAttack, 3, 0);
    PrintStatNumberWithColor(summary, 3, JUSTIFY_RIGHT, nature);
    Summary_NumberToString(summary, 123, summary->pokemonData.spDefense, 3, 0);
    PrintStatNumberWithColor(summary, 4, JUSTIFY_RIGHT, nature);
    Summary_NumberToString(summary, 124, summary->pokemonData.speed, 3, 0);
    PrintStatNumberWithColor(summary, 5, JUSTIFY_RIGHT, nature);

    for (int i = 0; i < 6; i++) {
        FillWindowPixelBuffer(&summary->defnWindows[0xF+i], 0);
        if (i != 0) // print a possibly colored text and append +/-
        {
            u32 msgId = 110;
            u32 color = WHITE;
            s32 effect = Summary_GetNatureStatEffect(nature, i);

            if (effect > 0) {
                msgId = 196-1; // Stat+
                color = RED_INVERT;
            } else if (effect < 0) {
                msgId = 201-1; // Stat-
                color = BLUE_INVERT;
            }

            //Summary_PrintStatStringAccountForStat(summary, 0xF+i, msgId+i, i-1, JUSTIFY_LEFT);
            Summary_PrintStringGeneric(summary, 0xF+i, msgId+i, color, JUSTIFY_LEFT);
        } else if (mode == 0) { // raw stat
            Summary_PrintStringGeneric(summary, 0xF, 110, WHITE, JUSTIFY_LEFT);
        } else if (mode == 1) { // ev's
            Summary_PrintStringGeneric(summary, 0xF, 206, WHITE, JUSTIFY_LEFT);
        } else {                // iv's
            Summary_PrintStringGeneric(summary, 0xF, 207, WHITE, JUSTIFY_LEFT);
        }
        CopyWindowToVram(&summary->defnWindows[0xF+i]);
    }
}

void Summary_ColorizeStatScreen_Wrap(struct SummaryState *summary)
{
    struct BoxPokemon *pokemon = Summary_GetPokemonData(summary);

    Summary_ColorizeStatScreen(summary, 0, pokemon, GetBoxMonNatureCountMints(pokemon));
}

void Summary_ChangeStatScreenState(struct SummaryState *summary, u8 mode)
{
    struct BoxPokemon *pokemon;
    u32 nature;

    for (int i = 0; i < 6; i++) {
        FillWindowPixelBuffer(&summary->addlWindows[i], 0);
    }

    UpdatePokemonData(summary, mode);
    pokemon = Summary_GetPokemonData(summary);
    nature = GetBoxMonNatureCountMints(pokemon);

    if (mode) {
        // Print IVs or EVs
        Summary_NumberToString(summary, 119, summary->pokemonData.hp, 3, 1);
        PrintStatNumberWithColor(summary, 0, JUSTIFY_CENTER, nature);
    } else {
        // Print cur and max
        u8 xsize = summary->addlWindows[0].sizx * 8;
        Summary_PrintCurrentOverMax(
            summary,
            0,
            117, 119, 118,
            summary->pokemonData.hp,
            summary->pokemonData.maxHP,
            3,
            xsize / 2, 0
        );
    }

    Summary_ColorizeStatScreen(summary, mode, pokemon, nature);

    for (int i = 0; i < 6; i++) {
        CopyWindowToVram(&summary->addlWindows[i]);
    }

    UpdatePokemonData(summary, 0);      // Recover old data for page change
}


// change this to possibly take a BoxPokemon structure but be compatible with vanilla handling just in case
u16 ModifyStatByNature(u32 nature, u16 n, u8 statIndex) {
    u32 retVal;

    // Dont modify HP, Accuracy, or Evasion by nature
    if (statIndex < STAT_ATTACK || statIndex > STAT_SPECIAL_DEFENSE) {
        return n;
    }

    if (nature & 0x02000000)
    {
        nature = GetBoxMonNatureCountMints(&((struct PartyPokemon *)nature)->box);
    }

    // thanks to dray for this fix!
    if (statIndex == STAT_SPEED) // have to convert to window index to use the sNatureStatEffects table
    {
        statIndex = 5;
    } else if (statIndex > STAT_SPEED) {
        statIndex--;
    }

    switch (Summary_GetNatureStatEffect(nature, statIndex)) {
    case 1:
        // NOTE: will overflow for n > 595 because the intermediate value is cast to u16 before the division.
        retVal = n * 110;
        retVal /= 100;
        break;
    case -1:
        // NOTE: will overflow for n > 728, see above
        retVal = n * 90;
        retVal /= 100;
        break;
    default:
        retVal = n;
        break;
    }
    return retVal;
}
