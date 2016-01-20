//
// Created by jan on 13.01.16.
//

#ifndef VLC_RINGBUFFER_HPP
#define VLC_RINGBUFFER_HPP

#include <stdint.h>

template<typename T, uint16_t SIZE>
class RingBuffer {
public:
    RingBuffer() : mRead(0), mWrite(0) {}

    const T at(uint16_t index) const {
        if(!validIndex(index)) {
            return T();
        }
        return mBuffer[(mRead + index) % (SIZE + 1)];
    }

    T& at(uint16_t index) {
        return mBuffer[(mRead + index) % (SIZE + 1)];
    }

    bool empty() const {
        return mRead == mWrite;
    }

    uint16_t size() const {
        return (SIZE + 1 + mWrite - mRead) % (SIZE + 1);
    }

    uint16_t capacity() const {
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
    bool validIndex(uint16_t index) const {
        return index < size();
    }

    T mBuffer[SIZE + 1];
    uint16_t mRead;
    uint16_t mWrite;
};


#endif //VLC_RINGBUFFER_HPP
