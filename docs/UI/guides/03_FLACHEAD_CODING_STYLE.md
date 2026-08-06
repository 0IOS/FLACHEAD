# 03_FLACHEAD_CODING_STYLE.md

# FLACHEAD UI — Coding Style Guide

## 1. Purpose

This document defines coding standards for the FLACHEAD codebase.

The goal is to maintain:

* readable code
* predictable architecture
* easy debugging
* long-term maintainability

FLACHEAD is an embedded system. Code clarity directly affects reliability.

Primary principle:

> Simple, explicit, and predictable code is preferred over clever solutions.

---

# 2. General coding philosophy

Code should prioritize:

1. Reliability
2. Readability
3. Performance
4. Extensibility
5. Shortness

---

Avoid:

* unnecessary abstractions
* overly generic systems
* complicated patterns without purpose

---

# 3. Language standards

Primary language:

```text id="m8x4q2"
C++
```

Recommended standard:

```text id="p7m3x9"
C++17
```

---

Use:

* standard library
* RAII
* strong typing
* clear ownership

---

Avoid:

* compiler-specific hacks
* unnecessary macros
* undefined behavior

---

# 4. Folder structure

Recommended source layout:

```text id="x5m8q3"
src/

├── core/

│   ├── Application

│   ├── Config

│   └── Logger


├── ui/

│   ├── components

│   ├── screens

│   ├── renderer

│   └── theme


├── audio/

│   ├── playback

│   ├── session

│   └── queue


├── hardware/

│   ├── input

│   ├── display

│   └── storage


├── events/

└── services/
```

---

# 5. Naming conventions

## Classes

Use PascalCase.

Example:

```cpp id="c6m2x8"
class MusicPlayerScreen;

class AudioSession;

class ThemeManager;
```

---

## Functions

Use camelCase.

Example:

```cpp id="q8x5m1"
loadArtwork();

updateVolume();

renderScreen();
```

---

## Variables

Use camelCase.

Example:

```cpp id="r4m9x2"
currentTrack;

screenWidth;

volumeLevel;
```

---

## Constants

Use uppercase snake case.

Example:

```cpp id="w7m3x5"
MAX_VOLUME

DEFAULT_FPS

SCREEN_WIDTH
```

---

# 6. File naming

Classes:

```text id="n3m8x7"
MusicPlayerScreen.hpp

MusicPlayerScreen.cpp
```

---

Components:

```text id="v6x2m9"
Button.hpp

Button.cpp
```

---

Documentation:

```text id="p5m8q4"
SCREAMING_SNAKE_CASE.md
```

---

# 7. Header file rules

Headers should:

* contain declarations
* avoid unnecessary includes

Prefer:

```cpp id="k4m8x2"
forward declarations
```

when possible.

---

Example:

```cpp
class Renderer;

class Screen
{
    Renderer* renderer;
};
```

---

# 8. Include order

Recommended:

```cpp id="x9m3q6"
// Own header

#include "Screen.hpp"


// Standard library

#include <vector>
#include <string>


// External libraries

#include <SDL.h>


// Project headers

#include "Renderer.hpp"
```

---

# 9. Class design

Classes should have:

* one responsibility
* clear ownership
* limited dependencies

---

Good:

```text id="m7x2p5"
AudioSession

handles audio communication
```

---

Bad:

```text id="z8m4q1"
AudioSession

handles:

audio

UI

database

hardware
```

---

# 10. Constructor rules

Constructors should initialize.

Example:

```cpp id="a5m8x3"
Renderer::Renderer()
    :
    width(240),
    height(320)
{
}
```

---

Avoid:

* heavy operations
* file loading
* network access

---

# 11. Memory management

Preferred:

```cpp id="q6m3x9"
std::unique_ptr

std::shared_ptr

std::vector
```

when appropriate.

---

Avoid:

```cpp
malloc()

free()

raw ownership pointers
```

---

Raw pointers are acceptable for:

* references
* non-owning relationships

---

# 12. Ownership rules

Every resource must have an owner.

Examples:

Texture:

```text id="j8m4x2"
Asset Manager
```

---

Playback:

```text id="w5x9m3"
Audio Engine
```

---

Screen:

```text id="n7m2q6"
Screen Manager
```

---

# 13. Error handling style

Avoid silent failures.

Bad:

```cpp
loadFile();
```

---

Good:

```cpp
if(!loadFile())
{
    Logger::error("Failed loading file");
}
```

---

Failures should:

* log
* recover
* fallback

---

# 14. Logging rules

Levels:

```text id="u4m8x1"
DEBUG

INFO

WARNING

ERROR
```

---

Examples:

INFO:

```text
Audio device connected
```

WARNING:

```text
Artwork missing
```

ERROR:

```text
Playback failed
```

---

# 15. Comments

Comments should explain:

* why something exists
* design decisions

Avoid:

```cpp
// increment counter
counter++;
```

---

Prefer:

```cpp
// Keep audio buffer above minimum threshold
// to prevent playback interruptions.
```

---

# 16. Performance-sensitive code

Mark clearly:

```cpp
// PERFORMANCE:
// Called every frame.
// Avoid allocations here.
```

---

Examples:

* render loop
* audio callback
* input polling

---

# 17. Render code rules

Inside render functions:

Allowed:

* drawing
* positioning
* cached resource access

Avoid:

* allocation
* file access
* database queries

---

# 18. Event handling style

Events should use clear names.

Example:

```cpp
EventType::TRACK_CHANGED
```

---

Avoid:

```cpp
EventType::THING_UPDATED
```

---

# 19. Configuration style

Hardware-specific values should not be hardcoded.

Bad:

```cpp
GPIO17
```

---

Good:

```cpp
config.homeButtonPin
```

---

# 20. Testing style

Tests should follow:

```text id="f3m7x8"
Feature

↓

Expected behavior

↓

Failure condition
```

---

Example:

```text
Volume encoder

Turn clockwise

Volume increases
```

---

# 21. Commit style

Commits should describe one change.

Format:

```text id="y5m8x2"
type: description
```

Examples:

```text
add: renderer initialization

fix: touch coordinate mapping

refactor: event dispatch system
```

---

Avoid:

```text
update stuff
```

---

# 22. Code review checklist

Before merging:

Check:

* Is ownership clear?
* Is the architecture respected?
* Does it run on Pi Zero W?
* Does it affect audio?
* Are errors handled?

---

# 23. Acceptance criteria

Code is acceptable when:

* style is consistent
* ownership is clear
* responsibilities are separated
* performance rules are followed
* future developers can understand it

---

# 24. Implementation notes for AI coding agent

When writing code:

* Follow existing naming.
* Avoid unnecessary files.
* Keep classes focused.
* Prefer simple solutions.
* Document important decisions.
* Optimize only where needed.
* Never sacrifice maintainability for cleverness.

FLACHEAD code should look like firmware written for a reliable physical product: clean, deliberate, and easy to maintain.
