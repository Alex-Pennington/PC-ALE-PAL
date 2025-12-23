# PC-ALE-PAL Architecture

## Layer Model Alignment

PC-ALE-PAL implements **Layers 8 and 9** of the MARS-ALE architecture as defined by the core team.

> **Note:** The MARS-ALE layer model uses different numbering than the OSI 7-layer reference model.
> In the MARS-ALE/MIL-STD-188-141 architecture:
> - **Layer 8**: Subnetwork Access - Hardware abstraction (radio control, serial I/O, audio I/O)
> - **Layer 9**: Subnetwork Dependent Convergence - Physical layer (modem, signal processing)
>
> This is NOT the same as "OSI Layer 8" (which doesn't exist in OSI and is sometimes used humorously to refer to users/politics).

## PAL Scope

PAL (Platform Abstraction Layer) provides the Layer 8-9 interfaces and protocol encoders:

```
┌─────────────────────────────────────────────────────────────┐
│                    PC-ALE Core (Layers 1-7)                 │
│                  (ALE Protocol, Channel Tables)             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│                    PC-ALE-PAL (Layers 8-9)                  │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              Layer 8: Subnetwork Access              │   │
│  │                                                      │   │
│  │  Interfaces:          Radio Protocol Encoders:       │   │
│  │  ├── IRadio           ├── Icom CI-V                 │   │
│  │  ├── ISerial          ├── Yaesu CAT                 │   │
│  │  ├── IAudioDriver     ├── Kenwood                   │   │
│  │  ├── ITimer           └── Elecraft                  │   │
│  │  ├── ILogger                                        │   │
│  │  └── IEventHandler                                  │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │     Layer 9: Subnetwork Dependent Convergence       │   │
│  │                                                      │   │
│  │  Utilities:                                         │   │
│  │  └── Resampler (48kHz ↔ 8kHz)                       │   │
│  │                                                      │   │
│  │  (Modem core lives in phoenix-sdr-core, not PAL)    │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                  Platform Implementations                   │
│            (Linux, Windows, Embedded - NOT in PAL)          │
│                                                             │
│  Examples:                                                  │
│  ├── LinuxSerial : ISerial                                 │
│  ├── WindowsSerial : ISerial                               │
│  ├── PortAudioDriver : IAudioDriver                        │
│  └── RaspberryPiGPIO : IRadio (for PTT)                    │
└─────────────────────────────────────────────────────────────┘
```

## Layer 8: Subnetwork Access

Layer 8 provides hardware abstraction through:

### Interfaces (Pure Virtual)
- **IRadio** - Radio control abstraction (frequency, mode, PTT)
- **ISerial** - Serial port abstraction (platform-independent)
- **IAudioDriver** - Audio I/O abstraction
- **ITimer** - Timing abstraction
- **ILogger** - Logging abstraction
- **IEventHandler** - Event callback abstraction

### Radio Protocol Encoders
Concrete implementations that encode commands for specific radio protocols:
- **Icom CI-V** - Binary protocol with BCD frequency encoding (LSB first)
- **Yaesu CAT** - 5-byte binary commands with packed BCD (MSB first)
- **Kenwood** - ASCII commands with semicolon terminator
- **Elecraft** - Kenwood-compatible with extensions

## Layer 9: Subnetwork Dependent Convergence

Layer 9 handles physical layer concerns:

### In PAL
- **Resampler** - Sample rate conversion (48kHz ↔ 8kHz) for audio interface compatibility

### Outside PAL (in phoenix-sdr-core)
- MIL-STD-188-110A modem implementation
- Signal processing, modulation/demodulation
- FEC encoding/decoding

## What's NOT in PAL

The following are explicitly outside PAL's scope:

| Component | Location | Reason |
|-----------|----------|--------|
| ALE Protocol | PC-ALE Core | Application logic, not hardware abstraction |
| Channel Tables | PC-ALE Core | Configuration data, not interfaces |
| Serial I/O Implementation | Platform-specific | OS-dependent (Linux/Windows/Embedded) |
| Audio I/O Implementation | Platform-specific | OS-dependent |
| Modem DSP | phoenix-sdr-core | Separate module with its own concerns |
| STANAG 5066 | Application Layer | Above Layer 7, uses ALE services |

## Repository Structure

```
PC-ALE-PAL/
├── include/pal/
│   ├── interfaces/        # Layer 8 interfaces
│   │   ├── i_radio.h
│   │   ├── i_serial.h
│   │   ├── i_audio_driver.h
│   │   ├── i_timer.h
│   │   ├── i_logger.h
│   │   └── i_event_handler.h
│   ├── radios/            # Layer 8 protocol encoders
│   │   ├── icom_civ.h
│   │   ├── yaesu_cat.h
│   │   ├── kenwood.h
│   │   └── elecraft.h
│   └── common/            # Layer 9 utilities
│       └── resampler.h
├── src/
│   ├── radios/            # Protocol encoder implementations
│   └── common/            # Utility implementations
└── tests/
```

## References

- MIL-STD-188-141B - Interoperability and Performance Standards for Medium and High Frequency Radio Systems
- MARS-ALE Core Team Architecture Document
- Steve Hajducek (N2CKH) - PC-ALE/MARS-ALE original author
