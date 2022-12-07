#include "mbed.h"
#include "TCPLogs.h"

#include "mbed-trace/mbed_trace.h"

#ifndef TRACE_GROUP
    #define TRACE_GROUP  "LOGS"
#endif


TCPLogs::TCPLogs():
    _network(nullptr) {
}

void TCPLogs::network(NetworkInterface *network) {
    _network = network;
}


void TCPLogs::set_server(const char *server, uint16_t port) {
    memcpy(_server, server, strlen(server));
    _port = port;
}

void TCPLogs::attach(Callback<void()> data_cb) {
    _data_cb = data_cb;
}

nsapi_error_t TCPLogs::connect() {
    nsapi_error_t ret = NSAPI_ERROR_PARAMETER;
    SocketAddress addr;

    if (_network == nullptr) {
        tr_error("Invalid network");
        goto END;
    }

    if (_open) {
        _socket.close();
        _open = false;
    }

    tr_debug("Connecting...");

    _socket.sigio(nullptr);
    ret = _socket.open(_network);

    if (ret != NSAPI_ERROR_OK) {
        tr_error("Socket open error: %i", ret);
        goto END;
    }

    _open = true;

    ret = _network->gethostbyname(_server, &addr);

    if (ret != NSAPI_ERROR_OK) {
        tr_error("DNS error: %i", ret);
        ret = NSAPI_ERROR_DNS_FAILURE;
        goto END;
    }

    addr.set_port(_port);
    _socket.set_blocking(true);

    tr_debug("Server IP address: %s", addr.get_ip_address());

    ret = _socket.connect(addr);

    if (ret != NSAPI_ERROR_OK) {
        tr_error("Connect error: %i", ret);
        goto END;
    }

    _socket.sigio(_data_cb);
    _socket.set_blocking(false);

    tr_info("Connect OK");
    _is_connected = true;

    return ret;

END:
    _socket.sigio(nullptr);
    _is_connected = false;

    if (_open) {
        _socket.close();
        _open = false;
    }

    return ret;
}

nsapi_error_t TCPLogs::send(const char *data, nsapi_size_t size) {
    nsapi_size_or_error_t ret = NSAPI_ERROR_OK;

    if (!_is_connected) {
        return ret;
    }

    if (data == nullptr || size == 0) {
        return NSAPI_ERROR_PARAMETER;
    }

    tr_debug("Sending[%u]: %s", size, tr_array((const uint8_t *)data, size));

    while (size) {
        ret = _socket.send(data + ret, size);

        if (ret < NSAPI_ERROR_OK) {
            break;
        }

        size -= ret;
    }

    if (ret < NSAPI_ERROR_OK && ret != NSAPI_ERROR_WOULD_BLOCK) {
        tr_error("Sending data failed: %d", ret);

        disconnect(ret != NSAPI_ERROR_DEVICE_ERROR);

        return ret;
    }

    return NSAPI_ERROR_OK;
}

bool TCPLogs::is_connected() {
    return _is_connected;
}

void TCPLogs::disconnect(bool full_disconnect) {
    if (full_disconnect) {
        _socket.set_blocking(true);
        _socket.sigio(nullptr);

        if (_open) {
            tr_info("Closing");
            _socket.close();
        }
    }

    _open = false;
    _is_connected = false;
}

nsapi_size_or_error_t TCPLogs::read(void *buffer, uint16_t size) {
    nsapi_size_or_error_t ret = NSAPI_ERROR_OK;

    if (!_is_connected) {
        return ret;
    }

    ret = _socket.recv(buffer, size);

    if (ret == NSAPI_ERROR_OK || ret < NSAPI_ERROR_WOULD_BLOCK) {
        if (ret != NSAPI_ERROR_OK) {
            tr_error("Receive error: %i", ret);

        } else {
            tr_info("Server closed");
        }

        disconnect(ret != NSAPI_ERROR_DEVICE_ERROR);

    } else if (ret != NSAPI_ERROR_WOULD_BLOCK) {
        tr_info("Received: %i bytes", ret);
        tr_debug("%s", tr_array((uint8_t *)buffer, ret));

    } else {
        tr_debug("No more to read");
    }


    return ret;
}