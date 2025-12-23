# PC-ALE-PAL Planning Document

## ⚠️ CLAUDE: READ THIS FIRST ⚠️

**This is your working document. Before writing ANY code:**
1. Check this document for current status
2. Update this document with what you're about to do
3. Update this document when you finish

**DO NOT create files without updating this document first.**

---

## What is PAL?

**PAL is Layers 8 + 9 from MARS-ALE architecture:**
- Layer 8: Radio Abstraction (interfaces)
- Layer 9: Radio Drivers (protocol encoders)

**PAL translates:** "Set 14.250 MHz, USB" → bytes for your specific radio

```
ALE Core: "14.250 MHz, USB"
    ↓
PAL: "You have FT-857, so send: 00 25 14 00 01"
    ↓
Platform: writes bytes to COM port
```

---

## What's IN PAL

### Interfaces (contracts - pure virtual)
| Interface | File | Purpose |
|-----------|------|---------|
| IRadio | radio.h | frequency, mode, PTT control |
| ISerial | serial.h | byte I/O abstraction |
| IAudioDriver | audio_driver.h | sound card in/out |
| ITimer | timer.h | monotonic time, sleep |
| ILogger | logger.h | logging |
| IEventHandler | events.h | async event notifications |

### Radio Protocols (encode commands to bytes)
| Radio | File | Protocol |
|-------|------|----------|
| Icom | icom_civ.cpp | CI-V |
| Yaesu | yaesu_cat.cpp | CAT |
| Kenwood | kenwood.cpp | Kenwood |
| Elecraft | elecraft.cpp | Elecraft |

### Utilities (pure math, no OS calls)
| Utility | File | Purpose |
|---------|------|---------|
| Resampler | resampler.cpp | 48kHz ↔ 8kHz |

---

## What's NOT in PAL

| Thing | Where it goes | Why |
|-------|---------------|-----|
| STANAG 5066 / ISIS | Above ALE (own layer) | Application interface, not platform |
| ALE protocol | PC-ALE core | Protocol logic |
| Channel tables | PC-ALE core | ALE configuration |
| Serial I/O impl | Platform (Linux/Windows) | OS-specific |
| GPIO impl | Platform | OS-specific |
| Audio impl | Platform | OS-specific |

---

## Project Info

**Repository:** PC-ALE-PAL  
**GitHub:** https://github.com/Alex-Pennington/PC-ALE-PAL  
**Local Path:** D:\claude_sandbox\PC-ALE-PAL\  
**License:** MIT

---

## Architecture

```
┌─────────────────────────────────────────┐
│         Platform App (PC-ALE-Linux)     │
│                                         │
│  ┌─────────────────────────────────┐    │
│  │  PC-ALE core (ALE protocol)     │    │
│  │  - Channel tables               │    │
│  │  - ALE state machine            │    │
│  │  - "go to channel 5"            │    │
│  └──────────────┬──────────────────┘    │
│                 │ "14.250 MHz, USB"     │
│  ┌──────────────▼──────────────────┐    │
│  │  PAL (this repo)                │    │
│  │  - IRadio interface             │    │
│  │  - Radio protocols (CI-V, CAT)  │    │
│  │  - "send these bytes"           │    │
│  └──────────────┬──────────────────┘    │
│                 │ bytes                 │
│  ┌──────────────▼──────────────────┐    │
│  │  Platform implementations       │    │
│  │  - LinuxSerial (termios)        │    │
│  │  - ALSA audio                   │    │
│  │  - GPIO                         │    │
│  └─────────────────────────────────┘    │
└─────────────────────────────────────────┘
```

---

## Directory Structure

```
PC-ALE-PAL/
├── planning_document.md
├── README.md
├── LICENSE
├── CMakeLists.txt
│
├── include/pal/
│   ├── audio_driver.h      # IAudioDriver interface
│   ├── serial.h            # ISerial interface
│   ├── timer.h             # ITimer interface
│   ├── radio.h             # IRadio interface
│   ├── logger.h            # ILogger interface
│   ├── events.h            # IEventHandler interface
│   ├── resampler.h         # Resampler utility
│   └── radios/             # Protocol headers
│       ├── icom_civ.h
│       ├── yaesu_cat.h
│       ├── kenwood.h
│       └── elecraft.h
│
├── src/
│   ├── radios/             # Protocol encoders
│   │   ├── icom_civ.cpp
│   │   ├── yaesu_cat.cpp
│   │   ├── kenwood.cpp
│   │   └── elecraft.cpp
│   │
│   └── common/
│       └── resampler.cpp   # Sample rate conversion
│
└── tests/
    ├── test_resampler.cpp
    └── test_radios.cpp     # Protocol encoding tests
```

---

## Interface Status

| Interface | File | Status |
|-----------|------|--------|
| IAudioDriver | audio_driver.h | ✅ Complete |
| ISerial | serial.h | ✅ Complete |
| ITimer | timer.h | ✅ Complete |
| IRadio | radio.h | ✅ Complete |
| ILogger | logger.h | ✅ Complete |
| IEventHandler | events.h | ✅ Complete |

## Radio Protocol Status

| Radio | File | Status |
|-------|------|--------|
| Icom CI-V | icom_civ.cpp | ✅ Complete |
| Yaesu CAT | yaesu_cat.cpp | ✅ Complete |
| Kenwood | kenwood.cpp | ✅ Complete |
| Elecraft | elecraft.cpp | ✅ Complete |

## Utility Status

| Utility | File | Status |
|---------|------|--------|
| Resampler | resampler.cpp | ✅ Complete |

---

## Progress Log

| Date | Action |
|------|--------|
| 2024-12-22 | Created repo, initial structure |
| 2024-12-22 | Created IAudioDriver, ITimer |
| 2024-12-22 | Implemented resampler + tests |
| 2024-12-23 | Received STANAG 5066 spec |
| 2024-12-23 | Received PC-ALE 1.x radio.dll spec |
| 2024-12-23 | Created IRadio (PTT included) |
| 2024-12-23 | Created ILogger, IEventHandler |
| 2024-12-23 | Removed ISIS - not PAL's job |
| 2024-12-23 | Clarified PAL = Layers 8+9 (radio abstraction + protocols) |
| 2024-12-23 | Created ISerial interface |
| 2024-12-23 | Created Icom CI-V protocol encoder |
| 2024-12-23 | Created Yaesu CAT protocol encoder |
| 2024-12-23 | Created Kenwood protocol encoder |
| 2024-12-23 | Created Elecraft protocol encoder |

---

## Next Steps

1. ✅ Create ISerial interface
2. ✅ Create radio protocol encoders (CI-V, Yaesu, Kenwood, Elecraft)
3. ❌ Tests for protocol encoders
4. ❌ Commit and push to GitHub

---

*Last Updated: 2024-12-23*
