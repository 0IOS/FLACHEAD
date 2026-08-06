# 27_SHUTDOWN.md

# FLACHEAD UI — Shutdown System

## 1. Purpose

The Shutdown System defines how FLACHEAD safely powers down while preserving user data, playback state, and hardware stability.

Unlike a normal computer, FLACHEAD is designed as a dedicated music device. Shutdown should be:

* predictable
* fast
* safe
* visually clear

The user should always know that the device is powering off and not frozen.

Primary goal:

> Safely stop all services while protecting the music library and user state.

---

# 2. Design goals

The shutdown system must:

* Prevent filesystem corruption.
* Save playback state.
* Save user preferences.
* Stop audio output safely.
* Handle hardware power removal gracefully.
* Provide clear feedback.
* Complete quickly on Raspberry Pi Zero W.

The system must avoid:

* long shutdown screens
* unnecessary animations
* forced waiting
* data loss

---

# 3. Shutdown architecture

Shutdown is controlled by a dedicated system manager.

Architecture:

```text id="g8y5st"
Shutdown Request
        |
        v
System Manager
        |
        +----------------+
                         |
                         v

              Shutdown Coordinator

        |
        +-------------+-------------+
        |             |             |
        v             v             v

 Playback       Storage        UI
 Manager        Manager        Manager

        |
        v

 Linux Power Off
```

---

# 4. Dependencies

The Shutdown System depends on:

* System Manager
* Playback Engine
* Queue Manager
* Configuration Service
* Library Database
* Storage Manager
* Display Service
* Power Management Service

---

# 5. Shutdown triggers

Shutdown can be triggered by:

## 5.1 User action

Examples:

* Settings → Power Off
* Shutdown dialog confirmation
* Hardware power button

---

## 5.2 Battery protection

Future support:

* critically low battery
* emergency save

---

## 5.3 System failure recovery

Examples:

* unrecoverable hardware failure
* filesystem protection

---

# 6. Shutdown sequence

Recommended order:

```text id="x9rx6c"
Shutdown requested

        ↓

Show shutdown overlay

        ↓

Stop accepting new actions

        ↓

Pause playback

        ↓

Save playback state

        ↓

Save settings

        ↓

Close database

        ↓

Unmount storage

        ↓

Stop services

        ↓

Power off
```

---

# 7. Shutdown screen layout

The shutdown screen should be simple.

Preferred:

```text id="6ghs6p"
┌─────────────────────┐
│                     │
│                     │
│      FLACHEAD       │
│                     │
│                     │
│  Shutting down...   │
│                     │
└─────────────────────┘
```

---

# 8. Shutdown states

The UI should represent real system states.

## Preparing

```text
Saving playback state...
```

---

## Saving

```text
Saving settings...
```

---

## Stopping

```text
Closing services...
```

---

## Complete

```text
Powering off...
```

---

Avoid fake progress.

Only display meaningful steps.

---

# 9. Playback shutdown behavior

Before stopping audio:

The Playback Engine should:

1. Stop accepting new commands.
2. Finish current buffer safely.
3. Stop audio stream.
4. Save current position.

Saved information:

```text id="p0m8r7"
Playback State

track_id

position_ms

queue_id

repeat_mode

shuffle_state

volume_level
```

---

# 10. TANCHJIM BUNNY DSP handling

The DSP should be disconnected safely.

Shutdown order:

```text id="f6nqzv"
Playback Engine

↓

Audio Backend

↓

USB Audio Device

↓

Power Off
```

Avoid:

* sudden USB power removal during active stream
* leaving audio subsystem locked

---

# 11. Storage protection

The Storage Manager must:

* flush pending writes
* close database connections
* sync filesystem
* unmount storage

Important for SD card reliability.

---

Recommended:

```text id="2m0j5p"
Database

↓

Filesystem Sync

↓

Unmount

↓

Power Off
```

---

# 12. User interaction during shutdown

Once shutdown begins:

Input should be locked.

Allowed:

* no additional actions

Not allowed:

* opening screens
* changing settings
* starting playback

Reason:

Prevents inconsistent system state.

---

# 13. Shutdown animation

Allowed:

* fade out
* logo fade

Duration:

<500ms

Not allowed:

* complex animations
* loading loops
* unnecessary transitions

---

# 14. Shutdown performance targets

Target:

Full shutdown:

<10 seconds

Ideal:

5 seconds

Optimization:

* save only changed settings
* avoid unnecessary database writes
* close services asynchronously where safe

---

# 15. Shutdown errors

## Database save failure

Behavior:

* retry once
* continue shutdown
* log issue

---

## Storage unmount failure

Behavior:

* attempt sync
* force cleanup
* warn next boot if needed

---

## Playback state save failure

Behavior:

* continue shutdown
* use last known state

---

## Service timeout

Behavior:

* terminate safely
* continue shutdown

A shutdown failure should never leave the device permanently stuck.

---

# 16. Emergency shutdown

For:

* power button hold
* critical battery

Emergency procedure:

```text id="p8v7cg"
Stop playback

↓

Save minimum state

↓

Sync storage

↓

Power off
```

Prioritize data protection over perfect cleanup.

---

# 17. Theme integration

Shutdown screen uses system theme.

Preferred:

* wallpaper-derived colors
* simple logo
* minimal brightness

Avoid:

* album-art themes
* application themes

Shutdown belongs to the operating system layer.

---

# 18. Acceptance criteria

Implementation is complete when:

* shutdown safely saves user state
* playback stops correctly
* storage is protected
* UI clearly shows shutdown progress
* inputs are locked during shutdown
* TANCHJIM BUNNY DSP disconnects safely
* shutdown completes reliably on Raspberry Pi Zero W
* failed services do not permanently block power off

---

# 19. Future improvements

Possible additions:

* instant sleep mode
* suspend/resume
* shutdown scheduling
* battery-aware shutdown
* fast boot state snapshots
* safe power controller integration

---

# 20. Architectural recommendations

## Shutdown Manager

Create:

```text id="dsvjpm"
Shutdown Manager

├── requestShutdown()
├── saveState()
├── stopPlayback()
├── stopServices()
├── syncStorage()
├── powerOff()
└── handleFailure()
```

---

## Persistent State Manager

Centralize:

```text id="6w8h8h"
Persistent State

├── playback_state
├── queue_state
├── user_settings
├── theme_state
└── system_preferences
```

---

## Event flow

```text id="f2j0m0"
Shutdown Request

↓

Shutdown Manager

↓

Event Bus

↓

Services

↓

Completion Events

↓

Power Off
```

---

# 21. Implementation notes for AI coding agent

When implementing:

* Treat shutdown as a system workflow, not a UI screen.
* Prioritize filesystem safety.
* Save state before power removal.
* Keep shutdown fast.
* Avoid blocking the UI thread.
* Separate system services from visual representation.

The final experience should feel like a dedicated hardware music player shutting down safely, not a desktop computer closing applications.
