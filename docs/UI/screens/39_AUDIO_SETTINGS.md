# 39_AUDIO_SETTINGS.md

# FLACHEAD UI — Audio Settings Screen Specification

## 1. Purpose

The Audio Settings Screen provides control over FLACHEAD's audio playback configuration.

This screen is one of the most important system interfaces because FLACHEAD is designed around high-quality offline FLAC playback and external DSP hardware.

It manages:

* output device information
* DSP connection status
* playback quality settings
* volume behavior
* audio processing options

Primary principle:

> Audio settings should expose useful control without compromising the simplicity of a dedicated music player.

---

# 2. Design goals

The Audio Settings Screen must be:

* technically informative
* simple to navigate
* safe for inexperienced users
* optimized for TANCHJIM BUNNY DSP
* lightweight on Raspberry Pi Zero W

---

# 3. Dependencies

Required systems:

* Playback Engine
* Audio Backend
* DSP Manager
* Media Session Manager
* Configuration Service
* Hardware Detection Service
* Theme Engine
* Screen Manager

---

# 4. Architecture

Data flow:

```text id="m8x4q7"
User Setting Change

↓

Configuration Service

↓

Audio Backend

↓

DSP / Output Device

↓

Playback Engine
```

---

The UI does not directly control:

* ALSA
* DSP drivers
* hardware devices

All changes go through audio services.

---

# 5. Screen layout

Portrait layout:

```text id="q5m8x9"
+----------------------+
| < Back               |
|                      |
| Audio Settings       |
|                      |
| Output Device        |
| TANCHJIM BUNNY DSP   |
|                      |
| Playback Quality     |
| FLAC                 |
|                      |
| Volume               |
| 70%                  |
|                      |
| Processing           |
| Equalizer            |
| ReplayGain           |
|                      |
| Mini Player          |
+----------------------+
```

---

# 6. Output device section

Displays:

* active output
* connection state
* capabilities

Example:

```text id="v7m3x8"
Output

TANCHJIM BUNNY DSP

Connected

USB Audio
```

---

States:

## Connected

Normal operation.

---

## Disconnected

Show:

```text id="x8m4p2"
DSP Disconnected
```

---

## Searching

Show:

```text id="r6m2x9"
Detecting Audio Device
```

---

# 7. Audio quality information

Displays:

## Current format

Example:

```text id="a5m8x3"
FLAC

24-bit

96 kHz

Stereo
```

---

## Output format

Future support:

* bit depth
* sample rate
* exclusive mode

---

# 8. Volume settings

Controls:

* master volume
* hardware/software volume mode

---

Example:

```text id="n4m7x5"
Volume Mode

Hardware

Software
```

---

Recommended:

Prefer hardware volume when supported.

---

# 9. ReplayGain

Purpose:

Normalize loudness between tracks.

Options:

```text id="c8m3x7"
Off

Track Gain

Album Gain
```

---

Default:

Off.

Reason:

Audiophile users may prefer original mastering.

---

# 10. Equalizer

Future feature.

Architecture support:

```text id="w5m8x2"
EQ Manager

↓

DSP Pipeline

↓

Audio Output
```

---

Initial implementation:

Placeholder only.

---

# 11. Audio processing

Possible options:

* DSP profiles
* filters
* output modes

Example:

```text id="p7m4x9"
Processing

Direct

DSP Mode

Custom
```

---

Avoid exposing advanced controls too early.

---

# 12. Physical control support

UP/DOWN:

Navigate settings.

SELECT:

Modify value.

BACK:

Return.

PLAY:

Toggle playback.

HOME:

Launcher.

---

# 13. Confirmation behavior

Safe changes:

Apply immediately.

Example:

* volume
* replay gain

---

Risky changes:

Require confirmation.

Example:

* switching output device
* resetting DSP settings

---

# 14. Theme behavior

Audio settings follow:

```text id="b6m9x3"
System Theme
```

---

When opened from player:

May inherit:

```text id="k8m3x5"
Album Accent Colors
```

---

# 15. Animations

Allowed:

* toggle transitions
* device connection animation

Duration:

100-200ms

---

Avoid:

* audio visualizers by default
* constant animations

Reason:

CPU savings.

---

# 16. Performance requirements

Targets:

Opening:

<200ms

Setting changes:

Immediate feedback

Memory:

minimal

---

Audio settings must never:

* interrupt playback unnecessarily
* freeze UI
* block rendering

---

# 17. Error handling

DSP unavailable:

Display:

```text id="m5x8q4"
Audio device unavailable
```

---

Unsupported format:

Display:

```text id="q9m3x7"
Unsupported output format
```

---

Backend failure:

Attempt recovery.

---

# 18. Acceptance criteria

Implementation is complete when:

* output device status works
* settings persist
* DSP information displays
* playback remains stable
* hardware controls work

---

# 19. Architectural recommendations

Required services:

```text id="z7m4x8"
Audio Settings Manager

├── Read Config

├── Update Config

├── Validate Settings

└── Apply Changes
```

---

DSP service:

```text id="h5m8x2"
DSP Manager

├── Detect Device

├── Query Capabilities

├── Configure Output

└── Monitor Status
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Keep audio settings separate from playback engine.
* Never restart audio unnecessarily.
* Apply settings asynchronously.
* Treat DSP detection as a hardware service.
* Preserve gapless playback where possible.
* Avoid adding smartphone-style audio features.

FLACHEAD should expose enough control to satisfy audio enthusiasts while remaining a simple dedicated music player.
