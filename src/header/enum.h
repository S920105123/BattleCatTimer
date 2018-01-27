
enum Lu_Table_Variable{
    INPUT_NET_TRANSITION = 0,
    TOTAL_OUTPUT_NET_CAP,
    CONSTRAINED_PIN_TRANSITION,
    RELATED_PIN_TRANSIITION,
    UNDEFINED_LU_TABLE_VARIABLE // default
};

enum Timing_Sense {
    NEGATIVE_UNATE = 0,
    POSITIVE_UNATE,
    NON_UNATE,
    UNDEFINED_TIMING_SENSE      // default
};

enum Timing_Type{
    RISING_EDGE = 0,
    FALLING_EDGE,
    SETUP_RISING,
    HOLD_RISING,
    COMBINATINAL                // default
};
