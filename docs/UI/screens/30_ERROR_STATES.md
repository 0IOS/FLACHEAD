# 30_ERROR_STATES.md

# FLACHEAD UI — Error States & Recovery System

## 1. Purpose

The Error State System defines how FLACHEAD handles failures while maintaining stability, recoverability, and a usable music experience.

FLACHEAD runs on Raspberry Pi Zero W hardware with limited resources, external storage, USB audio hardware, and custom services. Failures must be expected and handled gracefully.

The system should avoid:

* crashes
* frozen screens
* unexplained failures
* forced reboots

Primary goal:

> When something goes wrong, FLACHEAD should explain the problem, recover when possible, and keep the user in control.

---

# 2. Design goals

The Error System must:

* Separate recoverable and critical failures.
* Preserve playback state whenever possible.
* Avoid overwhelming the user.
* Provide useful information.
* Support offline recovery.
* Work with touchscreen and hardware controls.
* Protect user data.

Error messages should be:

* short
* actionable
* understandable

Avoid technical Linux errors unless shown in advanced diagnostics.

---

# 3. Error architecture

Errors should not be handled directly by individual screens.

Architecture:

```text
Subsystem Failure

        |

        v

Error Manager

        |

        +----------------+
                         |
                         v

              Error Classification

        |
        +-------------+-------------+
        |             |             |
        v             v             v

 Notification     Dialog       Recovery Mode

        |
        v

 User Action
```

---

# 4. Dependencies

The Error System depends on:

* Error Manager
* Notification Service
* Dialog System
* Logging Service
* Playback Engine
* Storage Manager
* Audio Manager
* Configuration Service
* Recovery Manager

---

# 5. Error severity levels

## 5.1 Informational

Minor issue.

Examples:

* lyrics unavailable
* artwork missing
* optional metadata failed

Behavior:

* notification only

Example:

```text
Lyrics unavailable
```

---

## 5.2 Warning

User should know.

Examples:

* storage almost full
* scan incomplete
* temporary audio disconnect

Behavior:

* notification
* optional action

Example:

```text
Storage space low

Manage storage?
```

---

## 5.3 Error

Functionality affected.

Examples:

* track cannot play
* database corrupted
* DSP unavailable

Behavior:

* dialog or recovery action

---

## 5.4 Critical

System operation compromised.

Examples:

* filesystem failure
* configuration corruption
* display failure

Behavior:

* recovery workflow
* safe mode if needed

---

# 6. Error display hierarchy

Priority:

```text
Critical Recovery Screen

        |

Error Dialog

        |

Warning Notification

        |

Information Notification
```

Rules:

* Critical errors override normal UI.
* Warnings never interrupt playback unnecessarily.
* Information errors remain silent where possible.

---

# 7. General error layout

## Dialog error

```text
┌─────────────────────┐
│                     │
│   Playback Error    │
│                     │
│ Unable to decode    │
│ this track.         │
│                     │
│ [SKIP]   [RETRY]    │
│                     │
└─────────────────────┘
```

---

## Recovery screen

```text
┌─────────────────────┐
│                     │
│   FLACHEAD Error    │
│                     │
│ System needs repair │
│                     │
│ [RECOVER]           │
│                     │
└─────────────────────┘
```

---

# 8. Playback errors

## 8.1 Unsupported file

Example:

```text
Cannot play track

Unsupported format
```

Actions:

* skip track
* remove from queue
* view details

---

## 8.2 Corrupted FLAC file

Behavior:

* stop current decode
* preserve queue
* move to next track if enabled

Display:

```text
Track corrupted

Skipping...
```

---

## 8.3 Decoder failure

Recovery:

1. restart decoder
2. retry track
3. offer skip

---

# 9. TANCHJIM BUNNY DSP errors

The DSP is a critical part of the audio chain.

## Device disconnected

Example:

```text
Audio Device Lost

TANCHJIM BUNNY DSP disconnected
```

Actions:

* reconnect
* retry playback
* continue without audio

---

## Initialization failure

Behavior:

* retry connection
* keep UI available
* log details

Never:

* freeze boot
* crash playback engine

---

# 10. Storage errors

## No SD card

Display:

```text
Music Storage Missing

Insert storage card
```

Available actions:

* settings
* retry detection

---

## Storage read failure

Behavior:

* stop affected operation
* protect database
* notify user

---

## Low storage

Warning:

```text
Storage Almost Full

500 MB remaining
```

Actions:

* open storage settings
* ignore

---

# 11. Library database errors

## Database unavailable

Recovery:

1. attempt repair
2. create backup
3. rebuild index

User message:

```text
Library needs rebuilding

Continue?
```

---

## Scan failure

Do not discard existing library.

Keep:

* previous database
* last working state

---

# 12. Configuration errors

## Invalid settings

Example:

```text
Settings reset required
```

Recovery:

* restore defaults
* preserve music library

---

## Configuration corruption

Process:

```text
Detect corruption

↓

Backup old config

↓

Create clean config

↓

Notify user
```

---

# 13. Display errors

## Touch failure

Fallback:

* hardware controls

Notification:

```text
Touch unavailable

Using buttons
```

---

## TFT initialization failure

Attempt:

* restart display service
* fallback rendering mode

Critical only if no visual output exists.

---

# 14. Input errors

## Button failure

Continue:

* touchscreen controls

---

## Encoder failure

Continue:

* touchscreen volume control

---

## Complete input failure

Enter:

Recovery mode

Allow:

* software repair
* reboot

---

# 15. Recovery Mode

Recovery Mode is a minimal FLACHEAD environment.

Purpose:

* repair configuration
* rebuild database
* diagnose hardware
* recover from failed updates

Layout:

```text
┌─────────────────────┐
│ FLACHEAD Recovery   │
│                     │
│ 1. Repair Library   │
│ 2. Reset Settings   │
│ 3. Logs             │
│ 4. Restart          │
└─────────────────────┘
```

---

# 16. Logging

Errors should create structured logs.

Example:

```text
Error Event

timestamp

module

severity

description

recovery_attempt

result
```

Logs should be:

* local
* lightweight
* rotated

Avoid filling SD storage.

---

# 17. User messaging rules

Bad:

```text
ALSA error -32 USB endpoint failure
```

Good:

```text
Audio device unavailable

Reconnect headphones and retry.
```

Technical details belong in logs.

---

# 18. Performance considerations

Error handling must not create additional instability.

Requirements:

* no heavy diagnostics by default
* no memory-heavy reports
* no blocking operations in UI thread

Recovery tasks should run asynchronously.

---

# 19. Acceptance criteria

Implementation is complete when:

* errors are classified correctly
* playback failures recover safely
* DSP failures do not crash the system
* storage problems protect user data
* configuration failures recover automatically
* users receive understandable messages
* logs are created
* recovery mode is available
* Pi Zero W remains responsive

---

# 20. Future improvements

Possible additions:

* automatic crash recovery
* remote diagnostics export
* health dashboard
* SMART storage monitoring
* hardware self-test
* update rollback system

---

# 21. Architectural recommendations

## Error Manager

Create:

```text
Error Manager

├── reportError()
├── classify()
├── notify()
├── recover()
├── log()
└── escalate()
```

---

## Recovery Manager

Create:

```text
Recovery Manager

├── repairDatabase()
├── resetConfig()
├── restoreBackup()
├── safeRestart()
└── enterRecoveryMode()
```

---

## Event flow

```text
Subsystem

↓

Error Event Bus

↓

Error Manager

↓

Notification/Dialog/Recovery

↓

User Action

↓

Recovery Handler
```

---

# 22. Implementation notes for AI coding agent

When implementing:

* Never allow errors to crash the entire UI.
* Keep recovery logic separate from screens.
* Prefer automatic recovery over user intervention.
* Preserve playback and user data whenever possible.
* Treat TANCHJIM BUNNY DSP, storage, and library database failures as expected conditions.
* Keep error handling lightweight for Raspberry Pi Zero W.

The final system should feel like a mature embedded audio device: failures are handled calmly, clearly, and safely.
