# 26_BOOT.md

# FLACHEAD UI — Boot Sequence

## 1. Purpose

The Boot System defines the complete startup experience of FLACHEAD from power-on to a fully usable music player interface.

Unlike a general-purpose Linux device, FLACHEAD should feel like a dedicated audio appliance.

The boot process should communicate:

* hardware initialization status
* system readiness
* audio availability
* library availability

while minimizing waiting time.

The user should not feel like they are booting a computer.

Primary goal:

> Reach playable music as quickly and reliably as possible.

---

# 2. Design goals

The boot system must:

* Optimize for Raspberry Pi Zero W limitations.
* Provide immediate visual feedback.
* Avoid unnecessary animations.
* Initialize services in parallel where possible.
* Allow the UI to appear before all background services finish.
* Handle missing hardware gracefully.
* Never leave the user with a blank screen.

---

# 3. Boot architecture

Boot should be divided into stages.

Recommended architecture:

```text
Power On
    |
    v
Bootloader
    |
    v
Linux Kernel
    |
    v
FLACHEAD Service
    |
    +----------------+
                     |
                     v
              System Initialization
                     |
        +------------+-------------+
        |            |             |
        v            v             v

 Display       Audio          Storage
 Init          Init           Init

        |
        v

UI Shell Start

        |
        v

Home Screen Ready

        |
        v

Background Services
```

---

# 4. Boot stages

## Stage 0 — Hardware power-on

Responsibilities:

* Raspberry Pi firmware startup
* Linux kernel loading
* hardware detection

No FLACHEAD UI yet.

---

## Stage 1 — FLACHEAD initialization

The FLACHEAD service starts.

Responsibilities:

Initialize:

* configuration service
* logging
* system state
* theme engine
* display manager

Display:

```text
FLACHEAD logo
```

---

## Stage 2 — Display initialization

Initialize:

* TFT display
* framebuffer
* touch controller
* backlight control

The screen should become active as early as possible.

Avoid:

* waiting for every subsystem before showing UI

---

## Stage 3 — Core services

Initialize:

Required:

* Playback Engine
* Media Session Manager
* Queue Manager
* Library Database
* Metadata Service
* Lyrics Engine

Optional:

* theme indexing
* artwork cache
* background scanning

---

## Stage 4 — User interface launch

Start:

* home screen
* mini player state
* system overlays
* input manager

At this point:

The device is usable.

---

## Stage 5 — Background initialization

After UI becomes available:

Continue:

* library indexing
* artwork generation
* metadata cleanup
* cache rebuilding

The user should not wait for these.

---

# 5. Boot screen layout

The boot screen should remain simple.

Preferred:

```text
┌─────────────────────┐
│                     │
│                     │
│      FLACHEAD       │
│                     │
│       ◉             │
│                     │
│   Starting...       │
│                     │
└─────────────────────┘
```

---

# 6. Boot screen elements

## Logo

Requirements:

* static image
* low resolution
* cached asset

Avoid:

* large animations
* video splash screens

---

## Status text

Examples:

```text
Starting FLACHEAD...

Loading audio system...

Preparing library...

Ready
```

Text should be short.

---

## Progress indicator

Optional.

Recommended:

Small horizontal progress bar.

Avoid:

* percentage obsession
* fake loading progress

Only show meaningful stages.

---

# 7. Progressive rendering

Important for Pi Zero W.

The UI should appear progressively.

Bad:

```text
Wait 20 seconds

Everything loads

Show UI
```

Good:

```text
Display starts

↓

Logo appears

↓

Home screen appears

↓

Services continue loading
```

---

# 8. Boot performance targets

Target:

## First display output

<3 seconds after application start

---

## UI available

Target:

<10 seconds after power-on

---

## Playback available

Target:

<15 seconds after power-on

---

## Optimization goals

* lazy load artwork
* defer database indexing
* avoid loading unused screens
* cache fonts
* preload common assets

---

# 9. Startup service priority

Recommended priority:

## Critical

Must start before UI:

* Display service
* Input manager
* Audio manager
* Configuration service

---

## Important

Can start alongside UI:

* Library database
* Queue manager
* Theme engine

---

## Background

Start later:

* artwork cache
* lyric indexing
* maintenance tasks

---

# 10. Audio initialization

The audio chain:

```text
FLAC Library

↓

Playback Engine

↓

USB Audio Interface

↓

TANCHJIM BUNNY DSP

↓

Headphones
```

During startup:

Check:

* USB device availability
* supported formats
* audio backend status

---

If DSP unavailable:

Do not block boot.

Show:

```text
Audio device unavailable

Playback will retry
```

Continue into UI.

---

# 11. Theme initialization

Theme loading should happen asynchronously.

Startup priority:

1. Load cached theme
2. Show UI
3. Refresh theme data later

Do not delay boot waiting for:

* album artwork extraction
* wallpaper analysis
* color generation

---

# 12. Input initialization

Initialize:

* touchscreen
* physical buttons
* encoder

Input failure behavior:

Touch unavailable:

Continue with hardware controls.

Buttons unavailable:

Continue with touchscreen.

Encoder unavailable:

Continue with software volume.

---

# 13. Error states during boot

## Storage missing

Display:

```text
No music storage found

Insert SD card
```

Allow:

* settings access
* system recovery

---

## Database corrupted

Behavior:

* backup old database
* rebuild index
* continue boot

---

## Audio failure

Behavior:

* boot normally
* notify user
* retry connection

---

## Display failure

Critical.

Attempt:

* restart display service
* fallback framebuffer mode

---

# 14. Boot animation rules

Allowed:

* logo fade
* simple progress movement

Not allowed:

* 3D animations
* particles
* video
* heavy transitions

Maximum animation time:

<2 seconds total

---

# 15. Shutdown preparation interaction

Before shutdown:

The system must save:

* current track
* playback position
* queue state
* settings
* theme state

This allows fast restoration.

---

# 16. Acceptance criteria

Implementation is complete when:

* FLACHEAD shows feedback during startup
* boot does not feel like Linux startup
* UI appears before background tasks finish
* music playback becomes available quickly
* missing hardware does not crash the system
* TANCHJIM BUNNY DSP initialization is handled safely
* touch and hardware controls initialize correctly
* boot remains stable on Raspberry Pi Zero W
* startup memory usage remains controlled

---

# 17. Future improvements

Possible additions:

* instant resume mode
* sleep/wake instead of full shutdown
* boot profiles
* startup sound
* firmware update mode
* recovery partition
* safe mode

---

# 18. Architectural recommendations

## Boot Manager

Create:

```text
Boot Manager

├── initializeHardware()
├── initializeServices()
├── loadUserState()
├── startUI()
├── monitorStartup()
└── reportFailures()
```

---

## Startup state model

Example:

```text
Boot State

starting

display_ready

audio_ready

storage_ready

ui_ready

complete

failed
```

---

## System event flow

```text
Boot Manager

↓

Event Bus

↓

UI Status Overlay

↓

User Feedback
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Optimize for Pi Zero W startup time.
* Do not block UI on optional services.
* Use asynchronous initialization.
* Show useful startup information.
* Keep splash rendering extremely cheap.
* Treat FLACHEAD as an embedded audio device, not a desktop Linux application.

The final experience should feel like powering on a dedicated premium music player: quick, reliable, and ready.
