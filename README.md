# TCPLogs

```cpp
#include "mbed.h"
#include "UbloxATCellularInterfaceExt.h"
#include "TCPLogs.h"

UbloxATCellularInterfaceExt mdm(PD_5, PD_6, 115200, false);
TCPLogs logs(&mdm);

const char* server = "text.com";
const int port = 12345;

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

        if (mdm.connect() == NSAPI_ERROR_OK) {
            printf("mdm connect OK\n");
            printf("Connecting to %s: %d\r\n", server, port);
            logs.setServer(server, port);

            if (serverConnect()) {
                while (1) {
                    if (logs.isConnected()) {
                        printf("sending\n");
                        logs.log("test");

                    } else {
                        serverConnect();
                    }

                    ThisThread::sleep_for(5000);
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