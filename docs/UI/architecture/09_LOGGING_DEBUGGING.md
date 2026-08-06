# 09_LOGGING_DEBUGGING.md

# FLACHEAD UI — Logging and Debugging Specification

## 1. Purpose

This document defines the logging and debugging architecture for FLACHEAD.

Because FLACHEAD runs on Raspberry Pi Zero W hardware, debugging can be difficult compared to a desktop environment.

The logging system provides visibility into:

* UI behavior
* hardware problems
* playback issues
* performance problems
* crashes

Primary principle:

> Debug information should help development without affecting normal device performance.

---

# 2. Design goals

The logging system must be:

* lightweight
* configurable
* structured
* safe
* production-friendly

---

# 3. Logging architecture

Data flow:

```text id="x7m4q9"
System Component

↓

Logger

↓

Log Queue

↓

Storage / Console
```

---

Components using logging:

```text id="m5x8q3"
UI

Audio Engine

Hardware

Database

Network

System Services
```

---

# 4. Log levels

FLACHEAD uses:

```text id="v8m3x5"
TRACE

DEBUG

INFO

WARNING

ERROR

CRITICAL
```

---

# 5. Production logging

Default mode:

```text id="c6m4x9"
INFO

WARNING

ERROR
```

---

Avoid excessive logging.

Reason:

* SD card wear
* CPU usage
* storage usage

---

# 6. Development mode

Developer mode enables:

```text id="r7m3x8"
DEBUG

TRACE
```

---

Useful for:

* UI testing
* hardware debugging
* performance analysis

---

# 7. Log format

Every log entry contains:

```text id="n8m4x6"
Timestamp

Component

Level

Message

Optional Data
```

---

Example:

```text id="w5m3x7"
12:05:22

Renderer

WARNING

Frame time exceeded limit
```

---

# 8. Component logging

Each subsystem should have its own category.

Example:

```text id="a7m4x8"
UI.RENDERER

AUDIO.PLAYER

HW.DSP

INPUT.GPIO

DB.LIBRARY
```

---

Benefits:

* easier filtering
* easier debugging

---

# 9. Crash logging

When FLACHEAD crashes:

Store:

```text id="p6m8x2"
Crash Time

Component

Error

Stack Information

Current State
```

---

Do not lose the previous session information.

---

# 10. Recovery logging

Important events:

```text id="u5m3x9"
Startup Recovery

Database Repair

Fallback Mode

Hardware Failure
```

---

---

# 11. Performance logging

The system should optionally measure:

```text id="k8m4x5"
Frame Time

CPU Usage

Memory Usage

Audio Buffer Status

Task Duration
```

---

---

# 12. Frame performance monitoring

Renderer debug mode:

Displays:

```text id="d7m3x8"
FPS

Frame Time

Memory

Current Screen
```

---

Only enabled during development.

---

# 13. Audio debugging

Audio logs:

Track:

* decoder state
* buffer underruns
* DSP connection
* playback errors

---

Never spam logs during normal playback.

---

# 14. Hardware debugging

Hardware logs:

Examples:

```text id="h5m8x2"
GPIO Initialized

Touch Calibration Loaded

DSP Connected

Display Started
```

---

---

# 15. Log storage

Storage options:

```text id="s6m4x9"
Temporary RAM Logs

↓

Persistent Logs

↓

Export
```

---

Avoid unlimited log growth.

---

# 16. Log rotation

Required:

```text id="b8m3x5"
Maximum Size

Old Log Removal

Compression Option
```

---

Reason:

Protect SD card space.

---

# 17. Debug overlay

Optional developer feature:

```text id="x7m4q2"
FPS: 58

RAM: 210MB

Screen: Player
```

---

Should never appear in normal use.

---

# 18. Error reporting

Errors should include:

* what failed
* where it failed
* recovery action

---

Bad:

```text id="d8m4x6"
Error
```

---

Good:

```text id="m5x8q3"
Artwork Loader

Failed loading image

Using fallback artwork
```

---

# 19. Performance requirements

Logging must:

* run asynchronously
* avoid blocking threads
* avoid excessive allocations

---

Critical:

Audio thread should not perform heavy logging.

---

# 20. Acceptance criteria

Implementation is complete when:

* logs identify failures
* debugging mode works
* crashes leave useful information
* normal operation remains lightweight
* logs can be cleared/exported

---

# 21. Architectural recommendations

Required module:

```text id="v8m3x5"
Logger Service

├── Log Levels

├── Log Queue

├── Storage Writer

├── Crash Handler

└── Debug Tools
```

---

# 22. Implementation notes for AI coding agent

When implementing:

* Keep logging separate from business logic.
* Never let logging block critical systems.
* Add useful context to errors.
* Make debug tools optional.
* Test failure scenarios deliberately.

A good logging system will dramatically reduce development time when debugging FLACHEAD on real Raspberry Pi Zero W hardware.
