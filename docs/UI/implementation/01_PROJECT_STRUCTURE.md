# 01_PROJECT_STRUCTURE.md

# FLACHEAD UI — Project Structure Specification

## 1. Purpose

This document defines the recommended software structure for implementing FLACHEAD UI.

The goal is to keep the project:

* modular
* maintainable
* easy to debug
* optimized for Raspberry Pi Zero W
* understandable by future developers and AI coding agents

The architecture separates:

* UI presentation
* application logic
* hardware communication
* audio systems
* storage
* platform-specific code

Primary principle:

> Every system should have a clear responsibility and a clear boundary.

---

# 2. Design goals

The project structure must provide:

* separation of concerns
* reusable components
* easy hardware replacement
* testable modules
* minimal dependency conflicts

---

# 3. Recommended project layout

```text id="x7m4q9"
FLACHEAD/

├── src/

│   ├── core/

│   ├── ui/

│   ├── audio/

│   ├── hardware/

│   ├── services/

│   ├── storage/

│   ├── applications/

│   └── platform/


├── assets/

│   ├── fonts/

│   ├── icons/

│   ├── themes/

│   └── images/


├── config/

├── data/

├── tests/

├── tools/

├── docs/

└── build/
```

---

# 4. Core directory

Location:

```text id="m5x8q3"
src/core/
```

Purpose:

Contains systems used everywhere.

Includes:

```text id="v8m3x5"
Event System

State Machine

Configuration Manager

Logger

Resource Manager
```

---

Core must not depend on:

* UI
* hardware
* applications

---

# 5. UI directory

Location:

```text id="c6m4x9"
src/ui/
```

Contains:

```text id="r7m3x8"
screens/

components/

renderer/

themes/

animations/

navigation/
```

---

Responsible for:

* displaying information
* handling visual states
* user interaction

---

UI must not directly access:

* GPIO
* filesystem
* audio decoder

---

# 6. Audio directory

Location:

```text id="n8m4x6"
src/audio/
```

Contains:

```text id="w5m3x7"
Playback Engine

Decoder

Media Session

Queue

DSP Interface
```

---

Audio must remain independent from UI.

---

# 7. Hardware directory

Location:

```text id="a7m4x8"
src/hardware/
```

Contains:

```text id="p6m8x2"
Display Driver

Touch Driver

GPIO

Volume Dial

DSP Hardware
```

---

Hardware layer provides abstractions.

---

# 8. Services directory

Location:

```text id="u5m3x9"
src/services/
```

Contains:

```text id="k8m4x5"
Library Service

Metadata Service

Lyrics Service

Theme Service

Task Service
```

---

Services provide reusable application logic.

---

# 9. Storage directory

Location:

```text id="d7m3x8"
src/storage/
```

Contains:

```text id="h5m8x2"
Database

Cache

File Manager

SD Card Manager
```

---

---

# 10. Applications directory

Location:

```text id="s6m4x9"
src/applications/
```

Purpose:

Third-party or built-in applications.

Examples:

```text id="b8m3x5"
Calculator

File Browser

Settings

Music Player
```

---

Applications should use:

* services
* UI components

---

# 11. Platform directory

Location:

```text id="x7m4q2"
src/platform/
```

Contains:

Hardware-specific implementations.

Examples:

```text id="d8m4x6"
Raspberry Pi

SDL Desktop Testing

Framebuffer
```

---

This allows development on desktop before hardware deployment.

---

# 12. Asset management

Assets are separated:

```text id="m5x8q3"
assets/

├── fonts

├── icons

├── images

├── themes
```

---

No hardcoded asset paths inside code.

---

# 13. Configuration directory

Contains:

```text id="v8m3x5"
Default Config

Hardware Profiles

Build Options
```

---

User configuration belongs in runtime storage.

---

# 14. Testing structure

Location:

```text id="c6m4x9"
tests/
```

Contains:

```text id="r7m3x8"
unit/

integration/

hardware/

performance/
```

---

---

# 15. Documentation structure

Current documentation:

```text id="n8m4x6"
docs/UI/

├── components/

├── foundations/

├── screens/

├── states/

├── hardware/

├── architecture/

└── implementation/
```

---

Existing numbering should remain unchanged.

---

# 16. Dependency rules

Allowed:

```text id="w5m3x7"
UI

↓

Core

↓

Platform
```

---

Avoid:

```text id="a7m4x8"
Hardware

↓

UI Screen
```

---

# 17. Build system recommendations

Recommended:

```text id="p6m8x2"
CMake
```

---

Requirements:

* cross compilation support
* Raspberry Pi build profile
* desktop testing profile

---

# 18. Performance considerations

Avoid:

* unnecessary libraries
* heavy frameworks
* runtime dependency loading

---

Prefer:

* native code
* lightweight systems
* static resources

---

# 19. Acceptance criteria

Implementation is complete when:

* modules are separated correctly
* dependencies are controlled
* hardware can be replaced
* desktop testing works
* Pi deployment is simple

---

# 20. Implementation notes for AI coding agent

When implementing:

* Follow the directory boundaries strictly.
* Do not put business logic inside UI files.
* Keep hardware access behind abstraction layers.
* Keep the audio engine isolated.
* Optimize for the Raspberry Pi Zero W from the beginning.

A clean project structure prevents FLACHEAD from becoming difficult to maintain as features grow.
