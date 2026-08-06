# 02_AI_AGENT_RULES.md

# FLACHEAD UI — AI Coding Agent Rules

## 1. Purpose

This document defines the rules an AI coding agent must follow while implementing FLACHEAD.

The purpose is to prevent:

* architectural drift
* unnecessary complexity
* accidental performance regressions
* broken hardware support
* mixing unrelated systems

The AI agent must treat FLACHEAD as an embedded product, not a normal desktop application.

Primary principle:

> Correct architecture and stability are more important than speed of implementation.

---

# 2. Core development rules

The AI agent must:

* understand existing architecture before modifying code
* follow documentation contracts
* avoid creating duplicate systems
* preserve existing working features
* test changes before moving forward

---

# 3. Architecture protection rules

The following boundaries are mandatory:

```text id="j5m8x2"
UI

|

Events

|

Services

|

Hardware / Backend
```

---

Never allow:

```text id="q7x3m9"
UI

↓

GPIO

```

---

Never allow:

```text id="p4m8x6"
Screen

↓

Database

```

---

Never allow:

```text id="n8m2q5"
Button Component

↓

Playback Engine
```

---

Correct approach:

```text id="w6x9m3"
Component

↓

Event

↓

Service
```

---

# 4. Before modifying code

The AI agent must:

1. Read related documentation.
2. Inspect existing implementation.
3. Identify dependencies.
4. Make the smallest required change.

---

Do not:

* rewrite entire modules unnecessarily
* replace working systems without reason
* introduce frameworks without approval

---

# 5. Codebase exploration rules

Before adding a feature:

Check:

```text id="c8m4x2"
Existing components

Existing services

Existing events

Existing utilities

Existing tests
```

---

Avoid duplicate creation:

Example:

Bad:

```text id="h2m7x9"
Create another Event Manager
```

If one exists:

```text id="m5x8q1"
Use existing Event Bus
```

---

# 6. C++ implementation rules

FLACHEAD uses performance-focused C++.

Preferred:

* clear ownership
* RAII
* smart pointers where appropriate
* explicit lifetimes

---

Avoid:

* unnecessary dynamic allocation
* hidden expensive operations
* large inheritance trees

---

# 7. Memory rules

The AI agent must consider:

* Raspberry Pi Zero W has limited RAM
* SD storage is slower
* memory leaks are serious

---

Avoid:

```cpp id="x3m8q5"
new Object()

inside render loop
```

---

Prefer:

```cpp id="z7m2x4"
Create once

Reuse

Destroy safely
```

---

# 8. Rendering rules

Rendering code must:

* stay fast
* avoid blocking
* avoid unnecessary redraws

---

Never:

```text id="k5m9x3"
Load image

↓

Decode

↓

Resize

↓

Render

Every frame
```

---

Use:

```text id="r8x4m2"
Cache asset

↓

Render cached version
```

---

# 9. UI rules

Screens:

* display state
* handle presentation
* coordinate components

They must not:

* own services
* control hardware
* access files

---

Components:

Must be:

* reusable
* lightweight
* independent

---

# 10. Audio rules

Audio has highest priority.

The AI agent must ensure:

* playback never depends on rendering
* UI failures cannot stop music
* audio threads remain independent

---

Never:

```text id="u3m7x9"
UI thread

↓

Wait for audio decoder
```

---

Correct:

```text id="f6x2m8"
UI command

↓

Audio Event

↓

UI update
```

---

# 11. Hardware rules

Hardware access must use abstraction.

Correct:

```text id="a9m5x7"
GPIO

↓

Hardware Backend

↓

Input Manager

↓

Events
```

---

Never:

```text id="m4x8q2"
Screen Code

↓

GPIO Pin
```

---

# 12. Documentation rules

Every new system requires documentation.

Before adding:

* component
* service
* screen
* event

Update:

```text id="p7x3m6"
docs/UI/
```

---

Documentation should include:

* purpose
* architecture
* usage
* performance impact
* acceptance criteria

---

# 13. Dependency rules

Avoid unnecessary dependencies.

Before adding a library:

Ask:

* Is it required?
* Does it run on Pi Zero W?
* Does it increase build size?
* Does it improve the project?

---

Prefer:

* standard library
* existing dependencies

---

# 14. Feature priority rules

Feature priority:

```text id="x5m8q3"
Audio Reliability

↓

Input Responsiveness

↓

Core UI

↓

Usability

↓

Visual Polish
```

---

Aesthetic improvements never override stability.

---

# 15. Error handling rules

The AI agent must assume:

* hardware can fail
* storage can fail
* files can be corrupted

---

Every subsystem needs:

* fallback behavior
* error reporting
* recovery path

---

Never:

```text id="b2m7x8"
Crash application

because artwork is missing
```

---

# 16. Testing rules

After changes:

Run:

* build
* tests
* hardware checks

---

Minimum validation:

```text id="g9x4m2"
Does it compile?

Does it run?

Does existing functionality still work?
```

---

# 17. Performance review checklist

Before accepting code:

Check:

CPU:

* unnecessary loops?

Memory:

* leaks?

Rendering:

* extra redraws?

Storage:

* unnecessary writes?

Audio:

* interruptions?

---

# 18. Git rules

The AI agent should:

* make logical commits
* avoid huge unrelated changes
* keep history understandable

---

Preferred:

```text id="q6m3x9"
Add renderer

Fix input system

Implement player screen
```

---

Avoid:

```text id="w8m2x5"
Massive everything update
```

---

# 19. When uncertain

The AI agent must:

1. Check documentation.
2. Check existing architecture.
3. Prefer simpler implementation.
4. Avoid assumptions.

---

If a decision affects:

* hardware
* architecture
* performance

it should be documented.

---

# 20. Acceptance criteria

The AI agent implementation is acceptable when:

* architecture remains clean
* documentation matches code
* performance targets are respected
* audio remains stable
* hardware abstraction is preserved
* changes are testable

---

# 21. Final AI instruction

Build FLACHEAD as if it were commercial embedded firmware.

Prioritize:

* reliability
* responsiveness
* maintainability

Avoid:

* unnecessary complexity
* desktop-style assumptions
* temporary hacks

Every implementation decision should answer:

> "Will this still work smoothly on Raspberry Pi Zero W after months of use?"

If the answer is no, redesign it.
