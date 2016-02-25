//
// Created by jan on 13.01.16.
//

#ifndef VLC_RINGBUFFER_HPP
#define VLC_RINGBUFFER_HPP

#include <stdint.h>

template<typename T, uint8_t SIZE>
class RingBuffer {
public:
    RingBuffer() : mRead(0), mWrite(0) {}

    const T at(uint8_t index) const {
        return mBuffer[(mRead + index) % (SIZE + 1)];
    }

    T& at(uint8_t index) {
        return mBuffer[(mRead + index) % (SIZE + 1)];
    }

    bool empty() const {
        return mRead == mWrite;
    }

    uint8_t size() const {
        return ((uint8_t)(((uint16_t)mWrite) + SIZE + 1 - mRead)) % (SIZE + 1);
    }

    uint8_t capacity() const {
        return SIZE;
    }

    RingBuffer& operator>>(T& out) {
        if(mRead == mWrite) {
            return *this;
        }
        out = mBuffer[mRead];
        mRead = (mRead + 1) % (SIZE + 1);
        return *this;
    }

    RingBuffer& operator<<(const T& in) {
        mBuffer[mWrite] = in;
        mWrite = (mWrite + 1) % (SIZE + 1);
        if(mRead == mWrite) {
            mRead = (mRead + 1) % (SIZE + 1);
        }
        return *this;
    }

    const T pop() {
        if(mRead == mWrite) {
            return T();
        }
        const T& out = mBuffer[mRead];
        mRead = (mRead + 1) % (SIZE + 1);
        return out;
    }

private:
    T mBuffer[SIZE + 1];
    uint8_t mRead;
    uint8_t mWrite;
};

#endif //VLC_RINGBUFFER_HPP
