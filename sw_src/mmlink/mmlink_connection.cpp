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
#include <string.h>
#include <sys/param.h>

#include "dprint.h"
#include "mmlink_connection.h"

const char *mmlink_connection::UNKNOWN = "UNKNOWN\n";
const char *mmlink_connection::OK = "OK\n";

// return value:
//   0: everything A-OK
//   negative: error code
int mmlink_connection::handle_receive()
{
  int fail = 0;
  int size = 0;
  int conn = this->get_socket();

  int bytes_to_receive = m_h2t_bufsize - m_h2t_buf_end;
  if (bytes_to_receive == 0)
  {
    // No room for more data, so exit.
    return 0;
  }

  size = ::recv(conn, m_h2t_buf + m_h2t_buf_end, bytes_to_receive, 0);
  if (size < 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      // Nothing to do, but no error.
      fail = 0;
    }
    else
    {
      DPRINT("%s: error on socket %d: %d (%s)\n", __func__, conn, errno, strerror(errno));
      fail = -errno;
    }
  } 
  else if (size == 0)
  {
  
    DPRINT("handle_receive (socket: %d): ::recv() returned 0\n", conn);
    fail = -1;
  }
  else
  {
    m_h2t_buf_end += size;
  }

  return fail;
}

size_t mmlink_connection::send_all(const char *msg, const size_t msg_len)
{
  size_t bytes_sent = 0;

  while (bytes_sent < msg_len)
  {
    int len = send(msg + bytes_sent, msg_len - bytes_sent);
    if (len > 0)
    {
      bytes_sent += len;
    }
    else
    {
      DPRINT("send failed: %d (%s)\n", errno, strerror(errno));	
      break;
    }
  }

  return bytes_sent;
}

size_t mmlink_connection::send(const char *msg, const size_t msg_len)
{
  size_t len;

  len = ::send(m_fd, msg, msg_len, 0);
  return len;
}

int mmlink_connection::handle_management()
{
  int i, start;
  size_t rem;
  int fail = 0;

  i = 0;
  start = 0;
  for (i = 0; i < m_h2t_buf_end; ++i)
  {
    if (m_h2t_buf[i] == '|') 
    {
      DPRINT("found a pipe\n");
      // If bound, set to data mode
      if (is_bound())
      {
        set_is_data();
        return 0;
      }

      // If not bound, close.
      DPRINT("%d: rejecting attempt to convert unbound connection to data.\n", get_socket());
      fail = -1;
      break;
    }
    else if (m_h2t_buf[i] == '\n' || m_h2t_buf[i] == '\r')
    {
      m_h2t_buf[i] = '\0';
      if (handle_management_command(m_h2t_buf + start))
      {
        // Pass the failure upward.
        fail = -1;
        return fail;
      }
      else
      {
        // point to the next command
        start = i + 1;
      }
    }
  }
  // Transfer any remaining unprocessed bytes to the start of the buffer.
  rem = m_h2t_buf_end - start;
  if (rem > 0)
    memmove(m_h2t_buf, m_h2t_buf + start, rem);
  m_h2t_buf_end = rem;

  // success
  return fail;
}

// Handle a single management connection command.
// cmd is a null-terminated string.
// return value: 0 on success, non-zero on failure.
int mmlink_connection::handle_management_command(char *cmd)
{
  int fail = 0;

  DPRINT("mmlink_connection::handle_management_command('%s')\n", cmd);
  // Ignore empty string.
  if (!*cmd)
    return 0;

  if (!this->is_bound())
    fail = this->handle_unbound_command(cmd);
  else
    fail = this->handle_bound_command(cmd);

  return fail;
}

int mmlink_connection::handle_unbound_command(char *cmd)
{
  int fail = 0;
  //
  // Only HANDLE=xxxxxxxx is allowed
  // If wrong handle value, close
  // if any other input, close
  char expect_handle[] = "HANDLE 01234567";
  sprintf(expect_handle + strlen("HANDLE "), "%08X", get_server_id());
  if (0 == strcmp(expect_handle, cmd))
  {
    DPRINT("%d: accepted handle value ('%s'), setting to bound state\n", get_socket(), cmd);

    bind();
    send(OK, strlen(OK));
  }
  else
  {
    DPRINT("%d: closing socket: incorrect HANDLE value (expected: '%s'; got: '%s')\n", get_socket(), expect_handle, cmd);
    fail = -1;
  }

  return fail;
}

int mmlink_connection::handle_data()
{
  m_h2t_buf[m_h2t_buf_end] = '\0';
  DPRINT("%d (data): ", get_socket());
  for (int i = 0; i < m_h2t_buf_end; ++i) 
  {
    DPRINT("%02X ", m_h2t_buf[i]);
  }
  DPRINT("\n");
  m_h2t_buf_end = 0;
}

int mmlink_connection::handle_bound_command(char *cmd)
{
  int arg1, arg2;
  bool unknown = true;

  if (1 == sscanf(cmd, "IDENT %X", &arg1))
  {
    if (arg1 >= 0 && arg1 <= 0xF) {
      int ident[4]; 
      size_t msg_len = 64;
      char msg[msg_len + 1];

      // Write the nibble value
      driver()->write_ident(arg1);
      driver()->ident(ident);
      snprintf(msg, msg_len, "%08X%08X%08X%08X\n",
        ident[3], ident[2], ident[1], ident[0],
        get_server_id());

      send(msg, strlen(msg));
      unknown = false;
    }
  }
  else if (1 == sscanf(cmd, "RESET %d", &arg1))
  {
    if (arg1 == 0 || arg1 == 1)
    {
      driver()->reset(arg1);
      send(OK, strlen(OK));
      unknown = false;
    }
  }
  else if (2 == sscanf(cmd, "ENABLE %d %d", &arg1, &arg2))
  {
    if (arg1 >= 0 && (arg2 == 0 || arg2 == 1))
    {
      driver()->enable(arg1, arg2);
      send(OK, strlen(OK));
      unknown = false;
    }
  }
  else if (0 == strncmp(cmd, "NOOP", 4))
  {
    send(OK, strlen(OK));
    unknown = false;
  }

  if (unknown)
    send(UNKNOWN, strlen(UNKNOWN));

  return 0;
}


