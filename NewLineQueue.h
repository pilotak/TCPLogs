#ifndef NEWLINEQUEUE_H
#define NEWLINEQUEUE_H

#include "mbed.h"

template <uint16_t N>
class NewLineQueue {
  public:
    NewLineQueue();
    virtual ~NewLineQueue(void);

    void push(const char *str, ...);
    bool fits(uint16_t len);
    void reset();
    bool empty();
    void pop();
    void lock();
    void unlock();
    void prefix(char *(*pref_f)(size_t));

    char buffer[N];
    uint16_t length = 0;

  private:
    Mutex _mutex;
    char *(*_prefix)(size_t) = 0;
};

template <uint16_t N>
NewLineQueue<N>::NewLineQueue() {
    reset();
}

template <uint16_t N>
NewLineQueue<N>::~NewLineQueue(void) {
}

template <uint16_t N>
void NewLineQueue<N>::prefix(char *(*pref_f)(size_t)) {
    _prefix = pref_f;
}

template <uint16_t N>
void NewLineQueue<N>::push(const char *str, ...) {
    va_list ap;
    va_start(ap, str);

    ssize_t msg_len = 1; // "\n"

    msg_len += vsnprintf(nullptr, 0, str, ap);

    if (_prefix) {
        msg_len += snprintf(nullptr, 0, "%s", _prefix(msg_len));
        msg_len += 2; // ": "
    }

    if (msg_len > N) {
        va_end(ap);
        return;
    }

    while (1) {
        if (msg_len <= (N - length)) {
            lock();

            if (_prefix) {
                length += snprintf(buffer + length, N - length, _prefix(0));
                buffer[length++] = ':';
                buffer[length++] = ' ';
            }

            length += vsnprintf(buffer + length, N - length, str, ap);

            buffer[length] = '\n';
            length++;

            unlock();

            break;

        } else {
            while (msg_len > (N - length)) {
                pop();  // remove oldest message
            }
        }
    }

    va_end(ap);
}

template <uint16_t N>
void NewLineQueue<N>::pop() {
    if (!empty()) {
        uint16_t index = 0;

        for (uint16_t i = 0; i < N; i++) {
            if (buffer[i] == '\n') {
                index = i;
                break;
            }
        }

        uint16_t msg_len = index + 1;
        length -= msg_len;
        memmove(buffer, buffer + msg_len, length);
    }
}

template <uint16_t N>
void NewLineQueue<N>::reset() {
    length = 0;
    memset(buffer, 0, N);
}

template <uint16_t N>
bool NewLineQueue<N>::empty() {
    return length == 0;
}

template <uint16_t N>
bool NewLineQueue<N>::fits(uint16_t len) {
    if (length + len > N) {
        return false;
    }

    return true;
}

template <uint16_t N>
void NewLineQueue<N>::lock() {
    _mutex.lock();
}

template <uint16_t N>
void NewLineQueue<N>::unlock() {
    _mutex.unlock();
}


#endif  // NEWLINEQUEUE_H
