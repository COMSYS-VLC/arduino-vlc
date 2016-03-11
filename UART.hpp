#ifndef VLC_UART_HPP
#define VLC_UART_HPP

#include <stdint.h>
#include "RingBuffer.hpp"

/** Globally accessible class for UART reading and writing */
class UART {
public:
    /** Constructor */
    UART();

    /**
     * Sends the given byte via UART.
     *
     * @param byte byte to send
     */
    void send(uint8_t byte);

    /**
     * Returns the next received byte, blocking if necessary.
     *
     * @returns next received bit
     */
    uint8_t receive();

    /**
     * Indicates if there is data available to receive.
     *
     * @returns true if there is data available, false otherwise
     * @see receive()
     */
    bool hasData() const;

    /**
     * Writes the given value converted to ASCII via UART.
     *
     * @param value value to convert and write
     * @returns this, for chaining
     */
    UART& operator<<(uint8_t value);

    /**
     * Writes the given value converted to ASCII via UART.
     *
     * @param value value to convert and write
     * @returns this, for chaining
     */
    UART& operator<<(uint16_t value);

    /**
     * Writes the given value converted to ASCII via UART.
     *
     * @param value value to convert and write
     * @returns this, for chaining
     */
    UART& operator<<(uint32_t value);

    /**
     * Writes the given value converted to ASCII via UART.
     *
     * @param value value to convert and write
     * @returns this, for chaining
     */
    UART& operator<<(int8_t value) { return value < 0 ? send('-'), *this << (uint8_t)(-value) : *this << (uint8_t)value; }

    /**
     * Writes the given value converted to ASCII via UART.
     *
     * @param value value to convert and write
     * @returns this, for chaining
     */
    UART& operator<<(int16_t value) { return value < 0 ? send('-'), *this << (uint16_t)(-value) : *this << (uint16_t)value; }

    /**
     * Writes the given value converted to ASCII via UART.
     *
     * @param value value to convert and write
     * @returns this, for chaining
     */
    UART& operator<<(int32_t value) { return value < 0 ? send('-'), *this << (uint32_t)(-value) : *this << (uint32_t)value; }

    /**
     * Writes the given character via UART.
     *
     * @param c character to write
     * @returns this, for chaining
     */
    UART& operator<<(char c);

    /**
     * Writes the given string via UART.
     *
     * @param str string to write
     * @returns this, for chaining
     */
    UART& operator<<(const char* str);

    /**
     * Reads the next received byte into the given reference, blocking if
     * necessary.
     *
     * @param value reference to read into
     * @returns this, for chaining
     * @see receive()
     */
    UART& operator>>(uint8_t &value);

    /**
     * Gives access to the global instance of this class.
     *
     * @returns global UART instance
     */
    static UART& get() { return mInstance; }

private:
    /** global UART instance */
    static UART mInstance;
};

#endif //VLC_UART_HPP
