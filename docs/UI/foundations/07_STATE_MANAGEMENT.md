# 07_STATE_MANAGEMENT.md

# FLACHEAD UI — State Management Foundation

## 1. Purpose

This document defines how FLACHEAD manages, stores, updates, and synchronizes application state.

A dedicated music player must always know:

* what is playing
* where the user is
* what settings are active
* what hardware is connected
* what should happen after restart

State management provides a single source of truth for all FLACHEAD systems.

Primary principle:

> Every piece of information should have one owner and one reliable path of communication.

---

# 2. Design goals

The State System must:

* Keep UI state separate from service state.
* Preserve important information after shutdown.
* Prevent conflicting states.
* Support asynchronous events.
* Recover safely after crashes.
* Remain lightweight for Raspberry Pi Zero W.

The State System should avoid:

* duplicated state
* hidden state changes
* direct screen-to-screen communication
* unnecessary disk writes

---

# 3. State architecture

High-level model:

```text
                 Application State


                        |


        +---------------+---------------+

        |                               |

   Runtime State                 Persistent State


        |                               |

        |                               |

Memory Only                     Saved Storage


        |

        |

+-------+--------+--------+

|       |        |        |

UI   Audio   Input   System

State State   State   State
```

---

# 4. State ownership model

Every state category has exactly one owner.

Example:

```text
Playback State

Owner:
Playback Engine
```

---

```text
Theme State

Owner:
Theme Engine
```

---

```text
Settings State

Owner:
Configuration Service
```

---

```text
Navigation State

Owner:
Navigation Manager
```

---

UI components only observe and display state.

---

# 5. Runtime state

Runtime state exists only while FLACHEAD is running.

Examples:

* current screen
* active overlays
* current animation
* temporary dialogs
* input focus

Example:

```text
Runtime State

current_screen

overlay_stack

focused_component

animation_state

temporary_flags
```

---

# 6. Persistent state

Persistent state survives reboot.

Stored information:

* settings
* theme preferences
* playback position
* queue
* user preferences

Example:

```text
Persistent State

volume

brightness

theme_mode

last_track

last_position

settings
```

---

# 7. Global application state

The Application Controller maintains global state access.

Architecture:

```text
Application Controller

        |

        |

State Manager

        |

+-------+--------+--------+

UI     Audio    System
```

---

The UI does not directly modify global state.

---

# 8. State update flow

Correct flow:

```text
User Action

↓

Input Event

↓

Service Update

↓

State Change

↓

Event Broadcast

↓

UI Refresh
```

---

Example:

Changing volume:

```text
Encoder Turn

↓

Input Manager

↓

Media Session

↓

Volume State Updated

↓

UI Volume Overlay Updates
```

---

# 9. Event-driven updates

State changes should create events.

Example:

```text
State Changed

        |

Event Bus

        |

Subscribers
```

---

Events:

```text
PlaybackStateChanged

ThemeChanged

SettingsChanged

ScreenChanged

StorageChanged

DeviceChanged
```

---

# 10. UI state

The UI owns only presentation state.

Examples:

```text
UI State

current_page

scroll_position

selected_item

animation_progress

dialog_visible
```

---

The UI does not own:

* songs
* volume
* playback
* database information

---

# 11. Screen state lifecycle

Every screen follows:

```text
Created

↓

Entered

↓

Active

↓

Paused

↓

Exited

↓

Destroyed
```

---

Methods:

```text
onCreate()

onEnter()

update()

render()

onExit()

onDestroy()
```

---

# 12. Navigation state

Navigation Manager stores:

```text
Navigation State

current_screen

previous_screen

screen_stack

transition
```

---

Example:

```text
Home

↓

Launcher

↓

Music Library

↓

Player
```

Back navigation follows the stack.

---

# 13. Playback state

Playback state belongs to the audio system.

Example:

```text
Playback State

track_id

status

position

duration

volume

repeat

shuffle
```

---

Possible states:

```text
Loading

Playing

Paused

Stopped

Error
```

---

# 14. Hardware state

Hardware availability is tracked separately.

Example:

```text
Hardware State

display

touch

buttons

encoder

DSP

storage
```

---

Example:

```text
DSP

Connected

↓

Ready

↓

Disconnected
```

---

# 15. Settings state

Configuration Service owns settings.

Examples:

```text
Settings

brightness

volume_limit

theme_mode

sleep_timer

library_path
```

---

Settings changes:

* update runtime
* save asynchronously

---

# 16. Persistence strategy

Not every state change should write to storage.

Bad:

```text
Volume changed

↓

Write SD card
```

---

Good:

```text
Volume changed

↓

Update memory

↓

Save after delay
```

---

Reason:

Protect SD card lifespan.

---

# 17. Crash recovery

After unexpected shutdown:

FLACHEAD should restore:

* last stable screen
* playback position
* settings
* library state

---

Recovery flow:

```text
Boot

↓

Load Saved State

↓

Validate

↓

Restore

↓

Start Services
```

---

# 18. State validation

Saved data may be invalid.

Example:

Missing track:

```text
Saved Track

↓

Check Library

↓

Not Found

↓

Ignore
```

---

Never allow invalid state to crash startup.

---

# 19. Temporary state

Temporary information should not persist.

Examples:

* open dialogs
* current animation
* touch position
* selected menu highlight

---

# 20. State synchronization

Multiple systems may depend on the same information.

Example:

Track change:

```text
Playback Engine

↓

TrackChanged Event

↓

Mini Player

↓

Full Player

↓

Lyrics

↓

Theme Engine
```

---

No direct communication.

---

# 21. Thread safety

Because services run separately:

State access must be protected.

Rules:

* avoid unsafe shared memory
* use events
* synchronize critical data

---

# 22. Performance considerations

State operations must be lightweight.

Avoid:

* copying large objects
* unnecessary serialization
* frequent disk writes

---

Prefer:

* references
* identifiers
* cached data

---

# 23. Error handling

Invalid state:

Action:

1. detect
2. log
3. repair
4. fallback

Example:

```text
Broken Theme

↓

Load Default Theme
```

---

# 24. Acceptance criteria

Implementation is complete when:

* state ownership is clear
* UI state is separated
* playback state survives UI changes
* settings persist correctly
* crashes recover safely
* events synchronize systems
* storage writes are controlled
* Raspberry Pi Zero W remains responsive

---

# 25. Future improvements

Possible additions:

* cloud backup
* multiple user profiles
* advanced history tracking
* smart resume system
* state snapshots

---

# 26. Architectural recommendations

## State Manager

Create:

```text
State Manager

├── getState()
├── updateState()
├── validateState()
├── saveState()
├── restoreState()
└── notifyChanges()
```

---

## Event Bus

Create:

```text
Event Bus

├── subscribe()
├── unsubscribe()
├── publish()
└── queueEvents()
```

---

## Persistence Manager

Create:

```text
Persistence Manager

├── load()
├── save()
├── backup()
├── validate()
└── recover()
```

---

# 27. Implementation notes for AI coding agent

When implementing:

* Give every state a single owner.
* Use events for communication.
* Do not store UI-only data permanently.
* Avoid unnecessary SD card writes.
* Make recovery a normal feature.
* Keep state handling independent from rendering.
* Preserve playback information whenever possible.

The final FLACHEAD system should behave like a dedicated embedded operating system: stable, recoverable, and always ready to continue the user's music experience.
