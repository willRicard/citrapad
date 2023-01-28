# Citrapad

Controller to touchscreen input Citra UDP client.

## About

To play my favorite 3DS game [Kid Icarus: Uprising](https://en.wikipedia.org/wiki/Kid_Icarus:_Uprising)
on [Citra emulator](https://citra-emu.org), I had to find a way to use a game controller joystick as mock
stylus input. Citra supports UDP input since <https://github.com/citra-emu/citra/pull/4049> but I found
available clients unsatisfactory. Thus, citrapad was born.

# Build instructions

`citrapad` requires SDL2 to be installed.

```
cmake -B build .
cmake --build build
```

## Configuration

Append these lines to the Citra `[Controls]` config to define the touchscreen mapping ranges:

```
profiles\1\motion_device\default=false
profiles\1\motion_device="engine:motion_emu,sensitivity:0.010000"
profiles\1\touch_device\default=false
profiles\1\touch_device="engine:cemuhookudp,max_x:16384,max_y:16384,min_x:0,min_y:0"
profiles\1\udp_input_address\default=true
profiles\1\udp_input_address=127.0.0.1
profiles\1\udp_input_port\default=true
profiles\1\udp_input_port=26760
profiles\1\udp_pad_index\default=true
profiles\1\udp_pad_index=0
```

**Important**: So far, packet CRC computation is not implemented. To use citrapad, edit `input_common/udp/protocol.cpp` to remove the lines:

```
-    if (crc32 != result.checksum()) {
-        LOG_ERROR(Input, "UDP Packet CRC check failed. Offset: {}", offsetof(Header, crc));
-        return std::nullopt;
-    }
```
