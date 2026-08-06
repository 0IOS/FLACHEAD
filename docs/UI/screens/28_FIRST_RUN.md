# 28_FIRST_RUN.md

# FLACHEAD UI — First Run Experience

## 1. Purpose

The First Run system defines the setup experience when FLACHEAD starts for the first time after installation, factory reset, or a fresh SD card deployment.

The goal is to transform a fresh Raspberry Pi Zero W installation into a ready-to-use dedicated music player with minimal configuration.

The experience should be:

* simple
* fast
* guided
* optional where possible
* designed around offline usage

FLACHEAD should not feel like configuring a Linux computer.

Primary goal:

> Get the user from first boot to playing music as quickly as possible.

---

# 2. Design goals

The First Run experience must:

* Work completely offline.
* Detect available hardware automatically.
* Configure essential settings only.
* Avoid overwhelming the user.
* Allow skipping non-essential steps.
* Support touchscreen and physical controls.
* Complete quickly on Raspberry Pi Zero W.

The setup should not become a long wizard.

---

# 3. First Run trigger conditions

First Run launches when:

* No user configuration exists.
* Installation is new.
* Factory reset was completed.
* Configuration database is missing.

State check:

```text id="p6kn4k"
Startup

↓

Configuration Service

↓

first_run_completed?

        |

        +---- YES → Normal Boot

        |

        +---- NO → First Run Wizard
```

---

# 4. Architecture

First Run is a system-level workflow.

Architecture:

```text id="zy2q3m"
First Boot Event

        |

        v

First Run Manager

        |

        +-------------+
                      |
                      v

             Setup Steps

        |
        +----------------+
        |                |
        v                v

 Hardware Setup     User Preferences

        |
        v

 Save Configuration

        |
        v

 Normal FLACHEAD Mode
```

---

# 5. Dependencies

The First Run system depends on:

* Configuration Service
* Hardware Detection Service
* Display Service
* Audio Manager
* Storage Manager
* Theme Engine
* Input Manager
* Library Database
* User Preference Manager

---

# 6. Setup flow

Recommended flow:

```text id="9k7t7s"
Welcome

↓

Display Check

↓

Audio Device Detection

↓

Storage Setup

↓

Theme Selection

↓

Music Library Setup

↓

Controls Setup

↓

Finish

↓

Home Screen
```

---

# 7. Step 1 — Welcome

Purpose:

Introduce FLACHEAD briefly.

Layout:

```text id="7f9p8h"
┌─────────────────────┐
│                     │
│      FLACHEAD       │
│                     │
│ Offline Music Player│
│                     │
│      [START]        │
│                     │
└─────────────────────┘
```

Content:

* FLACHEAD logo
* short description
* start button

Avoid:

* tutorials
* long explanations

---

# 8. Step 2 — Display setup

The system verifies:

* TFT display detected
* touch controller detected
* orientation

Example:

```text id="2r1k9j"
Display Ready

2.8" TFT detected

Touch:
Available

        [NEXT]
```

---

## Display calibration

Optional.

Only show if:

* touch coordinates are incorrect
* calibration data missing

Avoid forcing calibration every installation.

---

# 9. Step 3 — Audio setup

Critical for FLACHEAD.

Detect:

```text id="l4r7u6"
USB Audio Device

↓

TANCHJIM BUNNY DSP
```

Display:

```text id="m90qkp"
Audio Device Found

TANCHJIM BUNNY DSP

        [TEST]
```

---

## Audio test

Optional:

Play:

* short test tone
* sample track

Requirements:

* user-controlled
* low volume start
* safe output level

---

# 10. Step 4 — Storage setup

Detect:

* SD card
* music directory
* available space

Example:

```text id="4r5a5x"
Music Storage

SD Card Found

Available:
28 GB

[SCAN NOW]

[SCAN LATER]
```

---

# 11. Library initialization

Options:

## Scan now

Runs:

* metadata extraction
* database creation
* artwork indexing

---

## Scan later

Allows immediate usage.

Background scan begins afterward.

Recommended default:

Scan in background.

Reason:

Avoid blocking first experience.

---

# 12. Step 5 — Theme setup

FLACHEAD theme engine supports:

* wallpaper-derived colors
* album-art-derived player colors

Initial options:

```text id="8u7m4y"
Choose Theme

○ Dark System
○ Light System
○ Auto
```

---

Future:

* custom themes
* imported palettes

---

# 13. Step 6 — Controls setup

Configure:

* touchscreen
* buttons
* rotary encoder

Example:

```text id="0o7q4v"
Controls

Touch:
Detected ✓

Buttons:
Detected ✓

Encoder:
Detected ✓

        [NEXT]
```

---

# 14. Hardware button testing

Optional test screen:

```text id="m0x8kg"
Press:

UP
DOWN
SELECT
BACK

Waiting...
```

The system maps detected inputs.

---

# 15. Step 7 — Completion

Final screen:

```text id="gq1m8p"
FLACHEAD Ready

Music:
Ready

Audio:
Ready

Storage:
Ready


      [START]
```

---

# 16. Interaction model

First Run supports:

## Touch

* tap options
* swipe lists if needed

---

## Hardware controls

Mapping:

```text id="z7k4p1"
UP

Previous option


DOWN

Next option


SELECT

Confirm


BACK

Previous step
```

---

# 17. Skipping behavior

Non-critical steps can be skipped.

Allowed skips:

* theme selection
* library scan
* calibration

Cannot skip:

* basic configuration creation
* display initialization
* saving configuration

---

# 18. Error handling

## Audio device missing

Example:

```text id="q8p6w1"
Audio device not found

You can configure later.
```

Continue.

---

## Storage missing

Example:

```text id="a5j2sx"
No music storage found

Insert SD card later.
```

Continue.

---

## Touch unavailable

Fallback:

* hardware buttons

---

## Configuration save failure

Critical.

Behavior:

* retry
* create backup
* prevent incomplete completion state

---

# 19. Performance budget

First Run must remain lightweight.

Requirements:

* no heavy animations
* lazy loading
* simple screens
* minimal database operations

Target:

Complete setup:

<5 minutes

---

# 20. Persistent data created

After completion:

Configuration Service stores:

```text id="r2m7si"
User Configuration

first_run_completed

theme_settings

display_settings

audio_settings

control_mapping

library_preferences
```

---

# 21. Acceptance criteria

Implementation is complete when:

* new installations enter First Run automatically
* setup works offline
* TANCHJIM BUNNY DSP detection works
* touchscreen works
* hardware controls work
* setup can be skipped where appropriate
* configuration persists after reboot
* user reaches music interface quickly
* errors do not prevent recovery

---

# 22. Future improvements

Possible additions:

* QR-based configuration import
* backup/restore settings
* guided audio calibration
* automatic DSP profiles
* migration from previous SD cards
* setup animations

---

# 23. Architectural recommendations

## First Run Manager

Create:

```text id="v3d5bm"
First Run Manager

├── checkRequired()
├── startWizard()
├── nextStep()
├── previousStep()
├── saveProgress()
├── complete()
└── recoverFailure()
```

---

## Setup State Model

Example:

```text id="4x0n2n"
First Run State

current_step

display_ready

audio_ready

storage_ready

controls_ready

completed
```

---

## Implementation notes for AI coding agent

When implementing:

* Keep First Run separate from normal navigation.
* Use existing UI components.
* Do not duplicate settings screens.
* Save progress safely.
* Support recovery after interrupted setup.
* Keep the workflow short.

The final experience should feel like opening a premium dedicated music player for the first time: simple, fast, and ready for music.
