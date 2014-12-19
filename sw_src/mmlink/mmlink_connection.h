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

#ifndef MMLINK_CONNECTION_H
#define MMLINK_CONNECTION_H

#include <sys/socket.h>
#include <unistd.h>

#include "mm_debug_link_interface.h"
#include "mmlink_server.h"

class mmlink_connection
{
public:
  // m_h2t_bufsize is the size of the buffer for h2t data
  // m_t2h_bufsize is the size of the buffer for t2h data
  mmlink_connection(mmlink_server *server) : m_h2t_bufsize(3000), m_t2h_bufsize(3000) { m_h2t_buf = new char[m_h2t_bufsize]; m_t2h_buf = new char[m_t2h_bufsize]; init(server); }
  ~mmlink_connection() { close_connection(); delete[] m_h2t_buf; delete[] m_t2h_buf; }
  bool is_open() { return m_fd >= 0; }
  bool is_data() { return m_is_data; }
  bool is_bound() { return m_is_bound; }
  void set_is_data(void) { m_is_data = true; }

  size_t send(const char *msg, const size_t len);
  size_t send_all(const char *msg, const size_t len);
  void close_connection() { if (is_open()) ::close(m_fd); init(); }
  void bind() { m_is_bound = true; }
  void set_socket(int socket) { m_fd = socket; }
  int get_socket() { return m_fd; }
  int handle_receive();
  int handle_management(void);

  char *get_h2t_buf(void) { return m_h2t_buf; }
  void set_h2t_buf_end(size_t index) { m_h2t_buf_end = index; }
  size_t get_h2t_buf_end(void) { return m_h2t_buf_end; }

  char *get_t2h_buf(void) { return m_t2h_buf; }
  void set_t2h_buf_end(size_t index) { m_t2h_buf_end = index; }
  size_t get_t2h_buf_end(void) { return m_t2h_buf_end; }
  size_t get_t2h_buf_bytes_remaining(void) { return m_t2h_bufsize - m_t2h_buf_end; }

  static const char *UNKNOWN;
  static const char *OK;

protected:
  int m_fd;
  bool m_is_bound;
  bool m_is_data;
  mmlink_server *m_server;

  const int m_h2t_bufsize;
  char *m_h2t_buf;
  size_t m_h2t_buf_end;

  const int m_t2h_bufsize;
  char *m_t2h_buf;
  size_t m_t2h_buf_end;

  void init(mmlink_server *server) { m_server = server; init(); }

private:
  int handle_data(void);
  int handle_management_command(char *cmd);
  int handle_unbound_command(char *cmd);
  int handle_bound_command(char *cmd);
  int get_server_id(void) { return m_server->get_server_id(); }
  mm_debug_link_interface *driver(void) { return m_server->get_driver_fd(); }
  void init(void) { m_fd = -1; m_is_bound = false; m_is_data = false; m_h2t_buf_end = 0; m_t2h_buf_end = 0; }
};

#endif
