# 03_ERROR_RECOVERY_STATES.md

# FLACHEAD UI — Error Recovery States Specification

## 1. Purpose

This document defines how FLACHEAD detects, displays, and recovers from system failures.

Because FLACHEAD is a dedicated hardware music player running on Raspberry Pi Zero W, failures must be handled gracefully.

The system should recover from:

* audio device failures
* storage problems
* database corruption
* UI service failures
* hardware initialization issues

Primary principle:

> A failure should degrade functionality, not destroy the user experience.

---

# 2. Design goals

Error recovery states must be:

* understandable
* recoverable
* non-destructive
* lightweight
* user-focused

---

# 3. Error architecture

Data flow:

```text id="x7m4q9"
Service Failure

↓

Error Manager

↓

State Manager

↓

Recovery UI

↓

Recovery Action
```

---

The UI should never directly detect hardware failures.

All errors originate from:

* Audio Service
* Storage Manager
* Database Service
* Hardware Manager
* Renderer

---

# 4. Error severity levels

FLACHEAD uses three levels:

```text id="m5x8q3"
Level 1

Warning


Level 2

Recoverable Error


Level 3

Critical Failure
```

---

# 5. Level 1 — Warning

Purpose:

Inform user without interrupting usage.

Examples:

* DSP disconnected
* low storage
* missing artwork

---

UI:

Small notification.

Example:

```text id="v8m3x5"
DSP disconnected
Using fallback audio
```

---

No screen takeover.

---

# 6. Level 2 — Recoverable Error

Purpose:

User action required.

Examples:

* SD card unavailable
* library scan failed
* unsupported file

---

UI:

Recovery screen.

Example:

```text id="c6m4x9"
Library Scan Failed

Try Again

View Details
```

---

Available actions:

* retry
* cancel
* continue

---

# 7. Level 3 — Critical Failure

Purpose:

Protect system stability.

Examples:

* UI service crash
* corrupted configuration
* boot failure

---

UI:

Minimal recovery screen.

Example:

```text id="r7m3x8"
FLACHEAD Recovery

Restart UI

Safe Mode
```

---

# 8. Audio failure recovery

## DSP unavailable

Condition:

TANCHJIM BUNNY DSP cannot be detected.

---

Behavior:

1. Notify user.
2. Switch output mode.
3. Continue playback.

---

Example:

```text id="n8m4x6"
External DSP unavailable

Using default output
```

---

Playback should continue.

---

# 9. Storage failure recovery

## SD card removed

Behavior:

* pause library operations
* protect database
* notify user

---

Display:

```text id="w5m3x7"
Music Storage Removed

Insert card to continue
```

---

Never:

* corrupt database
* crash scanner

---

# 10. Database recovery

Condition:

Library database invalid.

---

Recovery process:

```text id="a7m4x8"
Detect Issue

↓

Backup Database

↓

Rebuild Index

↓

Restore Library
```

---

User message:

```text id="p6m8x2"
Repairing Music Library
```

---

# 11. Configuration recovery

Condition:

Invalid settings.

---

Behavior:

Restore defaults.

Affected:

* themes
* volume settings
* display options

---

Display:

```text id="u5m3x9"
Settings Reset

Safe defaults restored
```

---

# 12. Renderer failure

Condition:

UI rendering problem.

---

Recovery:

```text id="k8m4x5"
Restart Renderer

↓

Restore Screen

```

---

Playback continues.

---

# 13. Network failures

FLACHEAD is offline-first.

Therefore:

Network failure is not critical.

---

Features affected:

* updates
* optional services

---

Music playback:

Unaffected.

---

# 14. Error dialogs

Dialog structure:

```text id="d7m3x8"
Title

Explanation

Primary Action

Secondary Action
```

---

Example:

```text id="h5m8x2"
Storage Error

The SD card cannot be read

[Retry]

[Cancel]
```

---

# 15. Recovery animations

Allowed:

* progress indicator
* state transition

Duration:

100-200ms

---

Avoid:

* complex repair animations

---

# 16. Logging

Errors should generate logs.

Stored:

```text id="s6m4x9"
System Logs

/
└── flachead.log
```

---

Logs must not:

* fill storage
* slow system

---

# 17. Performance requirements

Recovery system must:

* run asynchronously
* avoid blocking playback
* use minimal memory

---

Critical rule:

Audio thread priority > recovery tasks.

---

# 18. Acceptance criteria

Implementation is complete when:

* failures do not crash the system
* recovery options exist
* playback survives errors
* user understands the problem
* logs are generated

---

# 19. Architectural recommendations

Required service:

```text id="b8m3x5"
Error Manager

├── Error Detection

├── Severity Classification

├── Recovery Actions

├── Logging

└── User Notification
```

---

Required components:

```text id="x7m4q2"
ErrorCard

RecoveryDialog

NotificationBanner

StatusIndicator
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Prefer graceful degradation.
* Never show raw system errors.
* Keep recovery actions simple.
* Protect user music data.
* Continue playback whenever possible.

FLACHEAD should behave like a reliable hardware player: problems are handled quietly, safely, and intelligently.
