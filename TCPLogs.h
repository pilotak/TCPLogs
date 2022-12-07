/*
MIT License

Copyright (c) 2018 Pavel Slama

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef TCPLOGS_H
#define TCPLOGS_H

#include <climits>
#include "mbed.h"

#if !defined(MBED_CONF_LOGS_DEBUG)
    #define tr_error(...) {}
    #define tr_warning(...) {}
    #define tr_info(...) {}
    #define tr_debug(...) {}
#endif

class TCPLogs {
  public:
    TCPLogs();
    void network(NetworkInterface *network);
    void set_server(const char *server, uint16_t port);
    nsapi_error_t connect();
    bool is_connected();
    nsapi_error_t send(const char *data, nsapi_size_t size);
    void disconnect(bool full_disconnect = true);
    void attach(Callback<void()> data_cb);
    nsapi_size_or_error_t read(void *buffer, uint16_t size);

  private:
    NetworkInterface *_network;
    TCPSocket _socket;
    Callback<void()> _data_cb;

    bool _is_connected = false;
    bool _open = false;
    uint16_t _port = 80;
    char _server[41] = {0};
};

#endif  // TCPLOGS_H
