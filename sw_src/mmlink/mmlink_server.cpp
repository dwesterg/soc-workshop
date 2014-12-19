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

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "dprint.h"
#include "mm_debug_link_interface.h"
#include "mmlink_connection.h"
#include "mmlink_server.h"

mmlink_server::mmlink_server(struct sockaddr_in *addr, mm_debug_link_interface *driver):
  m_listen(-1), m_server_id(0), m_num_bound_connections(0), m_num_connections(0),
  m_t2h_pending(false), m_h2t_pending(false), m_driver_write_blocked(false), m_host_write_blocked(false)
{
  m_addr = *addr;

  m_conn = new mmlink_connection*[MAX_CONNECTIONS];
  for (int i = 0; i < MAX_CONNECTIONS; ++i)
    m_conn[i] = new mmlink_connection(this);

  m_driver = driver;

#ifdef ENABLE_MMLINK_STATS
  m_h2t_stats = new mmlink_stats("h2t");
  m_t2h_stats = new mmlink_stats("t2h");
  m_mmlink_select_time = new mmlink_select_time();
#endif

}

mmlink_server::~mmlink_server()
{
  if (m_conn)
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
      delete m_conn[i];
    }
  delete[] m_conn;
  m_driver->close();

#ifdef ENABLE_MMLINK_STATS
  delete m_h2t_stats;
  delete m_t2h_stats;
  delete m_mmlink_select_time;
#endif
}

int mmlink_server::setup_listen_socket(void)
{
  m_listen = socket(AF_INET, SOCK_STREAM, 0);

  if (m_listen < 0)
  {
    DPRINT("Socket creation failed: %d\n", errno);
    return errno;
  }
  DPRINT("m_listen: %d\n", m_listen);

  // Allow reconnect sooner, after server exit.
  int optval = 1;
  int err =
    setsockopt(m_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  if (err < 0)
  {
    DPRINT("setsockopt failed: %d\n", errno);
    return errno;
  }

  return 0;
}

int mmlink_server::run(void)
{
  int err = 0;
  m_running = true;

  if (err = m_driver->open())
  {
    fprintf(stderr, "failed to init driver: %d (%s)\n", errno, strerror(errno));
    return err;
  }

  // Todo: modulate timeout based on number of connections, expectation of data.
  struct timeval tv;
  tv.tv_sec  = 0;
  tv.tv_usec = 1000;

  if (setup_listen_socket())
  {
    fprintf(stderr, "setup_listen_socket() failed\n");
    return -1;
  }

  if (bind(m_listen, (struct sockaddr *)&m_addr, sizeof(m_addr)) != 0)
  {
    fprintf(stderr, "bind() failed: %d (%s)\n", errno, strerror(errno));
    return errno;
  }

  if (listen(m_listen, 5) < 0)
  {
    fprintf(stderr, "listen() failed: %d (%s)\n", errno, strerror(errno));
    return errno;
  }

  DPRINT("listening on ip: %s; port: %d\n", inet_ntoa(m_addr.sin_addr),
    htons(m_addr.sin_port));

  while (m_running)
  {
    fd_set readfds, writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    int max_fd = -1;
    // Listen for more connections, if needed.
    if (m_num_connections < MAX_CONNECTIONS)
    {
      FD_SET(m_listen, &readfds);
      max_fd = MAX(m_listen, max_fd);
    }

    // Listen for read on all connections.
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
      mmlink_connection *pc = *(m_conn + i);
      if (pc->is_open())
      {
        int fd = pc->get_socket();
        FD_SET(fd, &readfds);

        max_fd = MAX(fd, max_fd);
      }
    }

    // If we have a data socket, listen for read and write on the driver fd.
    mmlink_connection *data_conn = get_data_connection();
    if (data_conn)
    {
      int driver_fd = m_driver->get_fd();
      int host_fd = data_conn->get_socket();

      // Listen for read on the driver fd.
      //   responses from the driver fd are written to the host via the data socket.
      FD_SET(driver_fd, &readfds);
      // Q: what's the behavior of the driver when no data is available for read?
      //  - if it lies and says data always available, this is wasteful
      //  - if it never says data is available, deadlock or incorrect blocking behavior
      //    - could wait until select timeout, also performance reduction - doesn't deadlock, but has to timeout first.

      // Listen for write on the driver fd.
      //   Host commands from the data socket are written to this fd.
      //   Only select the driver FD for write if
      //     1) there is h2t data to send;
      //     2) the driver previously blocked on write.
      if (m_h2t_pending && m_driver_write_blocked)
      {
        FD_SET(driver_fd, &writefds);
        max_fd = MAX(driver_fd, max_fd);
      }

      // Listen for write on the host
      //   Data from the driver are written here.
      // Only select the host FD for write if
      //   1) there is t2h data to send;
      //   2) the host FD previously blocked on write.
      if (m_t2h_pending && m_host_write_blocked)
      {
        FD_SET(host_fd, &writefds);
        max_fd = MAX(host_fd, max_fd);
      }
    }

    tv.tv_sec  = 0;
    tv.tv_usec = 1000;
#ifdef ENABLE_MMLINK_STATS
    struct timespec ts_before, ts_after;
    clock_gettime(CLOCK_REALTIME, &ts_before);
#endif
    if (select(max_fd + 1, &readfds, &writefds, NULL, &tv) < 0)
    {
      fprintf(stderr, "select error: %d (%s)\n", errno, strerror(errno));
      break;
    }
#ifdef ENABLE_MMLINK_STATS
    clock_gettime(CLOCK_REALTIME, &ts_after);
    m_mmlink_select_time->update(&ts_before, &ts_after);
#endif

    // Q: does select ever block? (get_nanotime [or something] for precise time data)

    // Handle new connection attempts.
    if (FD_ISSET(m_listen, &readfds))
    {
      mmlink_connection *pc = handle_accept();
      // If a new connection was accepted, send the welcome string.
      if (pc)
      {
        char msg[256];

        get_welcome_message(msg, sizeof(msg) / sizeof(*msg));
        DPRINT("sending welcome: '%s'\n", msg);
        pc->send_all(msg, strlen(msg));
      }
    }

    if (data_conn)
    {
      // Transfer response data from the driver to the data socket.
      bool can_write_host =
        FD_ISSET(data_conn->get_socket(), &writefds) || !m_host_write_blocked;
      bool can_read_driver = FD_ISSET(m_driver->get_fd(), &readfds);
      err = handle_t2h(data_conn, can_read_driver, can_write_host);

      if (err)
        break;

      // Transfer command data from the data socket to the driver.
      bool can_write_driver =
        FD_ISSET(m_driver->get_fd(), &writefds) || !m_driver_write_blocked;
      bool can_read_host = FD_ISSET(data_conn->get_socket(), &readfds);
      err = handle_h2t(data_conn, can_read_host, can_write_driver);

      if (err < 0)
      {
        m_num_connections--;
        data_conn->close_connection();
        DPRINT("closed data connection due to handle_h2t return value, now have %d\n", m_num_connections);
      }
    }

    // Handle management connection commands and responses.
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
      mmlink_connection *pc = *(m_conn + i);
      if (!pc->is_open())
        continue;
      if (pc->is_data())
        continue;

      if (FD_ISSET(pc->get_socket(), &readfds))
      {
        int fail = pc->handle_receive();
        if (fail)
        {
          --m_num_connections;
          DPRINT("%d: handle_receive() returned %d, closing connection, now have %d\n",
            pc->get_socket(), fail, m_num_connections);
          pc->close_connection();
        }
        else
        {
          fail = pc->handle_management();
          if (fail)
          {
            --m_num_connections;
            DPRINT("%d: handle_management() returned %d, closing connection, now have %d\n",
              pc->get_socket(), fail, m_num_connections);
            pc->close_connection();
          }
          else if (pc->is_data())
          {
            DPRINT("%d: converted to data\n", pc->get_socket());
            // A management connection was converted to data. There can be only one.
            close_other_data_connection(pc);
            m_h2t_pending = true;
          }
        }
      }
    }
  }
  DPRINT("goodbye with code %d\n", err);

  return err;
}

void mmlink_server::print_stats(void)
{
#ifdef ENABLE_MMLINK_STATS
  DPRINT("mmlink_connection::print_stats()\n");

  m_h2t_stats->print();
  m_t2h_stats->print();
  m_mmlink_select_time->print();
#endif
}

mmlink_connection *mmlink_server::handle_accept()
{
  int socket;
  struct sockaddr_in incoming_addr;
  socklen_t len = sizeof(incoming_addr);

  // Find an mmlink_connection for this new connection,
  // or NULL if none available.
  mmlink_connection *pc = get_unused_connection();

  socket = ::accept(m_listen, (struct sockaddr *)&incoming_addr, &len);
  if (socket < 0)
  {
    fprintf(stderr, "accept failed: %d (%s)\n", errno, strerror(errno));	
    pc = NULL;
  }
  else
  {
    // Set socket to non-blocking.
    int flags = ::fcntl(socket, F_GETFL, 0);
    ::fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    if (pc)
    {
      ++m_num_connections;
      pc->set_socket(socket);
      DPRINT("I have %d connections now; latest socket is %d\n", m_num_connections, socket);
      // The 1st connection is bound upon connection.  The 2nd connection will
      // be bound if it sends the correct handle.
      if (m_num_connections == 1)
      {
        DPRINT("%d: binding first connection\n", pc->get_socket());
        pc->bind();
      }
      DPRINT("%d: Accepted connection request from %s\n", pc->get_socket(), inet_ntoa(incoming_addr.sin_addr));
    }
    else
    {
      // If there are no unused connections available, we shouldn't be in
      // this routine in the first place. If this happens anyway, accept
      // and close the connection.
      fprintf(stderr, "%d: Rejected connection request from %s\n", socket, inet_ntoa(incoming_addr.sin_addr));
      ::close(socket);
      pc = NULL;
    }
  }

  return pc;
}

void mmlink_server::get_welcome_message(char *msg, size_t msg_len)
{
  int ident[4];

  m_driver->ident(ident);

  if (m_num_connections == 1)
  {
    ++m_server_id;
    snprintf(msg, msg_len, "SystemConsole CONFIGROM IDENT=%08X%08X%08X%08X HANDLE=%08X\r\n",
      ident[3], ident[2], ident[1], ident[0], m_server_id);
  }
  else
  {
    snprintf(msg, msg_len, "SystemConsole CONFIGROM IDENT=%08X%08X%08X%08X HANDLE\r\n",
      ident[3], ident[2], ident[1], ident[0]);
  }
}

mmlink_connection *mmlink_server::get_unused_connection()
{
  mmlink_connection *pc = NULL;
  for (int i = 0; i < MAX_CONNECTIONS; ++i)
    if (!m_conn[i]->is_open())
    {
      pc = *(m_conn + i);
      break;
    }

  return pc;
}

void mmlink_server::close_other_data_connection(mmlink_connection *pc)
{
  for (int i = 0; i < MAX_CONNECTIONS; ++i)
  {
    mmlink_connection *other_pc = *(m_conn + i);
    if (other_pc == pc)
      continue;
    if (other_pc->is_open() && other_pc->is_data())
    {
      DPRINT("closing old data connection in favor of new one\n");
      m_num_connections--;
      other_pc->close_connection();
    }
  }
}

// Return the data connection, or NULL if none.
// Could cache this.
mmlink_connection *mmlink_server::get_data_connection(void)
{
  for (int i = 0; i < m_num_connections; ++i)
  {
    mmlink_connection *pc = *(m_conn + i);
    if (pc->is_data())
      return pc;
  }

  return NULL;
}

int mmlink_server::handle_t2h(mmlink_connection *data_conn, bool can_read_driver, bool can_write_host)
{
  int err = 0;
  bool socket_error = false;
  bool t2h_ready = m_t2h_pending ? can_write_host : can_read_driver;

  if (!data_conn)
  {
    // We don't have a data connection. Nothing to do here.
    fprintf(stderr, "hardware returned data but there's no data socket\n");
    return -1;
  }

  if (!t2h_ready)
  {
    return 0;
  }

  // Try to get more data.
  if (can_read_driver)
  {
    int offset = data_conn->get_t2h_buf_end();
    int rem = data_conn->get_t2h_buf_bytes_remaining();
    int len = m_driver->read(data_conn->get_t2h_buf() + offset, rem);

    // If a non-zero number of bytes were read, update the end index.
    // The driver will return -1, and set errno to EAGAIN if there was 
    // no data available for read. If negative or 0 is returned, do nothing
    // (read again later).
    if (len > 0)
      data_conn->set_t2h_buf_end(offset + len);
  }

  if (!data_conn->get_t2h_buf_end())
  {
    // Still no t2h data; done here.
    m_t2h_pending = false;
    return 0;
  }

  // Handle response data from the driver.
  if (can_write_host && data_conn)
  {
    // Send the data to the data socket.
    int total_sent = 0;

    while (total_sent < data_conn->get_t2h_buf_end())
    {
      ssize_t sent = data_conn->send(data_conn->get_t2h_buf() + total_sent, data_conn->get_t2h_buf_end() - total_sent);

      if (sent < 0)
      {
        if (errno == EAGAIN)
        {
          // Try again later.
          break;
        }
        else
        {
          // Socket error, disconnected?
          socket_error = true;
          break;
        }
      }
      else if (sent == 0)
      {
        // Didn't send all data; Try to send the remaining data later.
        // The driver will return 0 when its write FIFO is full.
        break;
      }

      total_sent += sent;
    }

    if (total_sent > 0)
      m_t2h_stats->update(total_sent, data_conn->get_t2h_buf());

    int rem = data_conn->get_t2h_buf_end() - total_sent;
    if (rem > 0)
    {
      DPRINT("t2h rem: %d; total_sent: %d; m_h2t_pending: %d\n", rem, total_sent, m_t2h_pending);
      if (total_sent > 0)
      {
        m_t2h_pending = true;
        memmove(data_conn->get_t2h_buf(), data_conn->get_t2h_buf() + total_sent, rem);
      }
      // Not all data was sent - the host has blocked.
      m_host_write_blocked = true;
    }
    else
    {
      // All the data was sent. Assume the host can accept even more.
      m_host_write_blocked = false;
    }
    data_conn->set_t2h_buf_end(rem);
  }

  if (socket_error)
  {
    // We didn't have a data connection in the first place, or an error
    // has occurred on the data connection.
    fprintf(stderr, "hardware returned data but there's no data socket\n");
    err = -1;
  }

  return err;
}

int mmlink_server::handle_h2t(mmlink_connection *data_conn, bool can_read_host, bool can_write_driver)
{
  int err = 0;

  bool h2t_ready = m_h2t_pending ? can_write_driver : can_read_host;
  if (!h2t_ready)
  {
    return 0;
  }

  // If no stored data, try to get some.
  if (can_read_host)
  {
    err = data_conn->handle_receive();
    if (err < 0)
    {
      return err;
    }
  }

  if (data_conn->get_h2t_buf_end() == 0)
  {
    // No data to send.
    m_h2t_pending = false;
    return 0;
  }

  if (!can_write_driver)
    return 0;

  // Handle command data from the data socket.
  int total_sent = 0;
  bool socket_error = false;
  while (total_sent < data_conn->get_h2t_buf_end())
  {
    ssize_t sent = m_driver->write(data_conn->get_h2t_buf() + total_sent, data_conn->get_h2t_buf_end() - total_sent);
    if (sent < 0)
    {
      if (errno == EAGAIN)
      {
        // Try again later
        DPRINT("driver write returned %d (errno: %d)\n", sent, errno);
        break;
      }
      else
      {
        // Not sure if this can happen.
        DPRINT("handle_h2t(): driver returned error %d (%s)\n", errno, strerror(errno));
      }
    }
    if (sent == 0)
    {
      // Didn't send all data; Try to send the remaining data later.
      break;
    }
    total_sent += sent;
  }

  if (total_sent > 0)
    m_h2t_stats->update(total_sent, data_conn->get_h2t_buf());

  int rem = data_conn->get_h2t_buf_end() - total_sent;
  if (rem > 0)
  {
    m_h2t_pending = true;
    if (total_sent > 0)
    {
      memmove(data_conn->get_h2t_buf(), data_conn->get_h2t_buf() + total_sent, rem);
    }
    // Not all data was sent - the driver has blocked.
    if (m_driver_write_blocked == false)
      DPRINT("h2t: not all data sent: m_driver_write_blocked is now true\n");
    m_driver_write_blocked = true;
  }
  else
  {
    // All the data was sent. Assume the driver can accept even more.
    if (m_driver_write_blocked == true)
      DPRINT("h2t: sent %d bytes: m_driver_write_blocked is now false\n", total_sent);
    m_driver_write_blocked = false;
  }
  data_conn->set_h2t_buf_end(rem);

  return err;
}

