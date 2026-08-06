# 04_PERFORMANCE_CHECKLIST.md

# FLACHEAD UI — Performance Checklist

## 1. Purpose

This document defines performance requirements and optimization rules for FLACHEAD UI.

FLACHEAD targets Raspberry Pi Zero W hardware, which has limited:

* CPU power
* RAM
* GPU capability
* storage speed

The goal is not maximum graphical complexity.

The goal is:

* smooth interaction
* stable playback
* predictable performance

Primary principle:

> Consistent performance is more important than unnecessary visual effects.

---

# 2. Performance targets

Target device:

```text id="x7m4q9"
Raspberry Pi Zero W
```

---

Required targets:

```text id="m5x8q3"
UI Frame Rate:

30 FPS minimum

45 FPS preferred

60 FPS ideal
```

---

Frame timing:

```text id="v8m3x5"
60 FPS:

<16.6ms/frame


30 FPS:

<33.3ms/frame
```

---

# 3. Audio priority

Audio always has highest priority.

The system must maintain:

* stable buffers
* uninterrupted decoding
* reliable DSP communication

---

Never sacrifice:

```text id="c6m4x9"
Audio

for

Animation
```

---

# 4. Rendering optimization

Required:

* dirty rectangle rendering
* resource caching
* minimal redraws
* simple primitives

---

Avoid:

* full screen redraws every frame
* unnecessary animations
* expensive effects

---

# 5. Memory optimization

Rules:

* avoid unnecessary allocations
* reuse buffers
* release unused resources

---

Avoid:

```text id="r7m3x8"
Creating objects every frame
```

---

Prefer:

```text id="n8m4x6"
Create once

Reuse many times
```

---

# 6. Asset optimization

Images:

* resize before rendering
* cache decoded versions
* avoid oversized artwork

---

Fonts:

* load once
* reuse globally

---

# 7. CPU optimization

Avoid:

* expensive calculations in render loop
* repeated file access
* unnecessary polling

---

Prefer:

* events
* background workers
* cached results

---

# 8. Storage optimization

SD card operations are slow.

Avoid:

* frequent writes
* loading assets repeatedly
* scanning libraries unnecessarily

---

Use:

* caching
* indexing
* delayed writes

---

# 9. Animation rules

Animations should be:

* short
* simple
* interruptible

---

Recommended:

```text id="w5m3x7"
100-300ms transitions
```

---

Avoid:

* continuous heavy animations
* complex effects

---

# 10. Input responsiveness

Input latency target:

```text id="a7m4x8"
<50ms
```

---

Input handling must:

* prioritize user actions
* avoid blocking operations

---

# 11. Background task limits

Recommended:

```text id="p6m8x2"
Audio Thread

UI Thread

Input Thread

1-2 Worker Threads
```

---

Avoid excessive concurrency.

---

# 12. Startup optimization

Boot process:

```text id="u5m3x9"
Initialize Required Systems

↓

Start UI

↓

Load Optional Resources
```

---

Do not delay the entire interface for optional data.

---

# 13. Battery optimization

For portable operation:

Use:

* display timeout
* idle sleep
* reduced animation mode

---

Avoid:

* unnecessary CPU wakeups

---

# 14. Debug performance tools

Development mode should expose:

```text id="k8m4x5"
FPS

Frame Time

RAM Usage

CPU Usage

Current Screen

Active Tasks
```

---

# 15. Performance testing checklist

Before release:

```text id="d7m3x8"
✓ Boot is fast

✓ Navigation is smooth

✓ Audio never drops

✓ Memory remains stable

✓ Large libraries work

✓ Artwork loading is smooth

✓ Long playback works
```

---

# 16. Common performance mistakes

Avoid:

```text id="h5m8x2"
Large UI frameworks

Heavy animations

Uncached images

Blocking operations

Too many threads

Repeated allocations
```

---

# 17. AI implementation rules

When generating code:

* Prefer simple solutions.
* Measure before optimizing.
* Do not add dependencies without reason.
* Test on target hardware.
* Keep desktop compatibility where possible.

---

# 18. Acceptance criteria

Performance requirements are satisfied when:

* UI feels responsive
* playback is stable
* memory usage is predictable
* Pi Zero W can sustain normal operation
* no subsystem starves another

---

# 19. Final checklist

```text id="s6m4x9"
Architecture ✓

Rendering ✓

Memory ✓

Threads ✓

Audio Sync ✓

Testing ✓
```

---

FLACHEAD is designed as a dedicated embedded device, not a desktop application. Every optimization decision should prioritize reliability and user experience over complexity.
