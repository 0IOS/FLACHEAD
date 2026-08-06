# 05_DEBUGGING_GUIDE.md

# FLACHEAD UI — Debugging Guide

## 1. Purpose

This document defines the debugging workflow for FLACHEAD.

The goal is to make diagnosing problems fast and systematic.

FLACHEAD runs on constrained hardware:

* Raspberry Pi Zero W
* limited RAM
* limited CPU
* SD card storage

Debugging must focus on finding the root cause without adding unnecessary overhead.

Primary principle:

> Measure first, modify second.

---

# 2. Debugging philosophy

When a problem appears:

Follow this order:

```text id="u7m3q8"
Observe

↓

Collect information

↓

Identify subsystem

↓

Reproduce

↓

Fix

↓

Verify
```

---

# 3. System debugging layers

Debug problems by layer:

```text id="x5m8p2"
Hardware

↓

Drivers

↓

Services

↓

Events

↓

UI

↓

Components
```

---

Example:

Touch button does nothing:

Do not immediately edit UI code.

Check:

```text id="r8m3q6"
Touch Hardware

↓

Input Driver

↓

Input Event

↓

Navigation Manager

↓

Screen
```

---

# 4. Logging system

FLACHEAD uses structured logging.

Log format:

```text id="k4m8x9"
[TIME]

[LEVEL]

[SYSTEM]

[MESSAGE]
```

---

Example:

```text id="m6x2q5"
12:04:32

INFO

Audio

DSP connected
```

---

# 5. Log levels

## DEBUG

Detailed development information.

Example:

```text id="q9m3x7"
Button GPIO state changed
```

---

## INFO

Normal system events.

Example:

```text id="p5x8m2"
Library scan complete
```

---

## WARNING

Unexpected but recoverable.

Example:

```text id="v3m7q1"
Artwork missing
```

---

## ERROR

Failed operation.

Example:

```text id="a8x4m6"
Unable to open audio device
```

---

# 6. Logging rules

Always log:

* subsystem startup
* hardware connection
* major state changes
* errors
* recovery attempts

---

Do not log:

* every frame
* every pixel operation
* audio samples

---

# 7. Debug mode

Development builds should enable:

```text id="f7m2x9"
Verbose logging

Debug overlays

Performance counters

Event monitor
```

---

Production builds:

Disable expensive diagnostics.

---

# 8. Event debugging

Because FLACHEAD uses events, event tracing is important.

Debug view:

```text id="n5m8x3"
EVENT LOG

12:01:04

TRACK_CHANGED


12:01:10

VOLUME_CHANGED
```

---

Useful for:

* navigation bugs
* audio synchronization
* input issues

---

# 9. Event debugging workflow

Example:

Play button does not work.

Check:

```text id="w6x3m8"
1. Button generated event?

↓

2. Event Bus received event?

↓

3. Audio Session subscribed?

↓

4. Playback changed?
```

---

# 10. Performance debugging

Measure:

## CPU

Check:

* renderer load
* background tasks
* decoding load

---

## RAM

Check:

* leaks
* excessive caching
* texture memory

---

## FPS

Check:

* frame time
* expensive components
* animations

---

# 11. Frame debugging

Monitor:

```text id="c7m4x2"
FPS

Frame Time

Draw Calls

Texture Usage
```

---

Warning signs:

Frame drops during:

* screen changes
* scrolling
* artwork loading

---

# 12. Pi Zero W profiling

The Pi Zero W requires special attention.

Monitor:

```text id="y8m3x5"
CPU usage

Memory usage

Temperature

Storage speed
```

---

Optimization priority:

```text id="q4m8x6"
Remove unnecessary work

↓

Cache data

↓

Reduce rendering

↓

Optimize algorithms
```

---

# 13. UI debugging

Common problems:

---

## Wrong alignment

Check:

* screen dimensions
* scaling values
* component bounds

---

## Slow UI

Check:

* rendering loop
* image decoding
* excessive redraws

---

## Touch mismatch

Check:

* coordinate conversion
* rotation settings
* calibration

---

# 14. Audio debugging

Audio problems require separate analysis.

---

## No sound

Check:

```text id="m8x2q7"
DSP detected?

↓

Audio backend active?

↓

Track loaded?

↓

Playback running?
```

---

## Stuttering

Check:

* CPU usage
* buffer size
* storage speed
* decoder load

---

## Wrong metadata

Check:

* parser
* database
* cache

---

# 15. Hardware debugging

Hardware issues should be isolated.

---

Example:

Button failure:

```text id="x3m7q9"
Physical button

↓

GPIO reading

↓

Input backend

↓

Event creation
```

---

Do not debug from the screen backward.

---

# 16. Crash debugging

When FLACHEAD crashes:

Collect:

* logs
* last event
* current screen
* memory state
* hardware state

---

Crash report:

```text id="b5m8x4"
Time:

Screen:

Action:

Logs:

Backtrace:

Hardware:
```

---

# 17. Core dump handling

Development builds should support:

```text id="p3x8m6"
Core dumps
```

---

Useful information:

* crashed function
* memory state
* call stack

---

# 18. Common failure scenarios

## Missing SD card music

Expected:

```text id="v7m2x1"
Empty library state
```

---

## Corrupt FLAC file

Expected:

```text id="k9m4x8"
Skip file

Show warning
```

---

## DSP removed during playback

Expected:

```text id="r2m7x5"
Pause playback

Notify user

Attempt recovery
```

---

# 19. Debug tools

Recommended tools:

```text id="s5x8m3"
gdb

valgrind

perf

top

htop

journalctl
```

---

Use lightweight profiling on Pi.

Heavy tools should run during development on desktop builds when possible.

---

# 20. Development workflow

Recommended:

```text id="h4m9x2"
Develop on PC

↓

Test architecture

↓

Cross compile

↓

Deploy to Pi

↓

Hardware validation
```

---

# 21. Debug builds vs release builds

Debug build:

Enabled:

* logs
* assertions
* diagnostics

---

Release build:

Enabled:

* optimization
* reduced logging
* smaller binary

---

# 22. Assertions

Use assertions for:

Programming mistakes:

```cpp id="z6m3x8"
assert(renderer != nullptr);
```

---

Do not use assertions for:

Expected failures:

* missing artwork
* missing files
* disconnected devices

---

# 23. Recovery-first design

When possible:

Prefer:

```text id="n7x2m5"
Detect problem

↓

Recover

↓

Continue
```

---

Instead of:

```text id="m4x8q1"
Detect problem

↓

Crash
```

---

# 24. Debug checklist

Before reporting a bug:

```text id="w3m8q6"
☐ Reproduced issue

☐ Checked logs

☐ Identified subsystem

☐ Tested on clean build

☐ Verified hardware state
```

---

# 25. Acceptance criteria

Debugging system is complete when:

* logs explain failures
* events can be traced
* performance can be measured
* crashes provide useful data
* hardware issues can be isolated

---

# 26. Implementation notes for AI coding agent

When debugging:

* Do not randomly change code.
* Collect evidence first.
* Check architecture boundaries.
* Fix root causes.
* Add logging where future diagnosis improves.

A well-debugged FLACHEAD should be understandable even when running unattended as a standalone music device.
