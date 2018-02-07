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
    UNDEFINED_TIMING_TYPE
};

enum Transition_Type {
    FALL = 0,
    RISE
};

enum Direction_type {
	INPUT = 0,
	OUTPUT
};

enum Mode {
	EARLY = 0,
	LATE
};

#endif /* end ENUM_H */
