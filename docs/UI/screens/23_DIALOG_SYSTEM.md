# 23_DIALOG_SYSTEM.md

# FLACHEAD UI — Dialog System

## 1. Purpose

The Dialog System provides structured user confirmations, decisions, warnings, and configuration interactions that require more attention than a notification.

Unlike notifications, dialogs temporarily capture user focus.

Dialogs are used when:

* an irreversible action is requested
* user confirmation is required
* a setting requires choice
* an error requires a decision
* additional information must be displayed

The dialog system must remain lightweight and consistent with FLACHEAD's music-first philosophy.

Dialogs should feel like a premium DAP system interface, not a desktop operating system popup.

---

# 2. Design goals

The dialog system must:

* Be fast to open and close.
* Never interrupt playback unnecessarily.
* Support touchscreen and physical controls.
* Use predictable navigation.
* Avoid excessive confirmations.
* Work efficiently on Raspberry Pi Zero W.
* Maintain theme consistency.
* Keep layouts readable on a 2.8" portrait display.

Primary rule:

> If an action can safely happen automatically, do not create a dialog.

Dialogs should only exist when user attention is required.

---

# 3. Dialog architecture

Dialogs are part of the global overlay system.

They are not independent screens.

Architecture:

```text
Application
    |
    v
Screen Manager

    |
    v

Overlay Manager

    |
    +── Notification Overlay
    |
    +── Dialog Overlay
    |
    +── Volume Overlay
    |
    +── Brightness Overlay
```

A dialog temporarily receives input priority.

---

# 4. Dependencies

The Dialog System depends on:

* Overlay Manager
* Input Manager
* Theme Engine
* Configuration Service
* Navigation Manager
* Event Bus
* Localization System (future)
* Error State Manager

Recommended flow:

```text
System Event
      |
      v
Dialog Request
      |
      v
Dialog Manager
      |
      v
Overlay Renderer
      |
      v
User Input
      |
      v
Action Callback
```

---

# 5. Dialog categories

## 5.1 Confirmation Dialog

Used for actions requiring approval.

Examples:

* Delete playlist
* Remove track from library
* Factory reset
* Clear queue

Example:

```
Delete playlist?

"Favorites" will be permanently removed.

[CANCEL]      [DELETE]
```

---

## 5.2 Choice Dialog

Used when selecting between options.

Examples:

* Audio output device
* Sort method
* Playback mode

Example:

```
Playback Mode

○ Sequential
○ Shuffle
○ Repeat One

              [DONE]
```

---

## 5.3 Information Dialog

Used for important information that cannot fit in a notification.

Examples:

* About FLACHEAD
* Storage information
* Codec details

Example:

```
Track Information

Format:
FLAC 24-bit / 96kHz

Size:
120 MB

             [OK]
```

---

## 5.4 Error Recovery Dialog

Used when the user needs to decide what happens next.

Examples:

* Retry library scan
* Reconnect DAC
* Skip corrupted file

Example:

```
Playback Error

Unable to decode track.

[RETRY]   [SKIP]
```

---

# 6. Layout

Dialogs should occupy the center portion of the screen.

They should not always cover the entire display.

Preferred layout:

```
┌─────────────────────┐
│                     │
│   Current Screen    │
│                     │
│ ┌─────────────────┐ │
│ │ Dialog Title    │ │
│ │                 │ │
│ │ Message         │ │
│ │                 │ │
│ │ [NO]    [YES]   │ │
│ └─────────────────┘ │
│                     │
└─────────────────────┘
```

---

# 7. Dialog size rules

Because the display is small:

Maximum:

* 80-85% screen width
* 60% screen height

Content limits:

Title:

* 1 line

Description:

* maximum 3 lines

Buttons:

* maximum 3 actions

If more information is required:

* open a dedicated screen
* do not create scrolling dialogs

---

# 8. Background behavior

When a dialog appears:

Current screen remains visible behind a dim overlay.

Example:

```
Music Player
      +
Darkened overlay
      +
Dialog card
```

Rules:

* Playback continues unless explicitly stopped.
* Mini player remains active.
* Background animation should pause if unnecessary.
* Underlying screen should not receive input.

---

# 9. Interaction model

## Touch input

Supported:

* tap button
* tap outside to dismiss (only non-critical dialogs)
* scroll inside selection lists if required

---

## Hardware buttons

FLACHEAD must support physical controls.

Recommended mapping:

```
UP
 ↓
Move selection upward

DOWN
 ↓
Move selection downward

SELECT
 ↓
Confirm selected option

BACK
 ↓
Cancel / close dialog
```

---

# 10. Button behavior

Buttons should have clear states:

States:

* Normal
* Focused
* Pressed
* Disabled

Example:

```
[CANCEL]    [DELETE]
             ^
          selected
```

Default focus rules:

Destructive action:

* focus safe option

Example:

Delete dialog:

```
[CANCEL]    [DELETE]
 ^
default
```

---

# 11. Animation

Animations must be lightweight.

Opening:

* fade background
* scale dialog slightly

Duration:
100-180ms

Closing:

* fade out
* remove overlay

Duration:
100ms

Avoid:

* bouncing
* spring animations
* large transitions
* blur effects

---

# 12. Performance budget

Dialogs should be extremely cheap.

Requirements:

* Opening latency:
  <100ms

* Rendering:
  only update changed regions

* Memory:
  reuse dialog components

* Animations:
  maximum 180ms

Implementation recommendations:

* Cache dialog surfaces.
* Avoid rebuilding layouts every frame.
* Use shared components from UI library.
* Use dirty rectangle rendering.

---

# 13. Theme integration

Dialogs follow the active theme.

## Home/System

Use:

* wallpaper-derived colors
* system accent
* neutral readable surfaces

---

## Music Player

Use:

* album-art-derived accent
* dark/light contrast adaptation

---

## Third-party apps

Use:

* application theme if available
* fallback FLACHEAD theme

---

Important:

Dialog readability overrides theme styling.

Never sacrifice contrast for aesthetics.

---

# 14. Dialog stacking rules

Only one dialog should exist at a time.

Priority:

```
Critical Error Dialog
        |
Confirmation Dialog
        |
Choice Dialog
        |
Information Dialog
```

If another dialog request appears:

* queue it
* replace only if higher priority
* reject duplicate requests

Example:

Bad:

```
Delete playlist
     +
Storage warning
     +
DAC disconnected
```

Good:

```
DAC disconnected dialog

then

Storage warning notification
```

---

# 15. Common FLACHEAD dialogs

## Delete Queue

```
Clear queue?

All queued tracks will be removed.

[CANCEL] [CLEAR]
```

---

## Remove Track

```
Remove from library?

The file will remain on storage.

[CANCEL] [REMOVE]
```

---

## Shutdown Confirmation

```
Power off FLACHEAD?

[NO]     [YES]
```

---

## Factory Reset

```
Reset device?

All settings will be restored.

[CANCEL] [RESET]
```

---

# 16. Error handling

If dialog creation fails:

* do not crash application
* log failure
* fallback to notification
* maintain current screen state

If callback execution fails:

* close dialog safely
* report failure
* prevent UI lock

If invalid data is passed:

* show generic error
* discard invalid request

---

# 17. Acceptance criteria

The implementation is complete when:

* dialogs appear as global overlays
* playback continues correctly underneath
* hardware buttons work consistently
* touch interaction works
* dialogs respect themes
* only one dialog is active
* destructive actions require confirmation
* animations remain smooth on Pi Zero W
* dialogs never freeze the UI
* failed dialog actions recover safely

---

# 18. Future improvements

Possible additions:

* dialog history
* custom app dialogs
* keyboard input dialog
* text search dialog
* PIN/security dialogs
* advanced settings editor
* voice feedback support

---

# 19. Architectural recommendations

## Dialog Manager

Create a dedicated manager:

```text
Dialog Manager

├── create()
├── show()
├── close()
├── update()
├── handleInput()
├── executeAction()
└── queueDialog()
```

---

## Dialog definition model

Recommended structure:

```text
Dialog

id
type
title
message
buttons[]
default_button
priority
dismiss_behavior
callback
theme_override
```

---

## Component reuse

Dialogs should reuse:

* Button component
* Card component
* Text component
* List component

Do not create custom UI elements per dialog.

---

## Input ownership

When active:

```
Dialog
 |
 v
Input Manager

Screen input blocked
```

After closing:

```
Input Manager

returns control

to current screen
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Build dialogs as reusable UI components.
* Integrate with Overlay Manager.
* Never hard-code dialogs into individual screens.
* Support touch and hardware navigation.
* Keep rendering lightweight.
* Preserve music playback.
* Avoid mobile-style excessive confirmations.

The final result should feel like a carefully designed embedded audio device interface: clear, fast, and intentional.
