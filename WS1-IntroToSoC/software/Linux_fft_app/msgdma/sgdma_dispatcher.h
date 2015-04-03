#ifndef SGDMA_DISPATCHER_H_
#define SGDMA_DISPATCHER_H_


#include "alt_types.h"


// need these two for the Nios II IDE
#define SGDMA_DISPATCHER_INSTANCE(name, dev) extern int alt_no_storage
#define SGDMA_DISPATCHER_INIT(name, dev) while (0)


#define STANDARD_DESCRIPTOR_SIZE 16
#define EXTENDED_DESCRIPTOR_SIZE 32
#define RESPONSE_SIZE 8

#define sgdma_standard_descriptor_packed __attribute__ ((packed, aligned(STANDARD_DESCRIPTOR_SIZE)))
#define sgdma_extended_descriptor_packed __attribute__ ((packed, aligned(EXTENDED_DESCRIPTOR_SIZE)))
#define sgdma_response_packed __attribute__ ((packed, aligned(RESPONSE_SIZE)))


// use this structure if you haven't enabled the enhanced features
typedef struct {
  alt_u32 *read_address;
  alt_u32 *write_address;
  alt_u32 transfer_length;
  alt_u32 control;
} sgdma_standard_descriptor_packed sgdma_standard_descriptor;


// use ths structure if you have enabled the enhanced features (only the elements enabled in hardware will be used)
typedef struct {
  alt_u32 *read_address_low;
  alt_u32 *write_address_low;
  alt_u32 transfer_length;
  alt_u16 sequence_number;
  alt_u8  read_burst_count;
  alt_u8  write_burst_count;
  alt_u16 read_stride;
  alt_u16 write_stride;
  alt_u32 *read_address_high;
  alt_u32 *write_address_high;
  alt_u32 control;
} sgdma_extended_descriptor_packed sgdma_extended_descriptor;


// this struct should only be used if response information is enabled
typedef struct {
  alt_u32 actual_bytes_transferred;
  alt_u8 error;
  alt_u8 early_termination;  
} sgdma_response_packed sgdma_response;


/* public prototypes for use by the application */

// descriptor construction functions including 64-bit addressing extended versions
int construct_standard_st_to_mm_descriptor (sgdma_standard_descriptor *descriptor, alt_u32 *write_address, alt_u32 length, alt_u32 control);
int construct_standard_mm_to_st_descriptor (sgdma_standard_descriptor *descriptor, alt_u32 *read_address, alt_u32 length, alt_u32 control);
int construct_standard_mm_to_mm_descriptor (sgdma_standard_descriptor *descriptor, alt_u32 *read_address, alt_u32 *write_address, alt_u32 length, alt_u32 control);
int construct_extended_st_to_mm_descriptor (sgdma_extended_descriptor *descriptor, alt_u32 *write_address, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 write_burst_count, alt_u16 write_stride);
int construct_extended_mm_to_st_descriptor (sgdma_extended_descriptor *descriptor, alt_u32 *read_address, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 read_burst_count, alt_u16 read_stride);
int construct_extended_mm_to_mm_descriptor (sgdma_extended_descriptor *descriptor, alt_u32 *read_address, alt_u32 *write_address, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 read_burst_count, alt_u8 write_burst_count, alt_u16 read_stride, alt_u16 write_stride);
int construct_extended_st_to_mm_descriptor_64 (sgdma_extended_descriptor *descriptor, alt_u32 *write_address_low, alt_u32 *write_address_high, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 write_burst_count, alt_u16 write_stride);
int construct_extended_mm_to_st_descriptor_64 (sgdma_extended_descriptor *descriptor, alt_u32 *read_address_low, alt_u32 *read_address_high, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 read_burst_count, alt_u16 read_stride);
int construct_extended_mm_to_mm_descriptor_64 (sgdma_extended_descriptor *descriptor, alt_u32 *read_address_low, alt_u32 *read_address_high, alt_u32 *write_address_low, alt_u32 *write_address_high, alt_u32 length, alt_u32 control, alt_u16 sequence_number, alt_u8 read_burst_count, alt_u8 write_burst_count, alt_u16 read_stride, alt_u16 write_stride);


// descriptor write (to dispatcher) functions
int write_standard_descriptor (alt_u32 csr_base, alt_u32 descriptor_base, sgdma_standard_descriptor *descriptor);
int write_extended_descriptor (alt_u32 csr_base, alt_u32 descriptor_base, sgdma_extended_descriptor *descriptor);


// response reading function
int read_mm_response (alt_u32 csr_base, alt_u32 response_base, sgdma_response *response);


// control and status access functions
alt_u32 read_csr_status (alt_u32 csr_base);
alt_u32 read_csr_control (alt_u32 csr_base);
alt_u16 read_csr_read_descriptor_buffer_fill_level (alt_u32 csr_base);
alt_u16 read_csr_write_descriptor_buffer_fill_level (alt_u32 csr_base);
alt_u16 read_csr_response_buffer_fill_level (alt_u32 csr_base);
alt_u16 read_csr_read_sequence_number (alt_u32 csr_base);
alt_u16 read_csr_write_sequence_number (alt_u32 csr_base);


// functions for reading/clearing individual status register bits from the CSR port
alt_u32 read_busy (alt_u32 csr_base);
alt_u32 read_descriptor_buffer_empty (alt_u32 csr_base);
alt_u32 read_descriptor_buffer_full (alt_u32 csr_base);
alt_u32 read_response_buffer_empty (alt_u32 csr_base);
alt_u32 read_response_buffer_full (alt_u32 csr_base);
alt_u32 read_stopped (alt_u32 csr_base);
alt_u32 read_resetting (alt_u32 csr_base);
alt_u32 read_stopped_on_error (alt_u32 csr_base);
alt_u32 read_stopped_on_early_termination (alt_u32 csr_base);
alt_u32 read_irq (alt_u32 csr_base);
void clear_irq (alt_u32 csr_base);


// functions for setting/reseting individual control register bits for the CSR port
void stop_dispatcher (alt_u32 csr_base);
void start_dispatcher (alt_u32 csr_base);
void reset_dispatcher (alt_u32 csr_base);
void enable_stop_on_error (alt_u32 csr_base);
void disable_stop_on_error (alt_u32 csr_base);
void enable_stop_on_early_termination (alt_u32 csr_base);
void disable_stop_on_early_termination (alt_u32 csr_base);
void enable_global_interrupt_mask (alt_u32 csr_base);
void disable_global_interrupt_mask (alt_u32 csr_base);
void stop_descriptors (alt_u32 csr_base);
void start_descriptors (alt_u32 csr_base);


#endif /*SGDMA_DISPATCHER_H_*/
