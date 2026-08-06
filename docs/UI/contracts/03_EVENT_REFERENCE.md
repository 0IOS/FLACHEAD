# 03_EVENT_REFERENCE.md

# FLACHEAD UI — Event System Contract

## 1. Purpose

This document defines the event communication system used throughout FLACHEAD.

Events are the primary communication method between:

* UI
* audio services
* hardware services
* system services
* applications

The event system prevents direct dependencies between subsystems.

Primary principle:

> Systems announce changes; interested systems react.

---

# 2. Event architecture

FLACHEAD uses a centralized event bus.

Architecture:

```text id="q8m3x7"
Subsystem

     |

     v

Event Bus

     |

+----+----+----+

UI  Audio  System

```

---

# 3. Why events are required

Without events:

```text id="m5x9p2"
Music Player Screen

        |

direct call

        |

Playback Engine
```

Problems:

* tight coupling
* difficult testing
* harder expansion

---

With events:

```text id="z7q2m8"
Playback Engine

        |

TrackChanged Event

        |

Event Bus

        |

Any Listener
```

Benefits:

* modular architecture
* easier debugging
* future plugins
* independent services

---

# 4. Event structure

Every event follows:

```cpp id="x4m8q1"
Event
{
    type;

    source;

    timestamp;

    payload;
}
```

---

Example:

```text id="n6p3x9"
Event:

type:
TRACK_CHANGED

source:
Playback Engine

payload:
new_track_id
```

---

# 5. Event naming rules

Format:

```text id="k7m2v5"
CATEGORY_ACTION
```

Examples:

```text id="p4x8m1"
TRACK_CHANGED

VOLUME_CHANGED

THEME_UPDATED

SCREEN_CHANGED
```

---

Rules:

* uppercase
* descriptive
* no implementation details

---

# 6. Event categories

FLACHEAD events are divided into:

```text id="w3n8m6"
Playback Events

Input Events

Navigation Events

Theme Events

Hardware Events

Library Events

System Events

Error Events
```

---

# 7. Playback events

Owned by:

```text id="q5m9x2"
Playback Engine

Media Session Manager
```

---

## TRACK_CHANGED

Triggered:

When current track changes.

Payload:

```text id="a8x4m7"
track_id

title

artist

album
```

Consumers:

* Music Player
* Mini Player
* Lyrics
* Theme Engine

---

## PLAYBACK_STARTED

Payload:

```text id="m2x7p9"
track_id
```

---

## PLAYBACK_PAUSED

---

## PLAYBACK_STOPPED

---

## POSITION_CHANGED

Payload:

```text id="v8m3q5"
position

duration
```

Recommended frequency:

250-500ms

---

## PLAYBACK_ERROR

Payload:

```text id="s4x9m2"
error_code

description

recovery_action
```

---

# 8. Volume events

## VOLUME_CHANGED

Payload:

```text id="p7m2x8"
old_value

new_value
```

Consumers:

* Volume Overlay
* Audio Service
* Settings

---

# 9. Audio device events

## AUDIO_DEVICE_CONNECTED

Payload:

```text id="y3x8m5"
device_name
```

---

## AUDIO_DEVICE_DISCONNECTED

Used for:

* TANCHJIM BUNNY DSP removal
* USB audio failures

---

## AUDIO_DEVICE_ERROR

Payload:

```text id="c9m4x7"
error
```

---

# 10. Input events

Owned by:

```text id="f8m3q2"
Input Manager
```

---

## BUTTON_PRESSED

Payload:

```text id="r5x7m1"
button_id
```

---

## BUTTON_RELEASED

---

## BUTTON_LONG_PRESS

---

## TOUCH_TAP

Payload:

```text id="n4m8q6"
x

y
```

---

## TOUCH_SWIPE

Payload:

```text id="h7x2m9"
direction

distance
```

---

## ENCODER_CHANGED

Payload:

```text id="j5m8p3"
direction

steps
```

---

# 11. Navigation events

Owned by:

Navigation Manager.

---

## SCREEN_CHANGED

Payload:

```text id="b8m2x5"
previous

current
```

---

## HOME_REQUESTED

Triggered by:

Home button.

---

## BACK_REQUESTED

Triggered by:

Back button.

---

## TASK_VIEW_REQUESTED

Triggered by:

Hold home.

---

# 12. Theme events

Owned by:

Theme Engine.

---

## THEME_CHANGED

Payload:

```text id="x9m3q7"
theme_id

source
```

---

## WALLPAPER_CHANGED

---

## ALBUM_THEME_UPDATED

Triggered when album artwork generates new colors.

Consumers:

* Music Player
* Mini Player
* Lyrics

---

# 13. Library events

Owned by:

Library Database.

---

## LIBRARY_SCAN_STARTED

---

## LIBRARY_SCAN_PROGRESS

Payload:

```text id="m6x2p8"
percentage
```

---

## LIBRARY_SCAN_COMPLETED

---

## LIBRARY_ERROR

---

# 14. Hardware events

Owned by:

Hardware Layer.

---

## DISPLAY_READY

---

## TOUCH_AVAILABLE

---

## TOUCH_FAILURE

---

## STORAGE_CONNECTED

---

## STORAGE_REMOVED

---

# 15. System events

Owned by:

System Manager.

---

## BOOT_COMPLETED

---

## SHUTDOWN_REQUESTED

---

## SLEEP_ENTERED

---

## SLEEP_EXITED

---

# 16. Error events

Owned by:

Error Manager.

---

## ERROR_OCCURRED

Payload:

```text id="v7m3q9"
severity

module

message
```

---

## RECOVERY_STARTED

---

## RECOVERY_COMPLETED

---

# 17. Event subscription model

Services subscribe:

```cpp id="c4m9x7"
eventBus.subscribe(
EVENT_NAME,
callback
);
```

---

Example:

```text id="t8m2p5"
Music Player

subscribes:

TRACK_CHANGED

POSITION_CHANGED
```

---

# 18. Event lifecycle

Flow:

```text id="q2x7m9"
Create Event

↓

Publish

↓

Queue

↓

Dispatch

↓

Listeners React

↓

Destroy Event
```

---

# 19. Thread safety

Events may come from:

* audio thread
* worker threads
* input thread

Therefore:

Use:

* thread-safe queue
* controlled dispatch

---

# 20. Event performance rules

Events must be:

* small
* lightweight
* asynchronous

Avoid:

```text id="m7x4p8"
Sending large artwork data
inside event
```

---

Prefer:

```text id="x5n2q9"
Send artwork_id

Load from cache
```

---

# 21. Debugging events

Debug mode should allow:

```text id="p3m8x6"
EVENT LOG

12:01 TRACK_CHANGED

12:02 VOLUME_CHANGED

12:03 THEME_CHANGED
```

---

Production:

Only important events logged.

---

# 22. Acceptance criteria

Implementation is complete when:

* all systems communicate through events
* event names are consistent
* events are thread-safe
* payloads remain lightweight
* UI is not tightly coupled
* debugging is possible

---

# 23. Architectural recommendations

## Event Bus

```text id="y8m4q2"
Event Bus

├── publish()

├── subscribe()

├── unsubscribe()

├── queue()

└── dispatch()
```

---

## Event categories

Organize:

```text id="r9x2m5"
events/

├── playback

├── input

├── navigation

├── theme

├── hardware

├── library

└── system
```

---

# 24. Implementation notes for AI coding agent

When implementing:

* Prefer events over direct calls.
* Keep payloads minimal.
* Never send heavy objects through events.
* Keep event ownership clear.
* Make events asynchronous.
* Ensure thread safety.

The final event system should make FLACHEAD expandable like a real embedded OS: independent modules communicating cleanly without becoming tightly connected.
