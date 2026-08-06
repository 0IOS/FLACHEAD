# 04_TESTING_STRATEGY.md

# FLACHEAD UI — Testing Strategy

## 1. Purpose

This document defines the testing strategy for FLACHEAD.

The goal is to ensure:

* stable playback
* responsive UI
* reliable hardware interaction
* predictable behavior
* long-term operation on Raspberry Pi Zero W

FLACHEAD is an embedded music device. Testing must validate the complete system, not only individual features.

Primary principle:

> A feature is not complete when it works once. It is complete when it remains reliable under normal usage.

---

# 2. Testing philosophy

Testing priority:

```text
Audio Reliability

↓

Hardware Reliability

↓

UI Stability

↓

Performance

↓

Visual Polish
```

---

# 3. Testing layers

FLACHEAD testing is divided into:

```text
Unit Testing

Integration Testing

Hardware Testing

Performance Testing

User Experience Testing

Recovery Testing
```

---

# 4. Unit testing

## Purpose

Verify individual systems independently.

Test:

* classes
* utilities
* services
* data structures

---

Examples:

```text
Audio Queue

↓

Add track

↓

Correct order returned
```

---

```text
Theme Manager

↓

Generate colors

↓

Valid tokens returned
```

---

# 5. Components requiring unit tests

Required:

```text
Event Bus

Theme Engine

Configuration Manager

Queue Manager

Metadata Parser

Library Database

Input Parser
```

---

# 6. Event Bus tests

Verify:

## Publishing

```text
Create event

↓

Publish

↓

Listener receives
```

---

## Multiple listeners

```text
One event

↓

Multiple subscribers
```

---

## Removing listeners

```text
Subscribe

↓

Unsubscribe

↓

No callback
```

---

# 7. UI component testing

Every component should test:

* creation
* rendering
* input handling
* state changes
* destruction

---

Example:

Button:

```text
Render button

↓

Tap

↓

Callback triggered
```

---

# 8. Screen testing

Every screen must verify:

## Opening

```text
Screen Manager

↓

Screen appears
```

---

## Navigation

```text
Screen A

↓

Screen B

↓

Back
```

---

## Lifecycle

Test:

```text
onCreate()

onEnter()

onPause()

onResume()

onDestroy()
```

---

# 9. Audio testing

Audio is the highest priority system.

Test:

## Playback

```text
Load FLAC

↓

Play

↓

Pause

↓

Resume
```

---

## Track switching

```text
Track A

↓

Next

↓

Track B
```

---

## Queue

```text
Add tracks

↓

Play order correct
```

---

## Recovery

```text
Remove audio device

↓

System recovers
```

---

# 10. TANCHJIM BUNNY DSP testing

Required tests:

## Connection

```text
DSP connected

↓

Detected correctly
```

---

## Removal

```text
DSP removed

↓

Error handled
```

---

## Reconnection

```text
Reconnect DSP

↓

Playback restored
```

---

# 11. Hardware testing

Test all physical inputs:

```text
Touchscreen

UP Button

DOWN Button

SELECT Button

HOME Button

BACK Button

PLAY Button

Volume Encoder
```

---

# 12. Touchscreen tests

Verify:

* coordinate accuracy
* portrait orientation
* gestures
* scrolling
* button detection

---

Failure examples:

```text
Touch offset

↓

Wrong action triggered
```

---

# 13. Button testing

Test:

## Single press

Expected:

```text
One event
```

---

## Long press

Expected:

```text
Correct long press event
```

---

## Double press

Expected:

```text
Double action
```

---

# 14. Performance testing

FLACHEAD target hardware:

```text
Raspberry Pi Zero W
```

---

Measure:

* FPS
* CPU usage
* RAM usage
* startup time
* input latency

---

# 15. Performance targets

## Rendering

Target:

```text
30-45 FPS minimum
```

Preferred:

```text
60 FPS
```

---

## Input latency

Target:

```text
<100ms
```

---

## Screen transition

Target:

```text
<300ms
```

---

## Memory

Avoid:

* continuous growth
* leaks
* unnecessary caching

---

# 16. Frame performance testing

Measure:

```text
Average FPS

Minimum FPS

Frame time

Worst frame time
```

---

Example:

```text
Average:

60 FPS


Worst frame:

22ms
```

---

# 17. Startup testing

Measure:

```text
Power on

↓

Boot

↓

UI ready
```

---

Test:

* normal boot
* missing SD card data
* corrupted settings
* missing artwork

---

# 18. Storage testing

Test:

* SD card insertion
* SD card removal
* corrupted files
* large music libraries

---

Expected:

System remains usable.

---

# 19. Stress testing

Long-running test:

```text
Playback

+

UI navigation

+

Touch input

+

Screen changes
```

Duration:

Several hours.

---

Check:

* memory leaks
* audio interruptions
* crashes

---

# 20. Recovery testing

Every failure needs validation.

Examples:

## Missing artwork

Expected:

```text
Use placeholder
```

---

## Missing lyrics

Expected:

```text
Show unavailable state
```

---

## Audio failure

Expected:

```text
Show error

Attempt recovery
```

---

# 21. Visual testing

Verify:

* alignment
* spacing
* readability
* theme changes

---

Test:

Different:

* wallpapers
* album artwork
* brightness levels

---

# 22. Hardware simulation testing

Before physical testing:

Use:

* mock input backend
* simulated audio backend
* fake events

---

Allows development without hardware.

---

# 23. Release testing checklist

Before release:

```text
☐ Boots correctly

☐ Touch works

☐ Buttons work

☐ Audio playback stable

☐ Queue works

☐ Library works

☐ Themes work

☐ No crashes

☐ Memory stable

☐ Performance acceptable
```

---

# 24. Bug reporting format

Every bug should include:

```text
Title:

Steps:

Expected:

Actual:

Hardware:

Logs:

Severity:
```

---

# 25. Severity levels

## Critical

System unusable.

Examples:

* boot failure
* audio corruption

---

## Major

Important feature broken.

Examples:

* library unavailable

---

## Minor

Small issue.

Examples:

* visual alignment

---

# 26. Acceptance criteria

Testing is complete when:

* all major systems pass
* hardware works reliably
* performance targets are met
* failures recover correctly
* long-term playback is stable

---

# 27. Implementation notes for AI coding agent

When implementing:

* Add tests with new systems.
* Never assume hardware works.
* Measure performance instead of guessing.
* Test failure cases.
* Preserve previous functionality.

FLACHEAD should be validated like a consumer electronics product, not just a software project.
