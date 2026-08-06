# 42_LIBRARY_SCAN.md

# FLACHEAD UI — Library Scan Screen Specification

## 1. Purpose

The Library Scan Screen manages the process of discovering and updating music stored on FLACHEAD's local storage.

It provides visibility into background library operations while keeping the player usable.

The system handles:

* scanning music files
* extracting metadata
* detecting artwork
* updating the database
* reporting scan progress

Primary principle:

> Library management should happen in the background without interrupting music playback.

---

# 2. Design goals

The Library Scan Screen must be:

* transparent
* non-blocking
* fast
* lightweight
* safe during long operations

---

# 3. Dependencies

Required systems:

* Library Scanner
* Metadata Service
* Library Database
* Artwork Cache
* Storage Manager
* Notification Manager
* Playback Engine
* Screen Manager
* Theme Engine

---

# 4. Architecture

Data flow:

```text id="x7m4q9"
Storage

↓

Library Scanner

↓

Metadata Service

↓

Database

↓

Artwork Cache

↓

UI Update
```

---

The scanner operates independently from the UI.

The UI only observes:

* progress
* state
* results

---

# 5. Scan types

## Full Scan

Used for:

* first setup
* rebuilding library
* corrupted database recovery

Process:

```text id="m5x8q3"
Find Files

↓

Read Metadata

↓

Generate Index

↓

Update Database
```

---

## Quick Scan

Used normally.

Checks:

* new files
* modified files
* removed files

---

## Artwork Refresh

Only updates:

* album artwork
* embedded images

---

# 6. Screen layout

Portrait layout:

```text id="q8m3x5"
+----------------------+
| < Back               |
|                      |
| Library Scan         |
|                      |
| Status               |
| Scanning Music       |
|                      |
| Progress             |
| ███████░░░ 70%       |
|                      |
| Files                |
| 1524 / 2100          |
|                      |
| Current              |
| Album Name           |
|                      |
| Mini Player          |
+----------------------+
```

---

# 7. Scan status

States:

## Idle

```text id="v4m8x2"
Library Ready
```

---

## Scanning

```text id="p6m3x9"
Scanning Music
```

---

## Processing

```text id="c7m5x8"
Processing Metadata
```

---

## Complete

```text id="r9m4x2"
Library Updated
```

---

## Failed

```text id="n5m8x6"
Scan Failed
```

---

# 8. Progress information

Display:

* current operation
* files discovered
* files processed
* estimated progress

Example:

```text id="w6m3x7"
Processing:

Dark Side Of The Moon.flac

1240 / 3000
```

---

Avoid showing inaccurate time estimates.

---

# 9. Background behavior

During scanning:

Allowed:

* browse library
* play existing music
* adjust settings

---

Restricted:

* eject storage
* modify database manually

---

# 10. Playback interaction

Important:

Scanning must not interrupt playback.

Architecture:

```text id="k8m4x3"
Playback Engine

Independent

Library Scanner
```

---

CPU usage should be controlled.

---

# 11. Scan controls

Available:

## Pause Scan

Temporarily stops processing.

---

## Cancel Scan

Stops safely.

Database remains valid.

---

## Start Again

Restarts operation.

---

# 12. Physical control support

UP/DOWN:

Navigate actions.

SELECT:

Confirm.

BACK:

Exit screen.

---

If scan is active:

BACK does not cancel immediately.

Shows:

```text id="a5m8x7"
Cancel scan?
```

---

# 13. Notifications

After completion:

Example:

```text id="b6m3x8"
Library updated

542 new tracks
```

---

Errors:

```text id="u7m4x5"
12 files could not be processed
```

---

# 14. Theme behavior

Library Scan uses:

```text id="s4m8x2"
System Theme
```

---

Reason:

System operation.

---

# 15. Animations

Allowed:

* progress movement
* status transitions
* completion feedback

Duration:

100-200ms

---

Avoid:

* animated scanning graphics
* constant visualizers

---

# 16. Performance requirements

Targets:

UI responsiveness:

60 FPS preferred

Scanner:

background thread

Memory:

controlled allocation

---

Optimization:

Use:

* batch database writes
* incremental indexing
* cached metadata

---

# 17. Error handling

Corrupt audio file:

```text id="z8m3x6"
Skipped file
```

---

Unreadable storage:

```text id="h5m7x4"
Storage unavailable
```

---

Metadata failure:

Use fallback values.

---

# 18. Acceptance criteria

Implementation is complete when:

* scans complete successfully
* progress is visible
* playback continues
* errors are handled
* database remains consistent

---

# 19. Architectural recommendations

Required service:

```text id="d8m4x6"
Library Scanner

├── File Discovery

├── Metadata Extraction

├── Artwork Processing

├── Database Update

└── Progress Reporting
```

---

Required components:

```text id="e5m8x3"
ProgressBar

StatusCard

ScanStatistics

ActionButton
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Never scan on the UI thread.
* Make scanning resumable.
* Use incremental updates.
* Protect database integrity.
* Limit CPU usage on Raspberry Pi Zero W.
* Keep playback priority above scanning.

The Library Scan system should make adding music to FLACHEAD feel effortless while preserving the responsiveness expected from a dedicated audio player.
