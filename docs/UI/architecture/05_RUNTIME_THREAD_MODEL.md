# 05_RUNTIME_THREAD_MODEL.md

# FLACHEAD UI — Runtime Thread Model Specification

## 1. Purpose

This document defines how FLACHEAD separates system responsibilities across execution contexts.

FLACHEAD runs on Raspberry Pi Zero W, where CPU resources are limited.

A correct threading model is required to protect:

* audio playback stability
* UI responsiveness
* input latency
* background processing

Primary principle:

> No visual feature or background operation should ever compromise audio playback.

---

# 2. Design goals

The runtime model must be:

* predictable
* lightweight
* safe
* low latency
* easy to debug

---

# 3. Thread architecture

FLACHEAD uses separated execution domains:

```text id="x7m4q9"
Audio Thread

↓

UI Thread

↓

Input Thread

↓

Worker Threads

↓

Hardware Monitoring
```

---

# 4. Audio thread

Highest priority thread.

Responsibilities:

* FLAC decoding
* audio buffering
* DSP communication
* playback timing

---

The audio thread must never:

* wait for UI
* wait for storage
* perform heavy calculations unrelated to playback

---

Example:

Bad:

```text id="m5x8q3"
Audio Thread

↓

Load Album Artwork
```

---

Correct:

```text id="v8m3x5"
Audio Thread

↓

Send Track Event

↓

UI Loads Artwork
```

---

# 5. UI thread

Responsibilities:

* screen updates
* rendering
* animations
* UI state changes

---

The UI thread handles:

```text id="c6m4x9"
State Machine

Renderer

Component Updates

Transitions
```

---

The UI thread must remain responsive.

---

# 6. Input thread

Responsibilities:

* GPIO buttons
* touchscreen events
* volume dial

---

Flow:

```text id="r7m3x8"
Hardware Input

↓

Input Thread

↓

Event Queue

↓

UI Thread
```

---

Input should never directly modify UI.

---

# 7. Worker threads

Used for:

* library scanning
* metadata extraction
* artwork processing
* database maintenance

---

Worker tasks are:

* lower priority
* cancellable
* limited in number

---

# 8. Thread communication

Threads communicate through:

```text id="n8m4x6"
Event Queue

+

Message Passing
```

---

Avoid:

* shared mutable state
* direct cross-thread calls

---

# 9. Thread safety rules

Allowed:

```text id="w5m3x7"
Thread A

↓

Event

↓

Thread B
```

---

Avoid:

```text id="a7m4x8"
Thread A

↓

Modify Thread B Data
```

---

# 10. Locking strategy

Locks should be minimal.

---

Avoid:

Large global locks.

Reason:

They create:

* delays
* frame drops
* audio interruptions

---

Prefer:

* atomic values
* queues
* ownership transfer

---

# 11. Playback synchronization

Important synchronization points:

```text id="p6m8x2"
Track Change

Playback Position

Pause State

Volume Change
```

---

Example:

Playback position:

Audio thread:

```text id="u5m3x9"
Updates Time
```

↓

Event:

```text id="k8m4x5"
UI Updates Progress Bar
```

---

# 12. Background task limits

Raspberry Pi Zero W limitations require controlled concurrency.

Recommended:

```text id="d7m3x8"
1 Audio Thread

1 UI Thread

1 Input Thread

1-2 Worker Threads
```

---

Avoid:

Creating many small threads.

---

# 13. Thread priority

Priority order:

```text id="h5m8x2"
1. Audio

2. Input

3. UI

4. Hardware Monitoring

5. Background Tasks
```

---

# 14. Shutdown handling

Threads must stop gracefully.

Order:

```text id="s6m4x9"
Stop New Tasks

↓

Finish Audio Buffer

↓

Stop Workers

↓

Close Hardware

↓

Shutdown
```

---

# 15. Error handling

Thread failure:

Must:

* log error
* attempt recovery
* avoid full system crash

---

Example:

Worker crash:

```text id="b8m3x5"
Restart Worker

Continue Playback
```

---

# 16. Performance requirements

Thread operations must:

* avoid unnecessary context switching
* avoid excessive synchronization
* maintain stable timing

---

Targets:

Audio:

Real-time priority.

---

UI:

30-60 FPS.

---

Input:

<50ms response.

---

# 17. Acceptance criteria

Implementation is complete when:

* audio never depends on UI
* UI never blocks playback
* workers run independently
* input remains responsive
* shutdown is clean

---

# 18. Architectural recommendations

Required modules:

```text id="x7m4q2"
Runtime Manager

├── Audio Thread

├── UI Thread

├── Input Thread

├── Worker Pool

└── Event Bridge
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Keep thread ownership clear.
* Use events instead of shared state.
* Profile CPU usage on actual Pi Zero W.
* Keep worker count low.
* Protect the audio pipeline above everything else.

A good runtime model is what allows FLACHEAD to deliver high-quality audio while maintaining a smooth and responsive interface on extremely limited hardware.
