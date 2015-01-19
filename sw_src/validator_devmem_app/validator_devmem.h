//
// physical base address of the validator hardware
//
#define VALIDATOR_PHYS_BASE	(0xFF200000)

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

//
// usage string
//
#define USAGE_STR "\
\n\
Usage: validator_devmem [ONE-OPTION-ONLY]\n\
  -s, --sign-message <input file> [-o, --output-file <output file>]\n\
  -a, --auth-message <input file>\n\
  -d, --dump-random-numbers  [-o, --output-file <output file>]\n\
  -e, --dump-entropy-counters-lo  [-o, --output-file <output file>]\n\
  -E, --dump-entropy-counters-hi  [-o, --output-file <output file>]\n\
  -g, --get-entropy-counters-state\n\
  -t, --start-entropy-counters\n\
  -p, --stop-entropy-counters\n\
  -r, --reset-entropy-counters\n\
  -h, --help\n\
\n\
"  

//
// help string
//
#define HELP_STR "\
\n\
Only one of the following options may be passed in per invocation:\n\
\n\
  -s, --sign-message <input file> [-o, --output-file <output file>]\n\
Pass an input file containing exactly 32 bytes with this argument, the\n\
validator will sign the 32 byte message returning a 96 byte message containing\n\
the 32 byte local identity information, the original 32 byte input message and\n\
a 32 byte HMAC for the entire catenated 64 byte message.  The output is\n\
displayed on stdout in ascii, and if you specify an --output-file the binary\n\
data will be stored into that file.\n\
\n\
  -a, --auth-message <input file>\n\
Pass an input file containing exactly 96 bytes with this argument, the\n\
validator will verify that the first 64 byte message produces the same HMAC as\n\
the last 32 bytes of the input message.  Returns success if HMAC is authentic,\n\
or failure if HMAC is different.\n\
\n\
  -d, --dump-random-numbers  [-o, --output-file <output file>]\n\
The validator will output 512 bytes of random numbers on stdout in ascii.  If\n\
you specify an --output-file the binary data will be stored into that file.\n\
\n\
  -e, --dump-entropy-counters-lo  [-o, --output-file <output file>]\n\
The validator will dump the lower 128 counter values from the entropy counter\n\
to stdout in ascii.  If you specify an --output-file the same ascii information\n\
will be stored into that file as well.\n\
\n\
  -E, --dump-entropy-counters-hi  [-o, --output-file <output file>]\n\
The validator will dump the upper 128 counter values from the entropy counter\n\
to stdout in ascii.  If you specify an --output-file the same ascii information\n\
will be stored into that file as well.\n\
\n\
  -g, --get-entropy-counters-state\n\
The validator will return the state of the entropy counter on stdout.  The\n\
reported state is either ENABLED or DISABLED.\n\
\n\
  -t, --start-entropy-counters\n\
The validator will start the entropy counter.\n\
\n\
  -p, --stop-entropy-counters\n\
The validator will stop the entropy counter.\n\
\n\
  -r, --reset-entropy-counters\n\
The validator will reset the entropy counter.\n\
\n\
  -h, --help\n\
Display this help message.\n\
\n\
"  

