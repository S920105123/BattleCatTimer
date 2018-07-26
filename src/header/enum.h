#ifndef ENUM_H
#define ENUM_H

enum Lu_Table_Variable {
    INPUT_NET_TRANSITION = 0,
    TOTAL_OUTPUT_NET_CAP,
    CONSTRAINED_PIN_TRANSITION,   // data slew
    RELATED_PIN_TRANSIITION,      // clock slew
    UNDEFINED_LU_TABLE_VARIABLE // default
};

enum Timing_Sense {
    NEGATIVE_UNATE = 0,
    POSITIVE_UNATE,
    NON_UNATE,
    UNDEFINED_TIMING_SENSE      // default
};

enum Timing_Type {
    COMBINATINAL = 0,                // default
    RISING_EDGE,
    FALLING_EDGE,
    SETUP_RISING,
    SETUP_FALLING,
    HOLD_RISING,
    HOLD_FALLING,
    CLEAR,
    PRESET,
    RECOVERY_RISING,
    RECOVERY_FALLING,
    THREE_STATE_DISABLE,
    UNDEFINED_TIMING_TYPE
};

typedef bool Transition_Type;
enum {
    RISE = 0,
    FALL
};

typedef bool Direction_type;
enum {
	INPUT = 0,
	OUTPUT
};

typedef bool Mode;
enum {
	EARLY = 0,
	LATE
};

typedef enum {
	CACHE_FOUT = 0,
	CACHE_FIN
}CacheNode_Type;

#endif /* end ENUM_H */
