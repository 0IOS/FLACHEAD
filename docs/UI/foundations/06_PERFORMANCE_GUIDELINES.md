# 06_PERFORMANCE_GUIDELINES.md

# FLACHEAD UI — Performance Guidelines

## 1. Purpose

This document defines the performance rules for FLACHEAD.

FLACHEAD targets embedded hardware:

* Raspberry Pi Zero W
* ARM11 CPU
* 512MB RAM
* small TFT display
* offline operation

Performance is a core design requirement, not an optimization added later.

The system must remain:

* responsive
* stable
* predictable
* efficient

Primary principle:

> A feature that looks good but makes the device slow does not belong in FLACHEAD.

---

# 2. Performance philosophy

FLACHEAD is not competing with smartphones or desktop applications.

The goal is:

* instant interaction
* smooth music playback
* reliable long sessions
* low power usage

Priority order:

```text id="g8p3m1"
1. Audio stability

        ↓

2. User interaction speed

        ↓

3. UI smoothness

        ↓

4. Visual effects
```

---

# 3. Hardware constraints

The design must consider:

## CPU

Raspberry Pi Zero W:

* ARM11 single-core CPU
* limited processing power

Avoid:

* heavy background computation
* unnecessary threads
* expensive algorithms

---

## RAM

Available:

512MB

Avoid:

* large caches
* duplicate assets
* memory leaks
* uncontrolled artwork storage

---

## Storage

SD card limitations:

* slower random access
* limited write cycles

Avoid:

* constant writes
* unnecessary logging
* repeated database updates

---

# 4. Performance targets

## UI response

Target:

```text id="p4m8k2"
Touch response

<100ms
```

---

## Hardware buttons

Target:

```text id="r9x3m7"
Button response

<50ms
```

---

## Rendering

Minimum:

```text id="w5k1q8"
30 FPS
```

Preferred:

```text id="m3p7x9"
45-60 FPS
```

---

## Boot

Targets:

```text id="z6n2v4"
First visual output

<3 seconds


Usable UI

<10 seconds
```

---

# 5. CPU optimization rules

## Avoid work in render loop

Bad:

```text id="t8m5q1"
Every frame:

Load image

Calculate colors

Search database
```

---

Good:

```text id="c7x2m9"
Prepare data

↓

Cache result

↓

Render
```

---

# 6. Memory management

Memory should be predictable.

Rules:

* reuse objects
* free unused resources
* avoid unnecessary copies
* cache intelligently

---

Avoid:

```text id="n2p7x5"
Create temporary objects

Every frame
```

---

Prefer:

```text id="h6m3q8"
Object pools

Reusable buffers

Static allocation
```

---

# 7. Asset optimization

## Images

Requirements:

* resize before loading
* cache processed versions
* avoid oversized artwork

Example:

Bad:

```text id="x9m4p2"
4000x4000 album art

directly loaded
```

---

Good:

```text id="k5q8m3"
4000x4000

↓

256x256 cached version
```

---

## Fonts

Optimize:

* load only required sizes
* cache rendered text

Avoid:

* many font families
* unnecessary weights

---

# 8. Rendering optimization

Required:

* dirty rectangles
* layer caching
* partial updates

Avoid:

* full screen redraws
* expensive transparency
* blur effects

---

Rendering priority:

```text id="a8m4x6"
Playback Controls

>

Text

>

Navigation

>

Animations
```

---

# 9. Animation rules

Animations must enhance usability.

Allowed:

* fade
* slide
* progress movement

Duration:

100-300ms

---

Avoid:

* continuous animations
* background effects
* decorative motion

---

Example:

Good:

```text id="u7n3m9"
Track transition fade
```

Bad:

```text id="q2x8p5"
Animated particles behind album art
```

---

# 10. Background tasks

Background services must have priorities.

## High priority

* audio decoding
* playback buffering

---

## Medium priority

* library operations
* metadata extraction

---

## Low priority

* artwork processing
* cache cleanup
* statistics

---

# 11. Threading rules

Pi Zero W has limited CPU resources.

Avoid excessive threads.

Recommended:

```text id="v5m8k1"
Main Thread

UI

Input


Audio Thread

Playback


Worker Thread

Database

Metadata
```

---

# 12. Database optimization

Library database should:

* use indexes
* avoid unnecessary queries
* cache common searches

---

Example:

Bad:

```text id="p1x7m4"
Search entire library

every screen open
```

---

Good:

```text id="j8k3n6"
Load cached library state
```

---

# 13. Startup optimization

Boot should prioritize usability.

Start first:

```text id="x4m7p2"
Display

Input

Audio

UI
```

---

Delay:

```text id="w8n5q3"
Artwork indexing

Library analysis

Cache rebuilding
```

---

# 14. Power optimization

Lower power usage by:

* reducing unnecessary refreshes
* sleeping display when unused
* lowering brightness
* avoiding idle animations

---

Possible:

```text id="s5k9m2"
No interaction

↓

Reduce refresh rate

↓

Wake on input
```

---

# 15. Audio priority rules

Audio always wins.

If resources conflict:

```text id="m6q2x8"
Keep playback stable

↓

Reduce visual quality
```

---

Examples:

Allowed:

* reduce animation FPS
* disable effects

Not allowed:

* interrupt playback

---

# 16. Adaptive quality system

Recommended.

Example:

```text id="c3p8m7"
Performance Monitor

        |

Detect Load

        |

Adjust Quality
```

---

Possible reductions:

Level 1:

Disable unnecessary animations

Level 2:

Reduce refresh rate

Level 3:

Simplify artwork rendering

---

# 17. Logging rules

Logging must be controlled.

Production:

* errors
* warnings

Debug mode:

* performance data
* diagnostics

Avoid:

```text id="z8q4m1"
Writing logs constantly
```

---

# 18. Monitoring

Useful metrics:

* FPS
* frame time
* memory usage
* CPU usage
* audio buffer health

---

Example:

```text id="k7m2x9"
Performance State

FPS: 45

Frame: 22ms

RAM: 120MB

Audio Buffer: Stable
```

---

# 19. Features to avoid

Avoid unless hardware improves:

## Heavy visual effects

* blur
* real-time shaders
* particles

---

## Large frameworks

Avoid:

* browser-based UI
* heavyweight GUI frameworks

---

## Constant network features

FLACHEAD is offline-first.

---

# 20. Error handling

Performance failures should degrade gracefully.

Example:

```text id="n6x3p9"
High CPU Usage

↓

Disable animations

↓

Maintain playback
```

---

# 21. Acceptance criteria

Implementation is complete when:

* UI remains responsive after hours of use
* playback never stutters due to UI
* memory remains stable
* rendering targets are achieved
* startup is fast
* background tasks do not interrupt usage
* SD card writes are controlled

---

# 22. Future improvements

Possible additions:

* hardware acceleration
* performance profiler
* automatic quality scaling
* memory diagnostics
* battery optimization profiles

---

# 23. Architectural recommendations

## Performance Monitor

Create:

```text id="f3m7q8"
Performance Monitor

├── measureFPS()
├── trackMemory()
├── monitorCPU()
├── detectOverload()
└── adjustQuality()
```

---

## Resource Manager

Create:

```text id="p9x4m6"
Resource Manager

├── loadAsset()
├── cacheAsset()
├── releaseAsset()
├── monitorUsage()
└── cleanup()
```

---

# 24. Implementation notes for AI coding agent

When implementing:

* Optimize for Raspberry Pi Zero W first.
* Never sacrifice audio stability.
* Avoid unnecessary complexity.
* Profile before adding features.
* Keep memory usage predictable.
* Prefer cached data over repeated computation.
* Reduce visual quality before reducing responsiveness.

The final FLACHEAD experience should feel fast and intentional: every pixel, frame, and CPU cycle has a purpose.
