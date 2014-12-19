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

#include <stdlib.h>
#include <string.h>

#include "mm_debug_link_mock.h"
#include "dprint.h"

static int ident_value[4] = {
  0x01010101,
  0x02020202,
  0x03030303,
  0x04040404,
};

mm_debug_link_interface *get_mm_debug_link(void)
{
  return new mm_debug_link_mock();
}

// Warning: only one open driver instance at a time.
const char *const filename_template = "/tmp/mmlXXXXXX";
char filename[] = "/tmp/mmlXXXXXX";
int mm_debug_link_mock::open(void)
{
  m_fd = -1;
  
  strcpy(filename, filename_template);
  m_fd = mkstemp(filename);
  DPRINT("%s %s(): opened '%s'\n", __FILE__, __func__, filename);
  if (m_fd == -1)
    return m_fd;
  return 0;
}

ssize_t mm_debug_link_mock::read(void *buf, size_t count)
{
  return ::read(m_fd, buf, count);
}

ssize_t mm_debug_link_mock::write(const void *buf, size_t count)
{
  ssize_t written = 0;
  DPRINT("%s %s(): writing %d bytes\n", __FILE__, __func__, count);
  written = ::write(m_fd, buf, count);

  return written;
}

void mm_debug_link_mock::close(void)
{
  DPRINT("%s %s(): closing and deleting '%s'\n", __FILE__, __func__, filename);
  if (m_fd != -1)
    ::close(m_fd);
  m_fd = -1;
  unlink(filename);
}

void mm_debug_link_mock::write_ident(int val)
{
  DPRINT("%s %s(%d)\n", __FILE__, __func__, val);
  ident_value[3] ^= (val << 28);
}

void mm_debug_link_mock::reset(bool val)
{
  DPRINT("%s %s(%d)\n", __FILE__, __func__, val);
}

void mm_debug_link_mock::ident(int id[4])
{
  for (int i = 0; i < 4; ++i)
    id[i] = ident_value[i];
}

void mm_debug_link_mock::enable(int channel, bool state)
{
  DPRINT("%s %s(%d, %d)\n", __FILE__, __func__, state, channel);
}

