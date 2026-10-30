#include <cassert>
#include <cstdio>
#include <vector>

#include "PacketSerial.h"

/// A byte stream that loops transmitted bytes back to the receiver.
class LoopbackStream: public Stream
{
public:
    std::vector<uint8_t> bytes;

    int available() override
    {
        return static_cast<int>(bytes.size() - readIndex);
    }

    int read() override
    {
        return available() ? bytes[readIndex++] : -1;
    }

    size_t write(uint8_t value) override
    {
        bytes.push_back(value);
        return 1;
    }

    size_t write(const uint8_t* buffer, size_t size) override
    {
        bytes.insert(bytes.end(), buffer, buffer + size);
        return size;
    }

private:
    size_t readIndex = 0;
};

static size_t callbacks;
static std::vector<uint8_t> received;

/// Capture the actual callback produced by PacketSerial::update().
static void onPacket(const uint8_t* buffer, size_t size)
{
    ++callbacks;
    received.assign(buffer, buffer + size);
}

/// Check both wire bytes and decoded delivery, including the empty COBS frame.
static void checkCOBS(const uint8_t* data, size_t size,
                      const std::vector<uint8_t>& expectedWire)
{
    LoopbackStream stream;
    COBSPacketSerial packet;
    packet.setStream(&stream);
    packet.setPacketHandler(onPacket);
    callbacks = 0;
    received.clear();

    packet.send(data, size);
    assert(stream.bytes == expectedWire);
    packet.update();
    assert(callbacks == 1);
    assert(received == std::vector<uint8_t>(data, data + size));
}

/// Verify SLIP transmission without changing its existing receive semantics.
static void checkSLIP(const uint8_t* data, size_t size,
                      const std::vector<uint8_t>& expectedWire)
{
    LoopbackStream stream;
    SLIPPacketSerial packet;
    packet.setStream(&stream);
    packet.send(data, size);
    assert(stream.bytes == expectedWire);
}

/// Invalid pointers and a missing stream must still be ignored.
static void checkIgnoredInputs()
{
    LoopbackStream stream;
    COBSPacketSerial packet;
    const uint8_t byte = 1;
    packet.setStream(&stream);
    packet.send(nullptr, 0);
    packet.send(nullptr, 1);
    assert(stream.bytes.empty());
    packet.setStream(nullptr);
    packet.send(&byte, 0);
    packet.send(&byte, 1);
    assert(stream.bytes.empty());
}

int main()
{
    const uint8_t dummy = 0xA5;
    const uint8_t zero = 0;
    const uint8_t data[] = { 0x11, 0, 0x22, 0xFF };
    const uint8_t slipData[] = { SLIP::END, SLIP::ESC };

    checkCOBS(&dummy, 0, { 1, 0 });
    checkCOBS(&zero, 1, { 1, 1, 0 });
    checkCOBS(data, sizeof(data), { 2, 0x11, 3, 0x22, 0xFF, 0 });
    checkSLIP(&dummy, 0, { SLIP::END });
    checkSLIP(slipData, sizeof(slipData),
              { SLIP::END, SLIP::ESC, SLIP::ESC_END,
                SLIP::ESC, SLIP::ESC_ESC, SLIP::END });
    checkIgnoredInputs();
    std::puts("PASS: empty and nonempty COBS delivery, SLIP framing and ignored inputs");
}
