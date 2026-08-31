#ifndef HEARTLESS_GOLD_AI_CONFIG_H
#define HEARTLESS_GOLD_AI_CONFIG_H

/* Preference knobs. Mechanical rules do not belong in this file. */
#define AI_CFG_PROBABILITY_SCALE              255
#define AI_CFG_HP_BAR_UTILITY                 100
#define AI_CFG_READ_COMMITMENT_GAP            10
#define AI_CFG_FORCED_FUTURE_PERCENT          95
#define AI_CFG_SPECULATIVE_FUTURE_PERCENT     56
#define AI_CFG_ROBUST_COVERAGE_TARGET         230
#define AI_CFG_COMFORTABLE_RISK_WEIGHT        192
#define AI_CFG_EVEN_RISK_WEIGHT               128
#define AI_CFG_BEHIND_RISK_WEIGHT             64
#define AI_CFG_DESPERATE_RISK_WEIGHT          16
#define AI_CFG_HARD_READ_MIN_PAYOFF           32
#define AI_CFG_SWITCHABLE_TARGET_EFFECT_PERCENT 40
#define AI_CFG_UNKNOWN_ACTION_BASE_VALUE       2
#define AI_CFG_MAX_PLAYER_ACTIONS             9
#define AI_CFG_MAX_SEARCH_AI_ACTIONS          4
#define AI_CFG_MAX_FULL_SWITCH_CANDIDATES     2
#define AI_CFG_MAX_SEARCHED_SWITCH_ACTIONS    2
#define AI_CFG_PLAN_TURN_CAP                   6
#define AI_CFG_RANDOM_ACTION_BASE_VALUE       2
#define AI_CFG_RECENT_BATTLE_CASES            8
#define AI_CFG_PERSISTENT_PREDICTION_CASES    32
#define AI_CFG_TEAM_ASSET_WEIGHT              48
#define AI_CFG_SWITCH_MIN_IMPROVEMENT          8

#if AI_CFG_PROBABILITY_SCALE != 255
#error "Trainer AI probability scale must fit exactly in u8"
#endif
#if AI_CFG_MAX_PLAYER_ACTIONS < 9
#error "Trainer AI must retain all four moves and five singles switches"
#endif
#if AI_CFG_MAX_SEARCH_AI_ACTIONS < AI_CFG_MAX_SEARCHED_SWITCH_ACTIONS
#error "Trainer AI switch shortlist exceeds the complete action shortlist"
#endif
#if AI_CFG_MAX_FULL_SWITCH_CANDIDATES > AI_CFG_MAX_SEARCHED_SWITCH_ACTIONS
#error "Trainer AI admits more switch candidates than final switch slots"
#endif
#if AI_CFG_SWITCHABLE_TARGET_EFFECT_PERCENT < 0 || AI_CFG_SWITCHABLE_TARGET_EFFECT_PERCENT > 100
#error "Trainer AI switchable-target persistence must be a percentage"
#endif
#if AI_CFG_FORCED_FUTURE_PERCENT < 0 || AI_CFG_FORCED_FUTURE_PERCENT > 100
#error "Trainer AI forced continuation value must be a percentage"
#endif
#if AI_CFG_SPECULATIVE_FUTURE_PERCENT < 0 || AI_CFG_SPECULATIVE_FUTURE_PERCENT > 100
#error "Trainer AI speculative continuation value must be a percentage"
#endif
#if AI_CFG_UNKNOWN_ACTION_BASE_VALUE < 0
#error "Trainer AI unknown-action value must remain non-negative"
#endif
#if AI_CFG_SWITCH_MIN_IMPROVEMENT < 0
#error "Trainer AI switch improvement margin must remain non-negative"
#endif

#endif
