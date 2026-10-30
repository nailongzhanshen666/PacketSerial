# Empty-packet regression

Run from the repository root with a GNU C++11 compiler:

```sh
build=$(mktemp -d)
g++ -std=gnu++11 -Wall -Wextra -Werror -Itests/support -Isrc \
  tests/test_empty_packets.cpp -o "$build/test_empty"
"$build/test_empty"
```

The test includes the real `PacketSerial.h`, COBS and SLIP implementations.
`support/Arduino.h` only declares the small Arduino interface needed to compile
those headers; a loopback `Stream` captures and replays the actual wire bytes.
No Arduino SDK or external test framework is required.

Coverage:

- Empty COBS payload encodes as `01 00` (including the marker) and invokes one
  receive callback with size zero.
- A payload containing one zero byte remains distinct (`01 01 00`), and a mixed
  nonempty payload still decodes unchanged.
- SLIP transmits the empty packet marker; nonempty escaping remains unchanged.
  SLIP receive handling is not changed or exercised by this test.
- Null buffer pointers and calls without an attached stream remain ignored.

The empty payload uses a non-null buffer, as required by `send()`. The test does
not verify physical UART timing, Arduino board compilation or custom encoders.
GNU C++11 is used because the existing library uses variable-length arrays.
