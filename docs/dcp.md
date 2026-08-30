# Device Control Protocol (DCP)

A binary framing protocol between a host (desktop application) and an
adapter device (currently: the STM32F303VCT6 firmware in `adapter/`),
carried over USB CDC.

> **Status**: this document describes the protocol implemented by
> `adapter/modules/dcp/` (framing + codecs) and `adapter/core/inc/dcp/` +
> `adapter/core/src/dcp/` (`DcpClient`/`DcpHandler`) on branch `upb-9`. The
> desktop client
> (`desktop/dcp.h`, `desktop/dcp.cpp`) has not been updated to this wire
> format yet and still speaks the older 0x7E/CRC8 protocol — adapter and
> desktop are not currently interoperable. I²C is reserved in the command
> and peripheral numbering below but not implemented.

## Wire format

Every frame has this layout, all multi-byte fields little-endian:

| Offset | Size | Field           | Notes                                            |
|--------|------|-----------------|---------------------------------------------------|
| 0      | 1    | sync0           | `0xA5`                                             |
| 1      | 1    | sync1           | `0x5A`                                             |
| 2      | 1    | version         | `1`                                                 |
| 3      | 1    | message_type    | Request / Response / Event (see below)               |
| 4      | 1    | peripheral      | System / UART / (I2C, reserved)                        |
| 5      | 1    | resource_id     | Peripheral instance; always `0` today (one UART)         |
| 6      | 2    | command         | See Commands below                                         |
| 8      | 4    | request_id      | Echoed back on a Response; caller-assigned                  |
| 12     | 2    | payload_length  | Number of payload bytes (N), max 256                          |
| 14     | N    | payload         | Command-specific; a Response payload starts with `status` |
| 14+N   | 2    | crc16           | CRC-16/CCITT-FALSE over bytes 2..(13+N) (version..payload) |

- CRC-16/CCITT-FALSE: poly `0x1021`, init `0xFFFF`, no input/output reflect.
- `kMaxPayloadSize = 256`, `kMaxFrameSize = 272` (14-byte header incl. sync
  + 256-byte payload + 2-byte CRC).
- A **Response**'s payload always starts with a 2-byte `status` code,
  followed by any command-specific data. A **Request** or **Event**
  payload has no status prefix — it's just the command-specific data.

## Message types

| Value | Name       | Direction        |
|-------|------------|-------------------|
| 0x01  | Request    | Host → Device        |
| 0x02  | Response   | Device → Host (reply)   |
| 0x03  | Event      | Device → Host (unsolicited) |

The device only ever accepts `Request` frames; anything else received is a
protocol error.

## Peripheral types

| Value | Name    |
|-------|---------|
| 0x00  | System  |
| 0x01  | UART    |
| 0x02  | I2C (reserved, not implemented) |

## Commands

| Value  | Name               | Peripheral | Request payload           | Response/Event payload (after status, for Responses) |
|--------|--------------------|-----------|-----------------------------|----------------------------------------------------------|
| 0x0001 | GetProtocolInfo    | System    | —                             | not implemented                                            |
| 0x0002 | Ping               | System    | —                             | not implemented                                             |
| 0x0003 | GetCapabilities    | System    | —                             | `fw_major:u8, fw_minor:u8, fw_patch:u8, supported_peripherals_mask:u32` |
| 0x0004 | GetStatus          | System    | —                             | not implemented                                              |
| 0x0100 | Configure          | UART      | `baud_rate:u32, data_bits:u8, parity:u8, stop_bits:u8` | same shape, echoing the applied config |
| 0x0101 | GetConfig          | UART      | —                             | same shape as Configure's request                             |
| 0x0102 | Enable             | —         | —                             | not implemented                                                 |
| 0x0103 | Disable            | —         | —                             | not implemented                                                  |
| 0x0104 | Reset              | System, UART | —                          | (status only)                                                     |
| 0x0200 | UartWrite          | UART      | `execute_after_us:u32, data_length:u16, data[data_length]` | `bytes_accepted:u16` |
| 0x0300–0x03FF | (I2C, reserved) | I2C  | not implemented                | not implemented                                                    |
| 0x8000 | UartRxEvent (Event) | UART      | —                          | `timestamp_us:u32, data_length:u16, data[data_length]`               |
| 0x8001 | HeartbeatEvent (Event) | System | —                        | (no payload) — emitted every ~1000ms                                  |
| 0x8F00 | ErrorEvent (Event) | System    | —                             | `status:u16` — emitted on a frame-level protocol error (bad version, wrong message type, oversized payload, bad CRC) |
| 0x8F01 | OverflowEvent (Event) | System | —                          | not implemented                                                          |

`Parity`: `0=None, 1=Even, 2=Odd`. `StopBits`: `0=One, 1=Two`.

## Status codes

| Value | Name              |
|-------|-------------------|
| 0     | Ok                |
| 1     | InvalidCommand    |
| 2     | InvalidParameter  |
| 3     | InvalidState      |
| 4     | NotSupported      |
| 5     | Busy              |
| 6     | Timeout           |
| 7     | BufferFull        |
| 8     | Nack (reserved for I2C) |
| 9     | HardwareError     |
| 10    | ProtocolError     |

## Rules

- All multi-byte fields are little-endian.
- A peripheral-address field (relevant to the future I²C work) is always
  represented **unshifted** on the wire (the canonical 7-bit address,
  e.g. `0x38`) — a board's own driver, not this protocol, is responsible
  for any HAL-specific shift (e.g. STM32 HAL's `address << 1`).
