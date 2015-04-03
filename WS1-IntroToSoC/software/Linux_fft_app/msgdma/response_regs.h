/*
  The response slave port only carries the actual bytes transferred,
  error, and early termination bits.  Reading from the upper most byte
  of the 2nd register pops the response FIFO.  For proper FIFO popping
  always read the actual bytes transferred followed by the error and early
  termination bits using 'little endian' accesses.  If a big endian
  master accesses the response slave port make sure that address 0x7 is the
  last byte lane access as it's the one that pops the reponse FIFO.
  
  If you use a pre-fetching descriptor master in front of the dispatcher
  port then you do not need to access this response slave port. 
*/

#ifndef RESPONSE_REGS_H_
#define RESPONSE_REGS_H_

//#include "io.h"

#define RESPONSE_ACTUAL_BYTES_TRANSFERRED_REG    (0x0)
#define RESPONSE_ERRORS_REG                      (0x4)

// bits making up the "errors" register
#define RESPONSE_ERROR_MASK                      (0xFF)
#define RESPONSE_ERROR_OFFSET                    (0)
#define RESPONSE_EARLY_TERMINATION_MASK          (1<<8)
#define RESPONSE_EARLY_TERMINATION_OFFSET        (8)


// read macros for each 32 bit register
#define RD_RESPONSE_ACTUAL_BYTES_TRANSFFERED(base)     *(unsigned long *)(base + RESPONSE_ACTUAL_BYTES_TRANSFERRED_REG)
#define RD_RESPONSE_ERRORS_REG(base)                   *(unsigned long *)(base + RESPONSE_ERRORS_REG)


#endif /*RESPONSE_REGS_H_*/
