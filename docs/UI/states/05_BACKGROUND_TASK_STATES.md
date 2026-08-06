# 05_BACKGROUND_TASK_STATES.md

# FLACHEAD UI — Background Task States Specification

## 1. Purpose

This document defines how FLACHEAD represents and manages background operations.

FLACHEAD performs several tasks that may take time:

* music library scanning
* metadata extraction
* artwork processing
* database maintenance
* cache generation
* system preparation

These tasks must run without interrupting the main music experience.

Primary principle:

> Background work should be invisible when possible and informative when necessary.

---

# 2. Design goals

Background task states must be:

* non-blocking
* low resource usage
* transparent
* cancellable
* playback-safe

---

# 3. Background task architecture

Data flow:

```text id="x7m4q9"
Task Manager

↓

Background Worker

↓

Progress Event

↓

State Manager

↓

UI Notification
```

---

Background tasks must never:

* control rendering directly
* block the audio thread
* freeze navigation

---

# 4. Task categories

FLACHEAD background tasks:

```text id="m5x8q3"
1. Library Tasks

2. Metadata Tasks

3. Artwork Tasks

4. Database Tasks

5. Maintenance Tasks
```

---

# 5. Task visibility levels

Not every task needs UI.

Levels:

```text id="v8m3x5"
Silent

Informational

Interactive
```

---

# 6. Silent tasks

Used for:

* small cache updates
* preference saving
* minor indexing

---

No UI required.

---

Example:

```text id="c6m4x9"
Saving Settings
```

---

# 7. Informational tasks

Used when work takes noticeable time.

Examples:

* artwork generation
* metadata processing

---

UI:

Notification banner.

Example:

```text id="r7m3x8"
Updating Artwork...
```

---

# 8. Interactive tasks

Require user awareness.

Examples:

* full library scan
* database repair
* storage operations

---

UI:

Dedicated progress screen.

---

# 9. Library scanning state

States:

```text id="n8m4x6"
Idle

Preparing

Scanning

Processing

Complete

Failed

Cancelled
```

---

Example:

```text id="w5m3x7"
Scanning Music

1245 / 3000 tracks
```

---

# 10. Artwork processing state

Purpose:

Generate:

* thumbnails
* cached images
* color extraction

---

Behavior:

Low priority.

---

Reason:

Playback is more important.

---

# 11. Metadata processing state

Operations:

* read tags
* normalize information
* update database

---

Must support:

partial completion.

---

Example:

```text id="a7m4x8"
Processed:

800 / 1200 tracks
```

---

# 12. Database maintenance state

Used for:

* cleanup
* optimization
* rebuilding indexes

---

Requirements:

* safe transactions
* backup before destructive actions

---

# 13. CPU management

Important for Raspberry Pi Zero W.

Background tasks must have:

* CPU limits
* priority control
* pause capability

---

Priority:

```text id="p6m8x2"
Audio

↓

UI

↓

Input

↓

Background Tasks
```

---

# 14. Task cancellation

Long tasks support cancellation.

Example:

```text id="u5m3x9"
Cancel Scan?

Current progress will be saved.
```

---

Cancellation must:

* close files safely
* preserve database integrity
* release memory

---

# 15. Task notifications

Notification examples:

Complete:

```text id="k8m4x5"
Library Updated

42 new tracks
```

---

Failure:

```text id="d7m3x8"
Artwork Update Failed
```

---

# 16. Background task queue

Architecture:

```text id="h5m8x2"
Task Queue

├── Pending

├── Running

├── Completed

└── Failed
```

---

Only important tasks should be queued.

---

# 17. Theme behavior

Background notifications follow:

System theme.

---

Music-related tasks may use:

Album accent colors.

Example:

Artwork processing for current album.

---

# 18. Performance requirements

Background tasks must:

* never drop audio
* maintain UI responsiveness
* avoid memory spikes

---

Targets:

UI:

30-60 FPS

Input:

<50ms response

---

# 19. Error handling

Task failure:

Store:

* error type
* timestamp
* affected files

---

Do not:

* crash UI
* corrupt database

---

# 20. Acceptance criteria

Implementation is complete when:

* tasks run independently
* progress is available
* cancellation works
* playback remains stable
* failures recover safely

---

# 21. Architectural recommendations

Required service:

```text id="s6m4x9"
Task Manager

├── Queue Tasks

├── Execute Tasks

├── Report Progress

├── Cancel Tasks

└── Handle Failures
```

---

Required components:

```text id="b8m3x5"
TaskNotification

ProgressIndicator

TaskCard

BackgroundStatus
```

---

# 22. Implementation notes for AI coding agent

When implementing:

* Keep background workers separate from UI.
* Use asynchronous events.
* Never prioritize maintenance over playback.
* Limit concurrency on Raspberry Pi Zero W.
* Test with large music libraries.

A well-designed background task system is what allows FLACHEAD to feel fast even on low-power hardware.
