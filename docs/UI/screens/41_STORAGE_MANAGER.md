# 41_STORAGE_MANAGER.md

# FLACHEAD UI — Storage Manager Screen Specification

## 1. Purpose

The Storage Manager provides control and information about FLACHEAD's local music storage.

Since FLACHEAD is an offline-first music player using removable storage, storage management is a core system feature.

It allows users to:

* view storage status
* monitor SD card usage
* manage music availability
* safely handle storage changes

Primary principle:

> Storage should be reliable and transparent without feeling like a computer file manager.

---

# 2. Design goals

The Storage Manager must be:

* simple
* safe
* offline-focused
* optimized for SD card usage
* lightweight on Raspberry Pi Zero W

---

# 3. Dependencies

Required systems:

* Storage Manager Service
* Library Database
* Library Scanner
* File System Manager
* Configuration Service
* Notification Manager
* Screen Manager
* Theme Engine

---

# 4. Architecture

Data flow:

```text id="x6m4q8"
Storage Event

↓

Storage Manager

↓

Library System

↓

UI Update

↓

Notification
```

---

Storage Manager handles:

* mount state
* storage information
* safe operations

It does not:

* directly modify music metadata
* decode files
* control playback

---

# 5. Screen layout

Portrait layout:

```text id="q8m3x5"
+----------------------+
| < Back               |
|                      |
| Storage Manager      |
|                      |
| SD Card              |
| Connected            |
|                      |
| Used                 |
| ██████░░░ 64%        |
|                      |
| Music Library        |
| 542 Tracks           |
|                      |
| Actions              |
| Scan Library         |
| Eject Storage        |
|                      |
| Mini Player          |
+----------------------+
```

---

# 6. Storage information

Displays:

* device name
* mount status
* total capacity
* used space
* free space

Example:

```text id="m7x3q9"
Music SD Card

64GB

Used:
32GB

Free:
32GB
```

---

# 7. Storage states

## Connected

Normal operation.

---

## Not detected

Display:

```text id="p5m8x2"
No storage detected
```

---

## Mounting

Display:

```text id="r8m4x6"
Preparing storage
```

---

## Error

Display:

```text id="v6m3x7"
Storage error
```

---

# 8. Music storage structure

Recommended:

```text id="w4m8x2"
SD Card

└── Music

    ├── Artist

    │   └── Album

    │       └── Track.flac
```

---

FLACHEAD should not require this exact structure.

Metadata determines organization.

---

# 9. Library relationship

Storage Manager connects with:

```text id="c7m4x9"
Storage

↓

Library Scanner

↓

Database

↓

Music UI
```

---

Storage changes should trigger:

* scan recommendation
* library update

---

# 10. Safe eject

Before ejecting:

Check:

* playback state
* active scans
* file operations

---

If music is playing:

Option:

```text id="n8m3x5"
Stop playback and eject?
```

---

Never remove storage during active access.

---

# 11. Storage actions

Available:

## Scan Library

Opens:

```text id="k5m8x3"
Library Scan Screen
```

---

## Eject Storage

Safely unmounts.

---

## Refresh Status

Reloads storage information.

---

# 12. Physical control support

UP/DOWN:

Navigate options.

SELECT:

Open action.

BACK:

Return.

HOME:

Launcher.

---

# 13. Touch behavior

Tap:

Select option.

Long press:

Future advanced actions.

---

# 14. Theme behavior

Storage Manager uses:

```text id="b6m3x8"
System Theme
```

---

No album-based coloring.

Reason:

System utility.

---

# 15. Animations

Allowed:

* progress updates
* mount status changes
* scan indicators

Duration:

100-200ms

---

Avoid:

* animated storage graphs
* unnecessary effects

---

# 16. Performance requirements

Targets:

Opening:

<200ms

Storage information:

asynchronous

Scanning:

background operation

---

The UI must remain responsive during:

* library scans
* metadata processing
* storage checks

---

# 17. Error handling

Storage disconnected:

Show:

```text id="u7m4x5"
Music storage removed
```

---

Corrupted filesystem:

Show:

```text id="z8m3x6"
Storage requires repair
```

---

Permission issue:

Show recovery message.

---

# 18. Acceptance criteria

Implementation is complete when:

* storage status displays
* SD card changes are detected
* safe eject works
* library updates correctly
* playback remains stable

---

# 19. Architectural recommendations

Required service:

```text id="s4m8x7"
Storage Manager

├── Detect Storage

├── Mount State

├── Usage Information

├── Safe Eject

└── Notify Changes
```

---

Required components:

```text id="d6m3x9"
StorageCard

ProgressBar

StatusIndicator

ActionRow
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Treat SD card removal as a normal event.
* Never crash if storage disappears.
* Keep storage logic separate from UI.
* Use notifications for background operations.
* Avoid constant filesystem scanning.

FLACHEAD should feel like a dedicated music player that understands removable storage, not a desktop file manager.
