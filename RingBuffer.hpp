#ifndef VLC_RINGBUFFER_HPP
#define VLC_RINGBUFFER_HPP

#include <stdint.h>

/**
 * Template class for a ring buffer.
 *
 * SIZE must be (power of two - 1) due to the lack of a division unit on AVRs
 * for performance reasons.
 */
template<typename T, uint8_t SIZE>
class RingBuffer {
public:
    /** Constructor, initially empty */
    RingBuffer() : mRead(0), mWrite(0) {}

    /**
     * Returns a copy of the value stored at the given current index.
     *
     * @param index offset from current start of the element to return
     * @returns copy of the element at given index
     */
    const T at(uint8_t index) const {
        return mBuffer[(mRead + index) & SIZE];
    }

    /**
     * Returns a mutable reference to the value stored at the given current
     * index.
     *
     * @param index offset from current start of the element to return
     * @returns mutable reference of the element at given index
     */
    T& at(uint8_t index) {
        return mBuffer[(mRead + index) & SIZE];
    }

    /**
     * Indicates if this buffer is empty.
     *
     * @returns true if empty, false otherwise
     */
    bool empty() const {
        return mRead == mWrite;
    }

    /**
     * Returns the number of currently stored elements in this buffer.
     *
     * @returns fill size of this buffer
     */
    uint8_t size() const {
        return ((uint8_t)(((uint16_t)mWrite) + SIZE + 1 - mRead)) & SIZE;
    }

    /**
     * Returns the capacity (maximum of stored elements) of this buffer.
     *
     * @returns maximum size of this buffer
     */
    uint8_t capacity() const {
        return SIZE;
    }

    /**
     * Puts the currently first element in the buffer into the address provided
     * and removes it from the buffer.
     *
     * @param out reference/address to store the element into
     * @returns this, for chaining
     * @see pop()
     */
    RingBuffer& operator>>(T& out) {
        if(mRead == mWrite) {
            return *this;
        }
        out = mBuffer[mRead];
        mRead = (mRead + 1) & SIZE;
        return *this;
    }

    /**
     * Inserts a copy of the provided element at the end of this buffer.
     *
     * @param in element to append to the buffer
     * @returns this, for chaining
     */
    RingBuffer& operator<<(const T& in) {
        mBuffer[mWrite] = in;
        mWrite = (mWrite + 1) & SIZE;
        if(mRead == mWrite) {
            mRead = (mRead + 1) & SIZE;
        }
        return *this;
    }

    /**
     * Returns a copy of the currently first element in the buffer and removes
     * it from the buffer.
     *
     * @returns copy of the first element
     * @see operator>>(T&)
     */
    const T pop() {
        if(mRead == mWrite) {
            return T();
        }
        const T& out = mBuffer[mRead];
        mRead = (mRead + 1) & SIZE;
        return out;
    }

private:
    /** Storage array */
    T mBuffer[SIZE + 1];
    /** Current read index */
    volatile uint8_t mRead;
    /** Current write index */
    volatile uint8_t mWrite;
};

#endif //VLC_RINGBUFFER_HPP
