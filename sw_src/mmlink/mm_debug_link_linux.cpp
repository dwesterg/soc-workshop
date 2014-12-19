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

#include <errno.h>
#include <fcntl.h>
#include <linux/mm-debug-link.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mm_debug_link_linux.h"
#include "dprint.h"

#define DRIVER_PATH "/dev/mm_debug_link"
#define B2P_EOP 0x7B

mm_debug_link_interface *get_mm_debug_link(void)
{
  return new mm_debug_link_linux();
}

int mm_debug_link_linux::open(void)
{
  struct stat sts;

  m_fd = -1;

  if ((stat(DRIVER_PATH, &sts)) == -1)
  {
    DPRINT("[MM Link Task] Failed to open %s. MM Debug Link Driver may not be loaded.\n", DRIVER_PATH);
    exit(EXIT_FAILURE);
  }
  m_fd = ::open(DRIVER_PATH, O_RDWR);

  if (m_fd < 0)
  {
    DPRINT("open() failed: %d (%s)\n", errno, strerror(errno));
    return m_fd;
  }
  return 0;
}

ssize_t mm_debug_link_linux::read(void *buf, size_t count)
{
  ssize_t len =::read(m_fd, buf, count); 
  return len;
}

ssize_t mm_debug_link_linux::write(const void *buf, size_t count)
{
  ssize_t written = 0;
  written = ::write(m_fd, buf, count);

//  DPRINT("%s %s(): wrote %d bytes\n", __FILE__, __func__, written);
//  DPRINT("%s %s(): wanted to write %d bytes\n", __FILE__, __func__, count);
//  DPRINT_RAW("\t");
//  for (int i = 0; i < count; ++i)
//    DPRINT_RAW("%02X ", *((char*)buf + i));
//  DPRINT_RAW("\n");

  return written;
}

void mm_debug_link_linux::close(void)
{
  DPRINT("mm_debug_link_linux::close\n");
  if (m_fd != -1)
    ::close(m_fd);
  m_fd = -1;
}

void mm_debug_link_linux::write_ident(int val)
{
  if (ioctl(m_fd, MM_DEBUG_LINK_IOCTL_WRITE_MIXER, val) == -1)
  {
    DPRINT("[MM Link Task] ioctl (0x%X) failed: %d (%s)\n", MM_DEBUG_LINK_IOCTL_WRITE_MIXER, errno, strerror(errno));
  }
}

void mm_debug_link_linux::reset(bool val)
{
  if (ioctl(m_fd, MM_DEBUG_LINK_IOCTL_DEBUG_RESET, val) == -1)
  {
    DPRINT("[MM Link Task] ioctl (0x%X) failed: %d (%s)\n", MM_DEBUG_LINK_IOCTL_DEBUG_RESET, errno, strerror(errno));
  }
}

void mm_debug_link_linux::ident(int id[4])
{
  if (ioctl(m_fd, MM_DEBUG_LINK_IOCTL_READ_ID, id) == -1)
  {
    DPRINT("[MM Link Task] ioctl (0x%X) failed: %d (%s)\n", MM_DEBUG_LINK_IOCTL_READ_ID, errno, strerror(errno));
  }
}

void mm_debug_link_linux::enable(int channel, bool state)
{
  int encoded_cmd = (channel << 8) | (state ? 1 : 0);
  if (ioctl(m_fd, MM_DEBUG_LINK_IOCTL_ENABLE, encoded_cmd) == -1)
  {
    DPRINT("[MM Link Task] ioctl (0x%X) failed: %d (%s)\n", MM_DEBUG_LINK_IOCTL_ENABLE, errno, strerror(errno));
  }
}

