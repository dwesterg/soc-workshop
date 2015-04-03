//#include "sys/alt_errno.h"
#include "descriptor_regs.h"
#include "csr_regs.h"
#include "response_regs.h"
#include "sgdma_dispatcher.h"

#define ENOSPC          28      /* No space left on device */
#define ENODATA         61      /* No data available */
//#include "sys/alt_irq.h"


/*
 * Functions for constructing standard descriptors.  Unnecessary elements are
 * set to 0 for completeness and will be ignored by the hardware.
 */
int construct_standard_st_to_mm_descriptor (sgdma_standard_descriptor *descriptor, alt_u32 *write_address, alt_u32 length, alt_u32 control)
{
  descriptor->read_address = 0;
  descriptor->write_address = write_address;
  descriptor->transfer_length = length;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}


int construct_standard_mm_to_st_descriptor (sgdma_standard_descriptor *descriptor, alt_u32 *read_address, alt_u32 length, alt_u32 control)
{
  descriptor->read_address = read_address;
  descriptor->write_address = 0;
  descriptor->transfer_length = length;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}


int construct_standard_mm_to_mm_descriptor (sgdma_standard_descriptor *descriptor, alt_u32 *read_address, alt_u32 *write_address, alt_u32 length, alt_u32 control)
{
  descriptor->read_address = read_address;
  descriptor->write_address = write_address;
  descriptor->transfer_length = length;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}


/*
 * Functions for constructing extended descriptors.  If you disable some of the
 * extended features in the hardware then you should pass in 0 for that particular
 * descriptor element.  These disabled elements will not be buffered by the
 * dispatcher block.  With the new 64-bit addressing feature a new set of 64-bit
 * functions are provided.  Users who do not need 64-bit address support can
 * still use the 64-bit functions and not worry about hardware bloat because the
 * masters only generate enough address bits to span the memory they are connected
 * to.
 */
int construct_extended_st_to_mm_descriptor (sgdma_extended_descriptor *descriptor, alt_u32 *write_address, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 write_burst_count, alt_u16 write_stride)
{
  descriptor->read_address_low = 0;
  descriptor->write_address_low = write_address;
  descriptor->transfer_length = length;
  descriptor->sequence_number = sequence_number;
  descriptor->read_burst_count = 0;
  descriptor->write_burst_count = write_burst_count;
  descriptor->read_stride = 0;
  descriptor->write_stride = write_stride;
  descriptor->read_address_high = (alt_u32 *)0;
  descriptor->write_address_high = (alt_u32 *)0;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}

int construct_extended_st_to_mm_descriptor_64 (sgdma_extended_descriptor *descriptor, alt_u32 *write_address_low, alt_u32 *write_address_high, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 write_burst_count, alt_u16 write_stride)
{
  descriptor->read_address_low = 0;
  descriptor->write_address_low = write_address_low;
  descriptor->transfer_length = length;
  descriptor->sequence_number = sequence_number;
  descriptor->read_burst_count = 0;
  descriptor->write_burst_count = write_burst_count;
  descriptor->read_stride = 0;
  descriptor->write_stride = write_stride;
  descriptor->read_address_high = (alt_u32 *)0;
  descriptor->write_address_high = write_address_high;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}


int construct_extended_mm_to_st_descriptor (sgdma_extended_descriptor *descriptor, alt_u32 *read_address, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 read_burst_count, alt_u16 read_stride)
{
  descriptor->read_address_low = read_address;
  descriptor->write_address_low = 0;
  descriptor->transfer_length = length;
  descriptor->sequence_number = sequence_number;
  descriptor->read_burst_count = read_burst_count;
  descriptor->write_burst_count = 0;
  descriptor->read_stride = read_stride;
  descriptor->write_stride = 0;
  descriptor->read_address_high = (alt_u32 *)0;
  descriptor->write_address_high  = (alt_u32 *)0;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}

int construct_extended_mm_to_st_descriptor_64 (sgdma_extended_descriptor *descriptor, alt_u32 *read_address_low, alt_u32 *read_address_high, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 read_burst_count, alt_u16 read_stride)
{
  descriptor->read_address_low = read_address_low;
  descriptor->write_address_low = 0;
  descriptor->transfer_length = length;
  descriptor->sequence_number = sequence_number;
  descriptor->read_burst_count = read_burst_count;
  descriptor->write_burst_count = 0;
  descriptor->read_stride = read_stride;
  descriptor->write_stride = 0;
  descriptor->read_address_high = read_address_high;
  descriptor->write_address_high  = (alt_u32 *)0;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}


int construct_extended_mm_to_mm_descriptor (sgdma_extended_descriptor *descriptor, alt_u32 *read_address, alt_u32 *write_address, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 read_burst_count, alt_u8 write_burst_count, alt_u16 read_stride, alt_u16 write_stride)
{
  descriptor->read_address_low = read_address;
  descriptor->write_address_low = write_address;
  descriptor->transfer_length = length;
  descriptor->sequence_number = sequence_number;
  descriptor->read_burst_count = read_burst_count;
  descriptor->write_burst_count = write_burst_count;
  descriptor->read_stride = read_stride;
  descriptor->write_stride = write_stride;
  descriptor->read_address_high = (alt_u32 *)0;
  descriptor->write_address_high = (alt_u32 *)0;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}

int construct_extended_mm_to_mm_descriptor_64 (sgdma_extended_descriptor *descriptor, alt_u32 *read_address_low, alt_u32 *read_address_high, alt_u32 *write_address_low, alt_u32 *write_address_high, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 read_burst_count, alt_u8 write_burst_count, alt_u16 read_stride, alt_u16 write_stride)
{
  descriptor->read_address_low = read_address_low;
  descriptor->write_address_low = write_address_low;
  descriptor->transfer_length = length;
  descriptor->sequence_number = sequence_number;
  descriptor->read_burst_count = read_burst_count;
  descriptor->write_burst_count = write_burst_count;
  descriptor->read_stride = read_stride;
  descriptor->write_stride = write_stride;
  descriptor->read_address_high = read_address_high;
  descriptor->write_address_high = write_address_high;
  descriptor->control = control | DESCRIPTOR_CONTROL_GO_MASK;

  return 0;
}


/* 
 * Functions for writing descriptor structs to the dispatcher.  If you disable some of the
 * extended features in the hardware then you should pass in 0 for that particular
 * descriptor element.  These disabled elements will not be buffered by the
 * dispatcher block.
 * 
 * This function is non-blocking and will return an error code if there is no room to write
 * another descriptor to the dispatcher.  It is recommeneded to call 'read_descriptor_buffer_full'
 * and make sure it returns '0' before calling this function.
 */
int write_standard_descriptor (alt_u32 csr_base, alt_u32 descriptor_base, sgdma_standard_descriptor *descriptor)
{
  if ((RD_CSR_STATUS(csr_base) & CSR_DESCRIPTOR_BUFFER_FULL_MASK) != 0)
  {
    return -ENOSPC;  // at least one descriptor buffer is full, returning so that this function is non-blocking
  }
  
  WR_DESCRIPTOR_READ_ADDRESS(descriptor_base, (alt_u32)descriptor->read_address);
  WR_DESCRIPTOR_WRITE_ADDRESS(descriptor_base, (alt_u32)descriptor->write_address);
  WR_DESCRIPTOR_LENGTH(descriptor_base, descriptor->transfer_length);
  WR_DESCRIPTOR_CONTROL_STANDARD(descriptor_base, descriptor->control);
  return 0;
}


/*
 * This function is used for writing extented descriptors to the dispatcher.  It handles both
   32-bit and 64-bit addressing descriptors.
 */
int write_extended_descriptor (alt_u32 csr_base, alt_u32 descriptor_base, sgdma_extended_descriptor *descriptor)
{
  if ((RD_CSR_STATUS(csr_base) & CSR_DESCRIPTOR_BUFFER_FULL_MASK) != 0)
  {
    return -ENOSPC;  // at least one descriptor buffer is full, returning so that this function is non-blocking
  }
  
  WR_DESCRIPTOR_READ_ADDRESS(descriptor_base, (alt_u32)descriptor->read_address_low);
  WR_DESCRIPTOR_WRITE_ADDRESS(descriptor_base, (alt_u32)descriptor->write_address_low);
  WR_DESCRIPTOR_LENGTH(descriptor_base, descriptor->transfer_length);
  WR_DESCRIPTOR_SEQUENCE_NUMBER(descriptor_base, descriptor->sequence_number);
  WR_DESCRIPTOR_READ_BURST(descriptor_base, descriptor->read_burst_count);
  WR_DESCRIPTOR_WRITE_BURST(descriptor_base, descriptor->write_burst_count);
  WR_DESCRIPTOR_READ_STRIDE(descriptor_base, descriptor->read_stride);
  WR_DESCRIPTOR_WRITE_STRIDE(descriptor_base, descriptor->write_stride);
  WR_DESCRIPTOR_READ_ADDRESS_HIGH(descriptor_base, (alt_u32)descriptor->read_address_high);
  WR_DESCRIPTOR_WRITE_ADDRESS_HIGH(descriptor_base, (alt_u32)descriptor->write_address_high);
  WR_DESCRIPTOR_CONTROL_ENHANCED(descriptor_base, descriptor->control);
  return 0;
}


/* 
 * This function is used to read a response from the dispatcher when the
 * response port is configured as a memory mapped slave.  This function is
 * non-blocking so the response buffer will only be read when there is
 * a valid response buffered.  If there is no response bufferred an error
 * code is returned and it's up to the application to handle.  It is recommended
 * to use 'read_response_buffer_empty' and make sure it returns '0' before
 * calling this function
 */
int read_mm_response (alt_u32 csr_base, alt_u32 response_base, sgdma_response *response)
{
  alt_u32 errors;
  
  if ((RD_CSR_STATUS(csr_base) & CSR_RESPONSE_BUFFER_EMPTY_MASK) != 0)
  {
    return -ENODATA;  // response FIFO is empty
  }
  
  response->actual_bytes_transferred = RD_RESPONSE_ACTUAL_BYTES_TRANSFFERED(response_base);
  errors = RD_RESPONSE_ERRORS_REG(response_base);
  response->error = (errors & RESPONSE_ERROR_MASK) >> RESPONSE_ERROR_OFFSET;
  response->early_termination = (errors & RESPONSE_EARLY_TERMINATION_MASK) >> RESPONSE_EARLY_TERMINATION_OFFSET;
  
  return 0;
}


/* Functions for accessing the control and status port */
alt_u32 read_csr_status (alt_u32 csr_base)
{
  return RD_CSR_STATUS(csr_base);
}

alt_u32 read_csr_control (alt_u32 csr_base)
{
  return RD_CSR_CONTROL(csr_base);
}

alt_u16 read_csr_read_descriptor_buffer_fill_level (alt_u32 csr_base)
{
  return ((RD_CSR_DESCRIPTOR_FILL_LEVEL(csr_base) & CSR_READ_FILL_LEVEL_MASK) >> CSR_READ_FILL_LEVEL_OFFSET);
}

alt_u16 read_csr_write_descriptor_buffer_fill_level (alt_u32 csr_base)
{
  return ((RD_CSR_DESCRIPTOR_FILL_LEVEL(csr_base) & CSR_WRITE_FILL_LEVEL_MASK) >> CSR_WRITE_FILL_LEVEL_OFFSET);
}

alt_u16 read_csr_response_buffer_fill_level (alt_u32 csr_base)
{
  return ((RD_CSR_RESPONSE_FILL_LEVEL(csr_base) & CSR_RESPONSE_FILL_LEVEL_MASK) >> CSR_RESPONSE_FILL_LEVEL_OFFSET);
}

alt_u16 read_csr_read_sequence_number (alt_u32 csr_base)
{
  return ((RD_CSR_SEQUENCE_NUMBER(csr_base) & CSR_READ_SEQUENCE_NUMBER_MASK) >> CSR_READ_SEQUENCE_NUMBER_OFFSET);
}

alt_u16 read_csr_write_sequence_number (alt_u32 csr_base)
{
  return ((RD_CSR_SEQUENCE_NUMBER(csr_base) & CSR_WRITE_SEQUENCE_NUMBER_MASK) >> CSR_WRITE_SEQUENCE_NUMBER_OFFSET);
}


/* Helper functions to read/clear individual status registers */
alt_u32 read_busy (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_BUSY_MASK) != 0);  // returns '1' when the dispatcher is busy
}

alt_u32 read_descriptor_buffer_empty (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_DESCRIPTOR_BUFFER_EMPTY_MASK) != 0);  // returns '1' when both descriptor buffers are empty
}

alt_u32 read_descriptor_buffer_full (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_DESCRIPTOR_BUFFER_FULL_MASK) != 0);  // returns '1' when either descriptor buffer is full
}

alt_u32 read_response_buffer_empty (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_RESPONSE_BUFFER_EMPTY_MASK) != 0);  // returns '1' when the response buffer is empty
}

alt_u32 read_response_buffer_full (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_RESPONSE_BUFFER_FULL_MASK) != 0);  // returns '1' when the response buffer is full
}

alt_u32 read_stopped (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_STOP_STATE_MASK) != 0);  // returns '1' when the SGDMA is stopped (either due to application writing to the stop bit or an error condition stopped the SGDMA)
}

alt_u32 read_resetting (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_RESET_STATE_MASK) != 0);  // returns '1' when the SGDMA is in the middle of a reset (read/write masters are still resetting)
}

alt_u32 read_stopped_on_error (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_STOPPED_ON_ERROR_MASK) != 0);  // returns '1' when the SGDMA stopped due to an error entering the write master component (one of the conditions that will cause 'dispatcher_stopped' to return a '1')
}

alt_u32 read_stopped_on_early_termination (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_STOPPED_ON_EARLY_TERMINATION_MASK) != 0);  // returns '1' when the SGDMA stopped due to the eop not arriving at the write master streaming port before the length counter reaches 0 (one of the conditions that will cause 'dispatcher_stopped' to return a '1')
}

alt_u32 read_irq (alt_u32 csr_base)
{
  return ((RD_CSR_STATUS(csr_base) & CSR_IRQ_SET_MASK) != 0);  // returns '1' when the SGDMA is asserting the interrupt signal (no pre-fetching descriptor master)
}

void clear_irq (alt_u32 csr_base)
{
  WR_CSR_STATUS(csr_base, CSR_IRQ_SET_MASK);  // the status register is read/clear only so a read-modify-write is not necessary
}


/* Helper functions for writting the individual control registers */
void stop_dispatcher (alt_u32 csr_base)
{
  alt_u32 temporary_control;
 // alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) | CSR_STOP_MASK;  // setting the stop mask bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}

void start_dispatcher (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) & (CSR_STOP_MASK ^ 0xFFFFFFFF);  // reseting the stop mask bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}

void reset_dispatcher (alt_u32 csr_base)
{
  WR_CSR_CONTROL(csr_base, CSR_RESET_MASK);  // setting the reset bit, no need to read the control register first since this write is going to clear it out
}

void enable_stop_on_error (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) | CSR_STOP_ON_ERROR_MASK;  // setting the stop on error mask bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}

void disable_stop_on_error (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) & (CSR_STOP_ON_ERROR_MASK ^ 0xFFFFFFFF);  // reseting the stop on error mask bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}

void enable_stop_on_early_termination (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) | CSR_STOP_ON_EARLY_TERMINATION_MASK;  // setting the stop on early termination mask bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}

void disable_stop_on_early_termination (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) & (CSR_STOP_ON_EARLY_TERMINATION_MASK ^ 0xFFFFFFFF);  // resetting the stop on early termination mask bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}

void enable_global_interrupt_mask (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) | CSR_GLOBAL_INTERRUPT_MASK;  // setting the global interrupt mask bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}

void disable_global_interrupt_mask (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) & (CSR_GLOBAL_INTERRUPT_MASK ^ 0xFFFFFFFF);  // resetting the global interrupt mask bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}


void stop_descriptors (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) | CSR_STOP_DESCRIPTORS_MASK;  // setting the stop descriptors bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}


void start_descriptors (alt_u32 csr_base)
{
  alt_u32 temporary_control;
//  alt_irq_context context = alt_irq_disable_all();  // making sure the read-modify-write below can't be pre-empted
  temporary_control = RD_CSR_CONTROL(csr_base) & (CSR_STOP_DESCRIPTORS_MASK ^ 0xFFFFFFFF);  // resetting the stop descriptors bit
  WR_CSR_CONTROL(csr_base, temporary_control);
//  alt_irq_enable_all(context);
}

