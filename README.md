# PC-ALE-PAL

Platform Abstraction Layer for PC-ALE.

## What is PAL?

PAL is a contract. It defines WHAT functions exist, not HOW they work.

```cpp
class IRadio {
    virtual void set_ptt(bool tx) = 0;  // WHAT
};
```

Platform implementations provide the HOW:

```cpp
// In PC-ALE-Linux
void DrawsRadio::set_ptt(bool tx) {
    gpio_set_value(12, tx ? 1 : 0);  // HOW
}
```

## Why?

So PC-ALE core can be written once and run anywhere. The core calls `radio->set_ptt(true)`. It doesn't know or care if that's GPIO, serial RTS, or CAT command.

## What's Included

**Interfaces (pure virtual):**
- `IAudioDriver` - Sound card in/out
- `ITimer` - Monotonic time, sleep
- `IRadio` - Frequency, mode, PTT, power, antenna
- `ILogger` - Logging
- `IEventHandler` - Async events
- `ISIS` - STANAG 5066 Subnet Interface Sublayer

**Utilities (platform-independent):**
- `Resampler` - 48kHz ↔ 8kHz sample rate conversion

## What's NOT Included

- Implementations (see PC-ALE-Linux, PC-ALE-Windows)
- PC-ALE core (separate repo)
- Platform-specific code

## Usage

Include PAL as a submodule in your platform repo:

```bash
git submodule add https://github.com/Alex-Pennington/PC-ALE-PAL.git extern/PAL
```

Then implement the interfaces for your platform.

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## License

MIT
