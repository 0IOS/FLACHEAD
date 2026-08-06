# 08_CONFIGURATION_SYSTEM.md

# FLACHEAD UI — Configuration System Specification

## 1. Purpose

This document defines how FLACHEAD stores, loads, and manages user and system configuration.

FLACHEAD is an offline dedicated music player. Configuration must remain:

* local
* reliable
* fast
* recoverable

The configuration system controls:

* user preferences
* UI behavior
* hardware settings
* playback options
* personalization

Primary principle:

> Configuration should survive reboots without risking system stability.

---

# 2. Design goals

The configuration system must be:

* persistent
* lightweight
* fault tolerant
* easy to migrate
* optimized for SD card storage

---

# 3. Configuration architecture

Data flow:

```text id="x7m4q9"
UI / Services

↓

Configuration Manager

↓

Storage Layer

↓

Configuration Files / Database
```

---

No component should directly modify configuration files.

---

# 4. Configuration categories

FLACHEAD configuration contains:

```text id="m5x8q3"
1. User Settings

2. Playback Settings

3. Display Settings

4. Hardware Settings

5. System Settings

6. Application Settings
```

---

# 5. User settings

Examples:

```text id="v8m3x5"
Theme Preference

Language

Favorite UI Layout

Default Screen

Sleep Timer
```

---

# 6. Playback settings

Examples:

```text id="c6m4x9"
Default Volume

Replay Mode

Shuffle State

Playback Position

Gapless Playback
```

---

# 7. Display settings

Examples:

```text id="r7m3x8"
Brightness

Display Timeout

Animation Level

Touch Calibration
```

---

# 8. Hardware settings

Examples:

```text id="n8m4x6"
GPIO Mapping

DSP Configuration

Display Driver

Audio Output
```

---

Hardware settings must be separated from user settings.

---

# 9. Storage format

Recommended:

```text id="w5m3x7"
JSON / TOML

+

SQLite Database
```

---

Use:

Configuration files:

* preferences
* system options

Database:

* large structured data

---

# 10. Configuration loading

Boot process:

```text id="a7m4x8"
Power On

↓

Load Defaults

↓

Load Configuration

↓

Validate

↓

Apply Settings

↓

Start UI
```

---

# 11. Default values

Every setting requires:

```text id="p6m8x2"
Default Value

Validation Rule

Fallback Value
```

---

Example:

```text id="u5m3x9"
Brightness

Default:

70%

Range:

0-100%

Fallback:

50%
```

---

# 12. Configuration validation

Before applying:

Check:

* value type
* allowed range
* compatibility

---

Invalid:

```text id="k8m4x5"
Reset To Safe Default
```

---

# 13. Runtime updates

When user changes settings:

Flow:

```text id="d7m3x8"
UI Change

↓

Configuration Manager

↓

Validate

↓

Save

↓

Notify Services
```

---

# 14. Saving strategy

Avoid constant writes.

Reason:

* SD card wear
* unnecessary IO

---

Recommended:

Delayed saving.

Example:

```text id="h5m8x2"
User Changes Setting

↓

Wait

↓

Save Batch
```

---

# 15. Critical data

Some data requires immediate saving:

Examples:

* playback position
* volume changes
* calibration data

---

# 16. Backup and recovery

Configuration corruption:

Recovery:

```text id="s6m4x9"
Detect Error

↓

Backup Old Config

↓

Generate Defaults

↓

Continue Startup
```

---

# 17. Version migration

Configuration should include:

```text id="b8m3x5"
Config Version
```

---

Example:

```text id="x7m4q2"
Version 1

↓

Migration

↓

Version 2
```

---

# 18. Security considerations

Configuration should prevent:

* invalid values
* accidental corruption
* unsafe hardware settings

---

No network-based configuration.

---

# 19. Performance requirements

Configuration operations must:

* avoid blocking UI
* minimize storage writes
* load quickly during boot

---

Targets:

Startup configuration load:

```text id="d8m4x6"
<1 second
```

---

# 20. Acceptance criteria

Implementation is complete when:

* settings persist
* invalid data recovers safely
* migrations work
* SD writes are minimized
* services receive updates correctly

---

# 21. Architectural recommendations

Required module:

```text id="m5x8q3"
Configuration Manager

├── Loader

├── Validator

├── Storage Adapter

├── Migration System

└── Change Dispatcher
```

---

# 22. Implementation notes for AI coding agent

When implementing:

* Never let screens edit config files directly.
* Validate every setting.
* Use defaults as recovery.
* Minimize SD writes.
* Keep configuration independent from UI code.

A stable configuration system ensures FLACHEAD behaves like a dedicated consumer device instead of a fragile development prototype.
