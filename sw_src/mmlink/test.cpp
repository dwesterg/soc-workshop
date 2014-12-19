/*
 * Copyright (c) 2014, Altera Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Altera Corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "dprint.h"
#include "mm_debug_link_interface.h"
#include "udp_log.h"

volatile bool run = true;
void int_handler(int sig)
{
  DPRINT("SIGINT: stopping\n");
  run = false;
}

size_t packet_size(size_t byte_count)
{
  return byte_count + 4;
}

unsigned char *packetize(unsigned char *the_bytes, size_t len, int channel)
{
  size_t packet_len = packet_size(len);
  unsigned char *packet = new unsigned char[packet_len];
  packet[0] = 0x7C;
  packet[1] = (unsigned char)channel;
  packet[2] = 0x7A;
  for (int i = 0; i < len; ++i)
    packet[i + 3] = the_bytes[i];

  packet[packet_len - 1] = packet[packet_len - 2];
  packet[packet_len - 2] = 0x7B;

  return packet;
}

unsigned char *get_hub_query_cmd(size_t byte_count, size_t *buf_size)
{
  unsigned char hub_query_cmd[] = {
    0x01, 0x85, 0xf0, 0x10, 0x8f, 0x30, 0x0e, 0x00, 0x60, 0x82, 0x10, 0xde, 0x01, 0x81, 0x30, 0x8f, 0x30,
    0x0c, 0x00, 0x60, 0x82, 0x10, 0x43, 0x00, 0x80, 0xa2, 0x00, 0x00, 0x81, 0x30, 0x82, 0x10, 0x43, 0x00,
    0x80, 0xa2, 0x00, 0x00, 0x81, 0x30, 0x82, 0x10, 0x43, 0x00, 0x80, 0xa2, 0x00, 0x00, 0x81, 0x30, 0x82,
    0x10, 0x43, 0x00, 0x80, 0xa2, 0x00, 0x00, 0x81, 0x30, 0x82, 0x10, 0x43, 0x00, 0x80, 0xa2, 0x00, 0x00,
    0x81, 0x30, 0x82, 0x10, 0x43, 0x00, 0x80, 0xa2, 0x00, 0x00, 0x81, 0x30, 0x82, 0x10, 0x43, 0x00, 0x80,
    0xa2, 0x00, 0x00, 0x81, 0x30, 0x82, 0x10, 0x43, 0x00, 0xc0, 0xa2, 0x00, 0x00, 0x81, 0x30,
  };

  size_t len = sizeof(hub_query_cmd) / sizeof(*hub_query_cmd);
  *buf_size = packet_size(len);
  return packetize(hub_query_cmd, len, 1);
}

unsigned char *get_write_buf(size_t byte_count, size_t *buf_size)
{
  size_t size = byte_count + 12;
  *buf_size = 0;
  // if (size > sizeof(my_global_buffer) / sizeof(*my_global_buffer))
  //   return NULL;

  unsigned char *buf = new unsigned char[size];
  if (!buf)
  	return buf;
  
  buf[0] = 0x7C;
  buf[1] = 0x01;
  buf[2] = 0x7A;
  buf[3] = 0x04;
  buf[4] = 0x00;
  buf[5] = byte_count >> 8;
  buf[6] = byte_count & 0xFF;
  buf[7] = 0x00;
  buf[8] = 0x00;
  buf[9] = 0x00;
  buf[10] = 0x00;

  for (int i = 0; i < byte_count; ++i)
  {
    buf[11 + i] = (i + 1) & 0x3F;
  }
  buf[size - 1] = buf[size - 2];
  buf[size - 2] = 0x7B;

  *buf_size = size;
  return buf;
}

int do_read(mm_debug_link_interface *driver, unsigned char *read_buf, size_t read_buf_size)
{
  int total_read = 0;
  int len;
  do
  {
    len = driver->read(read_buf + total_read, read_buf_size - total_read);
    if (len >= 0)
    {
      DPRINT("read %d bytes\n", len);
      total_read += len;
    }

    if (len < 0)
    {
      DPRINT("read return: %d (errno: %d, '%s')\n", len, errno, strerror(errno));
    }

  } while (len > 0);
  for (int printed_bytes = 0; printed_bytes < total_read; ++printed_bytes)
    DPRINT_RAW("0x%02X ", read_buf[printed_bytes]);
  DPRINT_RAW("\n");

  return 0;
}

int do_test(mm_debug_link_interface *driver, unsigned char *write_buf, size_t write_buf_size, unsigned char *read_buf, size_t read_buf_size)
{

  bool have_read = false;
  bool have_written = false;
  int lim = 1000;
  int len;

  DPRINT("do_test(write_buf_size: %d; read_buf_size: %d)\n", write_buf_size, read_buf_size);

  // First, read (something might be available).
  //
  do_read(driver, read_buf, read_buf_size);
  
  // Next, write the entire buffer.
  int total_written = 0;
  do {
    ssize_t written = driver->write(write_buf + total_written, write_buf_size - total_written);
    if (written < 0)
    {
      DPRINT("driver->write() returned %d\n", written);
      break;
    }
    else if (written == 0)
    {
      DPRINT("driver->write() returned %d\n", written);
      break;
    }
    else 
    {
      DPRINT("wrote %d bytes to driver\n", written);
      for (int i = 0; i < written; ++i)
        DPRINT_RAW("0x%02X ", *(write_buf + total_written + i));
      DPRINT_RAW("\n");

      total_written += written;
    }
  } while (total_written < write_buf_size);

  // and a final read.
  //
  do_read(driver, read_buf, read_buf_size);

/*
  while (run && (!have_read || !have_written))
  {
    DPRINT("(have_read, have_written): (%d, %d)\n", have_read, have_written);
    if (have_written && !have_read)
    {
      len = driver->read(read_buf, read_buf_size);
      DPRINT("read %d bytes from driver\n", len);
      if (len > 0)
      {
        for (int printed_bytes = 0; printed_bytes < len; ++printed_bytes)
          DPRINT_RAW("0x%02X ", read_buf[printed_bytes]);
        DPRINT_RAW("\n");
        have_read = true;
        DPRINT("done reading\n");
      }
    }

    if (!have_written)
    {
      ssize_t written = driver->write(write_buf + total_written, write_buf_size - total_written);
      DPRINT("write %d bytes to driver\n", written);
      if (written < 0)
      {
        DPRINT("driver->write() returned %d\n", written);
      }
      if (written == 0)
      {
        DPRINT("driver->write() returned %d\n", written);
      }
      total_written += written;
      if (total_written == write_buf_size)
      {
        DPRINT("done writing\n");
        have_written = true;
      }
    }

    if (--lim == 0)
    {     
      run = false;
      DPRINT("hit the limit\n");
    }
  }
*/
}

int main(int argc, char **argv)
{
  int err = 0;

  size_t write_byte_count = 1024;
  size_t read_byte_count = 1024;
  size_t iterations = 1;

  if (argc >= 2)
  {
    sscanf(argv[1], "%u", &write_byte_count);
    if (argc >= 3)
    {
      sscanf(argv[2], "%u", &iterations);
    }
  }

  unsigned char *write_buf = NULL;
  size_t write_buf_size = 0;
//   write_buf = get_write_buf(write_byte_count, &write_buf_size);
  write_buf = get_hub_query_cmd(write_byte_count, &write_buf_size);

  if (!write_buf)
  {
    fprintf(stderr, "failed to allocate memory.\n");
    return -1;
  }

  unsigned char *read_buf = new unsigned char[read_byte_count];

  signal(SIGINT, int_handler);
  mm_debug_link_interface *driver = get_mm_debug_link();

  if (err = driver->open())
  {
    fprintf(stderr, "failed to init driver (%d).\n", err);
    return err;
  }

  driver->reset(true);
  driver->reset(false);
  driver->enable(1, 1);

  for (int i = 0; i < iterations; ++i)
  {
    do_test(driver, write_buf, write_buf_size, read_buf, read_byte_count);
  }

  if (write_buf)
    delete write_buf;

  if (read_buf)
    delete read_buf;

  driver->close();
  delete driver;

  DPRINT("goodbye.\n");
  return 0;
}

