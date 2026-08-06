# 02_SCREEN_LIFECYCLE.md

# FLACHEAD UI — Screen Lifecycle Contract

## 1. Purpose

This document defines the implementation contract for all FLACHEAD screens.

Screens represent complete user-facing experiences.

Examples:

* Home
* App Launcher
* Music Library
* Full Music Player
* Settings
* Lyrics View

A screen coordinates:

* components
* layout
* user interaction
* screen-specific presentation

A screen does not own:

* playback logic
* hardware access
* database operations
* global application state

Primary principle:

> A screen is a view of system state, not the owner of that state.

---

# 2. Screen architecture

Every screen follows:

```text id="w5n8q2"
Screen Manager

        |

        v

Screen Instance

        |

+-------+--------+

Layout          Components

        |

        v

Renderer
```

---

# 3. Base screen interface

Every screen implements:

```cpp id="m7x2p9"
class Screen
{

public:

    virtual void onCreate();

    virtual void onEnter();

    virtual void update();

    virtual void render();

    virtual void handleInput();

    virtual void onPause();

    virtual void onResume();

    virtual void onExit();

    virtual void onDestroy();

};
```

---

# 4. Screen lifecycle states

A screen moves through:

```text id="q8m4z1"
Created

↓

Initialized

↓

Active

↓

Paused

↓

Inactive

↓

Destroyed
```

---

# 5. Lifecycle methods

## onCreate()

Called once.

Purpose:

* allocate screen resources
* create components
* initialize layout

Example:

```text id="p3k7x9"
Create:

Album Artwork Component

Track Info Component

Controls
```

---

Must not:

* start playback
* perform long operations

---

# 6. onEnter()

Called whenever the screen becomes visible.

Used for:

* refreshing displayed data
* subscribing to events
* updating state

Example:

Music Player:

```text id="r6m2x8"
Subscribe:

TrackChanged

PositionUpdated

PlaybackStateChanged
```

---

# 7. update()

Called during active usage.

Responsible for:

* animation updates
* local UI changes

Avoid:

* database queries
* file loading
* heavy calculations

---

# 8. render()

Responsible for:

* drawing components
* arranging layers

Example:

```text id="n9x4m6"
Background

↓

Content

↓

Controls
```

---

Render must:

* be fast
* avoid state modification

---

# 9. handleInput()

Receives:

* touch events
* navigation events

Example:

```text id="v2m8q5"
Tap Album

↓

Open Artist Page
```

---

Input handling order:

```text id="h7p3x9"
Global Input

↓

Screen Input

↓

Component Input
```

---

# 10. onPause()

Called when another screen temporarily appears.

Examples:

* dialog opened
* overlay displayed

The screen remains in memory.

Should:

* pause animations
* reduce updates

Should not:

* destroy resources

---

# 11. onResume()

Called when returning.

Used for:

* refreshing state
* restarting animations

---

# 12. onExit()

Called when leaving.

Used for:

* unsubscribing events
* saving temporary state

---

# 13. onDestroy()

Called when removed permanently.

Responsible for:

* releasing memory
* destroying components
* freeing resources

---

# 14. Screen Manager

The Screen Manager owns:

```text id="k8m5x2"
Screen Stack

Current Screen

Previous Screen

Transitions
```

---

Architecture:

```text id="x3n7q4"
Screen Manager

├── pushScreen()

├── popScreen()

├── replaceScreen()

├── update()

├── render()

└── destroy()
```

---

# 15. Screen stack behavior

Navigation example:

```text id="z4m8p6"
Home

↓

Launcher

↓

Library

↓

Player
```

Back:

```text id="c7x2m9"
Player

↓

Library

↓

Launcher

↓

Home
```

---

# 16. Screen memory strategy

Because Raspberry Pi Zero W has limited RAM:

Not every screen should remain loaded.

Recommended:

## Persistent screens

Keep loaded:

* Home
* Mini Player
* System overlays

---

## Temporary screens

Unload:

* Settings
* Library views
* Dialog-heavy pages

---

# 17. Screen transitions

Transitions must be lightweight.

Allowed:

* fade
* slide
* instant switch

Duration:

100-300ms

---

Avoid:

* complex animations
* expensive effects

---

# 18. Screen data loading

Screens should request data asynchronously.

Bad:

```text id="m9q4x7"
Open Library

↓

Scan SD card

↓

Freeze UI
```

---

Good:

```text id="v5n8m2"
Open Library

↓

Show cached data

↓

Update when scan finishes
```

---

# 19. Event subscriptions

Screens may subscribe to:

* playback events
* theme events
* hardware events

Example:

```text id="a8x3m7"
Music Player

subscribes:

TrackChanged

VolumeChanged
```

---

Before destruction:

unsubscribe.

---

# 20. Screen-specific state

Screens may own:

* scroll position
* selected item
* temporary filters

Example:

Library:

```text id="f4m7x8"
selected_album

scroll_offset

search_text
```

---

They may not own:

```text id="q2n6p9"
Current Song

Volume

Settings
```

---

# 21. Error handling

Screen errors should stay isolated.

Example:

Artwork failure:

```text id="b8x5m1"
Missing Artwork

↓

Placeholder

↓

Continue
```

---

Screen crash protection:

```text id="w6m3q9"
Screen Error

↓

Error Manager

↓

Fallback Screen
```

---

# 22. Performance requirements

Every screen must:

* enter quickly
* avoid unnecessary loading
* release unused resources
* maintain FPS

Targets:

Screen transition:

<300ms

Input response:

<100ms

---

# 23. Testing requirements

Each screen must test:

* opening
* closing
* navigation
* input
* theme changes
* missing data
* low memory conditions

---

# 24. Acceptance criteria

A screen implementation is complete when:

* follows lifecycle
* uses Screen Manager
* does not own services
* handles events correctly
* releases resources
* performs on Pi Zero W

---

# 25. Architectural recommendations

Screen base class:

```text id="y7m2x4"
Screen

├── onCreate()

├── onEnter()

├── update()

├── render()

├── handleInput()

├── onPause()

├── onResume()

├── onExit()

└── onDestroy()
```

---

Screen Manager:

```text id="n3x8p5"
Screen Manager

├── stack

├── current

├── transition

├── lifecycle

└── memory_control
```

---

# 26. Implementation notes for AI coding agent

When implementing screens:

* Never put backend logic inside screens.
* Never access hardware directly.
* Use services and events.
* Follow lifecycle methods.
* Keep screens lightweight.
* Preserve playback while navigating.
* Optimize for the 2.8" display.

A FLACHEAD screen should behave like a view layer in an embedded operating system: fast, replaceable, and completely controlled by the architecture around it.
