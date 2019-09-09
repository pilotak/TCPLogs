#include "mbed.h"
#include "TCPLogs.h"

TCPLogs::TCPLogs(NetworkInterface *network):
    _network(network),
    _is_connected(false),
    _port(80) {
    memset(_server, 0, sizeof(_server));
}

void TCPLogs::setServer(const char * server, uint16_t port) {
    memcpy(_server, server, strlen(server));
    _port = port;
}

nsapi_error_t TCPLogs::connect() {
    nsapi_error_t ret = NSAPI_ERROR_DEVICE_ERROR;

    if (_network) {
        ret = _socket.open(_network);

        if (ret == NSAPI_ERROR_OK) {
            _socket.set_blocking(false);
            ret = _socket.connect(_server, _port);

            if (ret == NSAPI_ERROR_OK || ret == NSAPI_ERROR_IS_CONNECTED) {
                _is_connected = true;
                return ret;
            }
        }
    }

    _socket.close();
    _is_connected = false;
    return ret;
}

void TCPLogs::log(const char * str) {
    nsapi_size_or_error_t response = NSAPI_ERROR_OK;
    uint32_t size = strlen(str);

    if (_is_connected && size > 0) {
        while (size) {
            response = _socket.send(str + response, size);

            if (response < NSAPI_ERROR_OK) {
                break;
            }

            size -= response;
        }

        char buf[2];
        response = _socket.recv(buf, 2);

        if (response == NSAPI_ERROR_NO_SOCKET) {
            _is_connected = false;
        }
    }
}

bool TCPLogs::isConnected() {
    return _is_connected;
}

void TCPLogs::disconnect(bool network) {
    _is_connected = false;
    _socket.close();

    if (_network && network) {
        _network->disconnect();
    }
}
