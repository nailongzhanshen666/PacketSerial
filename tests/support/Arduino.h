#pragma once

#include <stddef.h>
#include <stdint.h>

/// Minimal Arduino interface for the host-side PacketSerial regression.
class Stream
{
public:
    virtual ~Stream() {}
    virtual int available() = 0;
    virtual int read() = 0;
    virtual size_t write(uint8_t value) = 0;
    virtual size_t write(const uint8_t* buffer, size_t size) = 0;
};

/// Declarations needed to parse PacketSerial's unused begin() methods.
class HardwareSerial: public Stream
{
public:
    void begin(unsigned long speed);
    explicit operator bool() const;
    int available() override;
    int read() override;
    size_t write(uint8_t value) override;
    size_t write(const uint8_t* buffer, size_t size) override;
};

extern HardwareSerial Serial;
