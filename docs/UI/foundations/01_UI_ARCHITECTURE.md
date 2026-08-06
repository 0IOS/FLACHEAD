# 01_UI_ARCHITECTURE.md

# FLACHEAD UI — Architecture Foundation

## 1. Purpose

This document defines the overall architecture of the FLACHEAD user interface system.

It establishes the rules that all components, screens, overlays, and services must follow.

FLACHEAD is not designed as a traditional desktop application or mobile interface.

It is a dedicated offline digital audio player running on:

* Raspberry Pi Zero W
* 2.8" portrait TFT touchscreen
* TANCHJIM BUNNY DSP audio chain
* Custom C++ SDL-based UI framework

The architecture prioritizes:

* responsiveness
* low memory usage
* predictable behavior
* hardware integration
* music-first interaction

Primary principle:

> The UI should serve the music system, not compete with it.

---

# 2. Architecture philosophy

FLACHEAD follows an embedded layered architecture.

The UI must be:

* modular
* event-driven
* hardware-independent
* lightweight

Avoid:

* tightly coupled screens
* direct hardware access from UI
* duplicated logic
* unnecessary abstractions

---

# 3. High-level architecture

```text id="1n2q7z"
                 FLACHEAD APPLICATION

                         |

                Application Controller

                         |

        +----------------+----------------+

        |                                 |

   UI System                         Core Services


        |                                 |

 Screen Manager                   Playback Engine

 Overlay Manager                 Media Session

 Component System                Library Database

 Renderer                        Metadata Service

 Input Manager                   Lyrics Engine

 Theme Engine                    Configuration

                         |

                  Hardware Layer

                         |

        +----------------+----------------+

        |                |               |

    TFT Display       Buttons        Audio DSP

                     Encoder        TANCHJIM
```

---

# 4. Core layers

FLACHEAD consists of five major layers.

---

# Layer 1 — Hardware Layer

Responsible for:

* display communication
* touch input
* buttons
* encoder
* audio hardware

Examples:

* TFT driver
* GPIO input
* USB audio interface

Rules:

UI must never directly access hardware.

Bad:

```text id="a1q5xp"
Screen

↓

GPIO Write

↓

LED/Button
```

Good:

```text id="7s8wcz"
Screen

↓

Input Manager

↓

Hardware Driver
```

---

# Layer 2 — System Services Layer

Responsible for device functionality.

Includes:

* Playback Engine
* Library Database
* Audio Manager
* Configuration Service
* Power Manager

This layer contains FLACHEAD logic.

---

# Layer 3 — UI Framework Layer

Responsible for visual presentation.

Includes:

* renderer
* layout system
* components
* animations
* input routing

---

# Layer 4 — Screens Layer

Contains user-facing experiences.

Examples:

```text id="2j3p4s"
Home

Music Player

Library

Settings

Lyrics

Queue
```

Screens should only coordinate components.

They should not contain low-level logic.

---

# Layer 5 — Application Layer

Controls:

* startup
* navigation
* global state
* lifecycle

---

# 5. UI tree structure

Every screen follows:

```text id="xk6z3r"
Screen

 |
 +-- Layout

      |
      +-- Components

            |
            +-- Renderer

            |
            +-- Input Handler
```

Example:

Music Player:

```text id="pf1m3h"
Full Music Player

 |
 +-- Album Artwork

 |
 +-- Track Information

 |
 +-- Progress Bar

 |
 +-- Playback Controls
```

---

# 6. Screen management

Screens are managed centrally.

Architecture:

```text id="q0f5ui"
Screen Manager

├── current_screen
├── previous_screen
├── screen_stack
├── transition_state
└── lifecycle_events
```

Screens receive:

* enter()
* update()
* render()
* handleInput()
* exit()

---

# 7. Overlay architecture

Overlays exist above screens.

Examples:

* Notifications
* Dialogs
* Volume
* Brightness

Rendering order:

```text id="2v90q7"
Background

↓

Current Screen

↓

Mini Player

↓

Overlay Layer

↓

Input Layer
```

Overlays do not replace screens.

---

# 8. Event-driven communication

Subsystems communicate using events.

Example:

```text id="n4p5xu"
Playback Engine

        |

TrackChanged Event

        |

Event Bus

        |

UI Components

        |

Update Display
```

Avoid direct communication.

Bad:

```text id="5f8q31"
Music Player Screen

calls

Playback Engine directly
```

Good:

```text id="f5v8iu"
Music Player

listens

to Playback Events
```

---

# 9. Navigation model

Global navigation:

```text id="qk90yb"
HOME Button

Tap:
Launcher

Double Tap:
Home

Hold:
Task Overview


BACK Button:

Previous state
```

Navigation belongs to:

```text id="s9g2xm"
Navigation Manager
```

Not individual screens.

---

# 10. State ownership

Every piece of state has one owner.

Example:

Playback state:

Owner:

```text id="8n1q3k"
Playback Engine
```

UI only observes.

Theme state:

Owner:

```text id="4p7w2c"
Theme Engine
```

Settings:

Owner:

```text id="1z5q8v"
Configuration Service
```

---

# 11. Threading model

Recommended:

```text id="6h0j3p"
Main Thread

|
+-- UI Rendering
+-- Input Processing


Worker Threads

|
+-- Audio Processing
+-- Database Operations
+-- Metadata Scanning
+-- Artwork Processing
```

Never block UI thread.

---

# 12. Memory rules

Because Pi Zero W has:

* ARM11 CPU
* 512 MB RAM

Rules:

Avoid:

* unnecessary object creation
* duplicate images
* large caches
* dynamic allocations every frame

Prefer:

* object pools
* asset caching
* static resources

---

# 13. Rendering rules

The renderer should support:

* dirty rectangles
* cached surfaces
* partial updates

Avoid:

* full screen redraw every frame
* expensive effects
* real-time blur

---

# 14. Component architecture

Components are reusable.

Examples:

```text id="t8v2e5"
Button

Card

List

Slider

Progress Bar

Text
```

Components should not know about:

* music
* settings
* navigation

They only render and handle local interaction.

---

# 15. Error isolation

Failures should stay contained.

Example:

Bad:

```text id="4v9p1f"
Lyrics Engine Failure

↓

UI Crash
```

Good:

```text id="z7x3m1"
Lyrics Engine Failure

↓

Error Event

↓

Notification
```

---

# 16. Performance targets

Required:

```text id="f8w2q4"
UI Response:
<100ms

Normal Rendering:
30-45 FPS

Ideal:
60 FPS

Boot UI:
Fast startup

Memory:
Stable over long sessions
```

---

# 17. Future expansion

Architecture should allow:

* new apps
* plugins
* alternate displays
* additional audio devices
* theme packs
* hardware revisions

without rewriting the core.

---

# 18. Implementation rules for AI coding agent

When implementing FLACHEAD:

* Keep layers separated.
* Never mix UI and hardware logic.
* Use events instead of direct dependencies.
* Optimize for Pi Zero W first.
* Prefer simple solutions.
* Reuse existing components.
* Preserve music playback priority.
* Avoid unnecessary frameworks.

The final architecture should resemble a dedicated embedded audio operating system, not a scaled-down desktop application.
