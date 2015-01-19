//
// physical base address of the validator hardware
//
//#define VALIDATOR_PHYS_BASE	(0xFF200000)

//
// handshake ram structure offsets
//
#define HS_FLAGS_OFFSET         (0x00)
#define HS_FLAGS_SIZE           (0x04)
#define ARM_REQ_OFFSET          (HS_FLAGS_OFFSET + HS_FLAGS_SIZE)
#define ARM_REQ_SIZE            (0x04)
#define LAST_REQ_OFFSET         (ARM_REQ_OFFSET + ARM_REQ_SIZE)
#define LAST_REQ_SIZE           (0x04)
#define RESULT_OFFSET           (LAST_REQ_OFFSET + LAST_REQ_SIZE)
#define RESULT_SIZE             (0x04)
#define MESSAGE_OUT_OFFSET      (RESULT_OFFSET + RESULT_SIZE)
#define MESSAGE_OUT_SIZE        (0x20)
#define MESSAGE_IN_OFFSET       (MESSAGE_OUT_OFFSET + MESSAGE_OUT_SIZE)
#define MESSAGE_IN_SIZE         (0x20)
#define HMAC_OUT_OFFSET         (MESSAGE_IN_OFFSET + MESSAGE_IN_SIZE)
#define HMAC_OUT_SIZE           (0x20)
#define UPPER_HALF_OFFSET       (0x200)
#define UPPER_HALF_SIZE         (0x200)

//
// hs flag values
//
#define ARM_OWNS_FLAG           (0x00)
#define NIOS_OWNS_FLAG          (0xFF)

//
// ARM request values
//
#define SIGN_MESSAGE_REQ                (1)
#define AUTHENTICATE_MESSAGE_REQ        (2)
#define DUMP_RANDOM_NUMBERS_REQ         (3)
#define DUMP_ENTROPY_COUNTERS_LO_REQ	(4)
#define DUMP_ENTROPY_COUNTERS_HI_REQ    (5)
#define GET_ENTROPY_COUNTERS_STATE_REQ  (6)
#define START_ENTROPY_COUNTERS_REQ      (7)
#define STOP_ENTROPY_COUNTERS_REQ       (8)
#define RESET_ENTROPY_COUNTERS_REQ      (9)

