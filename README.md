# TCPLogs

```cpp
#include "mbed.h"
#include "UbloxATCellularInterfaceExt.h"
#include "TCPLogs.h"

UbloxATCellularInterfaceExt mdm(PD_5, PD_6, 115200, false);
TCPLogs logs;

const char* server = "text.com";
const int port = 12345;
uint8_t data[8] = {0};

bool serverConnect() {
    if (logs.connect() == NSAPI_ERROR_OK) {
        return true;

    } else {
        printf("Socket connect FAILED\n");
    }

    return false;
}

int main() {
    printf("Start\n");

    if (mdm.init()) {
        printf("mdm init OK\n");
        logs.network(mdm);

        if (mdm.connect() == NSAPI_ERROR_OK) {
            printf("mdm connect OK\n");
            printf("Connecting to %s: %d\r\n", server, port);
            logs.set_server(server, port);

            if (serverConnect()) {
                while (1) {
                    if (logs.is_connected()) {
                        printf("sending\n");
                        int size = snprintf(data, sizeof(data), "test");
                        logs.log(data, size);

                    } else {
                        serverConnect();
                    }

                    ThisThread::sleep_for(5s);
                }
            }

        } else {
            printf("mdm connect FAILED\n");
        }

    } else {
        printf("mdm init FAILED\n");
    }
}

```
