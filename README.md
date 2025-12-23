# PC-ALE-PAL

Platform Abstraction Layer for PC-ALE.

## What is PAL?

PAL is **Layers 8 + 9** from the MARS-ALE architecture:
- **Layer 8: Subnetwork Access** - Hardware abstraction interfaces (IRadio, ISerial, IAudioDriver)
- **Layer 9: Subnetwork Dependent Convergence** - Protocol encoders (CI-V, CAT, Kenwood)

> **Note:** MARS-ALE uses MIL-STD layer numbering, NOT OSI. Layers 8-9 refer to the
> hardware/radio abstraction layers defined by the MARS-ALE core team - not the
> humorous "OSI Layer 8 = users" concept.

PAL translates "Set 14.250 MHz, USB" into bytes for your specific radio:

```
ALE Core: "14.250 MHz, USB"
    ↓
PAL: "You have FT-857, so send: 01 42 50 00 01"
    ↓
Platform: writes bytes to COM port
```

## What's Included

### Interfaces (pure virtual contracts)

| Interface | File | Purpose |
|-----------|------|---------|
| IRadio | radio.h | Frequency, mode, PTT control |
| ISerial | serial.h | Serial port abstraction |
| IAudioDriver | audio_driver.h | Sound card in/out |
| ITimer | timer.h | Monotonic time, sleep |
| ILogger | logger.h | Logging |
| IEventHandler | events.h | Async event notifications |

### Radio Protocols (encode commands to bytes)

| Radio | File | Protocol | Example Output |
|-------|------|----------|----------------|
| Icom | icom_civ.cpp | CI-V | `FE FE 94 E0 05 00 00 25 14 00 FD` |
| Yaesu | yaesu_cat.cpp | CAT | `01 42 50 00 01` |
| Kenwood | kenwood.cpp | ASCII | `FA00014250000;` |
| Elecraft | elecraft.cpp | Kenwood-compatible | `FA00014250000;` |

### Utilities (platform-independent)

| Utility | File | Purpose |
|---------|------|---------|
| Resampler | resampler.cpp | 48kHz ↔ 8kHz sample rate conversion |

## What's NOT Included

| Thing | Where it belongs |
|-------|------------------|
| Serial I/O implementation | Platform repo (PC-ALE-Linux, PC-ALE-Windows) |
| Audio I/O implementation | Platform repo |
| GPIO implementation | Platform repo |
| ALE protocol | PC-ALE core |
| Channel tables | PC-ALE core |
| STANAG 5066 | Application layer (above ALE) |

## Directory Structure

```
PC-ALE-PAL/
├── include/pal/
│   ├── audio_driver.h
│   ├── serial.h
│   ├── timer.h
│   ├── radio.h
│   ├── logger.h
│   ├── events.h
│   ├── resampler.h
│   └── radios/
│       ├── icom_civ.h
│       ├── yaesu_cat.h
│       ├── kenwood.h
│       └── elecraft.h
│
├── src/
│   ├── radios/
│   │   ├── icom_civ.cpp
│   │   ├── yaesu_cat.cpp
│   │   ├── kenwood.cpp
│   │   └── elecraft.cpp
│   └── common/
│       └── resampler.cpp
│
└── tests/
    └── test_resampler.cpp
```

## Usage

Include PAL as a submodule in your platform repo:

```bash
git submodule add https://github.com/Alex-Pennington/PC-ALE-PAL.git extern/PAL
```

Then implement the interfaces for your platform:

```cpp
// Your platform implements ISerial
class LinuxSerial : public pal::ISerial {
    bool open(const std::string& port, const SerialConfig& config) override {
        fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY);
        // ... termios setup ...
    }
    size_t write(const uint8_t* data, size_t len) override {
        return ::write(fd_, data, len);
    }
    // ... etc ...
};

// Use PAL's radio protocol with your serial implementation
LinuxSerial serial;
serial.open("/dev/ttyUSB0", {9600, 8, Parity::NONE, StopBits::ONE});

pal::IcomCiv radio(&serial, pal::IcomRadioAddress::IC_7300);
radio.initialize();

// Set frequency - PAL encodes to CI-V bytes, your serial sends them
pal::Channel ch;
ch.rx_frequency = 14250000;  // 14.250 MHz
ch.rx_mode = pal::RadioMode::USB;
radio.set_channel(ch);

// PTT
radio.set_ptt(true);   // Transmit
radio.set_ptt(false);  // Receive
```

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Supported Radios

### Icom (CI-V protocol)
IC-706, IC-718, IC-746, IC-756, IC-7000, IC-7100, IC-7200, IC-7300, IC-7600, IC-7610, IC-7700, IC-7800, IC-7850, IC-7851, IC-9700, and others

### Yaesu (CAT protocol)
FT-817, FT-857, FT-897, FT-991, and others using 5-byte CAT commands

### Kenwood (ASCII protocol)
TS-480, TS-590, TS-890, TS-990, and others

### Elecraft (Kenwood-compatible)
K2, K3, K3S, KX2, KX3, and others

## Architecture

```
┌─────────────────────────────────────────┐
│         Platform App                    │
│  ┌─────────────────────────────────┐    │
│  │  PC-ALE core                    │    │
│  │  - ALE state machine            │    │
│  │  - Channel tables               │    │
│  └──────────────┬──────────────────┘    │
│                 │ "14.250 MHz, USB"     │
│  ┌──────────────▼──────────────────┐    │
│  │  PAL (this repo)                │    │
│  │  - IRadio interface             │    │
│  │  - Radio protocols              │    │
│  └──────────────┬──────────────────┘    │
│                 │ bytes                 │
│  ┌──────────────▼──────────────────┐    │
│  │  Platform implementations       │    │
│  │  - ISerial (termios/Win32)      │    │
│  │  - IAudioDriver (ALSA/WASAPI)   │    │
│  └─────────────────────────────────┘    │
└─────────────────────────────────────────┘
```

## License

MIT

## Author

Alex Pennington, AAM402/KY4OLB
