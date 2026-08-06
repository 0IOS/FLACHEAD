# 29_LOCK_SCREEN.md

# FLACHEAD UI — Lock Screen

## 1. Purpose

The Lock Screen prevents accidental touch interactions while FLACHEAD is being carried, stored, or used as a dedicated music player.

Because FLACHEAD is designed around:

* physical controls
* pocket/portable usage
* continuous music playback

the lock system should protect playback from accidental touchscreen input without making the device inconvenient to use.

The lock screen is not a security feature.

It is an interaction protection layer.

Primary goal:

> Keep music playing while preventing unwanted commands.

---

# 2. Design goals

The Lock Screen must:

* Preserve playback.
* Block accidental touch input.
* Allow essential hardware controls.
* Provide quick unlock.
* Consume minimal resources.
* Work globally across FLACHEAD.

It should feel similar to a dedicated DAP hold switch, not a smartphone password screen.

---

# 3. Lock states

FLACHEAD has multiple lock states.

## 3.1 Unlocked

Normal operation.

Input:

* touchscreen enabled
* buttons enabled
* encoder enabled

---

## 3.2 Touch Locked

Most common mode.

Touchscreen disabled.

Hardware controls remain active.

Example:

```text
Music playing

↓

Touch Lock Enabled

↓

Buttons still work
Volume still works
```

Recommended default.

---

## 3.3 Full Lock

Optional.

All user input disabled except unlock action.

Used for:

* storage
* transport
* accidental button protection

---

# 4. Architecture

Lock Screen is managed by the system input layer.

It is not a normal application screen.

Architecture:

```text
User Input

      |

Input Manager

      |

Lock State Manager

      |

+----------------+

|                |

Unlocked       Locked

|                |

UI Events     Blocked Events

```

---

# 5. Dependencies

The Lock System depends on:

* Input Manager
* Overlay Manager
* Configuration Service
* Playback Engine
* Display Service
* Power Management Service

---

# 6. Lock activation methods

## 6.1 Manual activation

User actions:

Examples:

* Settings → Lock Screen
* Hardware shortcut
* Quick action menu

---

## 6.2 Automatic activation

Future support:

* screen timeout
* device inactivity
* pocket mode

---

## 6.3 Hardware switch

Future:

A physical hold switch could directly control:

```text
Hardware Lock

↓

Input Controller

↓

FLACHEAD Lock State
```

This would match the dedicated DAP philosophy.

---

# 7. Lock Screen layout

The Lock Screen should remain minimal.

Preferred:

```text
┌─────────────────────┐
│                     │
│                     │
│       🔒            │
│                     │
│   FLACHEAD LOCKED   │
│                     │
│   Tap to unlock     │
│                     │
└─────────────────────┘
```

---

# 8. Music information

Because FLACHEAD is music-first, the lock screen should optionally show:

* album artwork
* track title
* artist
* playback state

Example:

```text
┌─────────────────────┐
│                     │
│    Album Art        │
│                     │
│  Track Name         │
│  Artist             │
│                     │
│  ▶ 03:24            │
│                     │
│  LOCKED             │
└─────────────────────┘
```

---

# 9. Input behavior

## Touch locked mode

Touch events:

```text
Touch

↓

Input Manager

↓

Rejected

↓

Optional unlock detection
```

---

## Hardware controls

Recommended:

Remain active:

* Play/Pause
* Next
* Previous
* Volume encoder

This allows FLACHEAD operation without unlocking.

---

# 10. Unlock methods

Supported methods:

## Touch gesture

Example:

```text
Hold screen for 1 second

↓

Unlock
```

Avoid simple tap.

Reason:

Prevents accidental unlock.

---

## Hardware button

Example:

```text
Hold SELECT

↓

Unlock
```

---

## Combination shortcut

Future:

```text
BACK + SELECT

↓

Unlock
```

---

# 11. Animation

Lock transitions should be subtle.

Lock:

* fade overlay
* lock icon appears

Duration:

150ms

Unlock:

* fade away

Duration:

150ms

Avoid:

* security-style animations
* large transitions

---

# 12. Performance budget

The lock system should be almost free.

Requirements:

* no continuous heavy rendering
* no background animation
* minimal memory usage

When locked:

The device should reduce unnecessary UI work.

Possible:

```text
Locked

↓

Lower refresh rate

↓

Maintain playback
```

---

# 13. Power management integration

Lock mode can enable power saving.

Example:

```text
Locked

+

Screen inactive

        |

Power Manager

        |

Reduce refresh rate
```

---

# 14. Theme integration

Lock Screen belongs to system UI.

Uses:

* system theme
* wallpaper-derived colors

Not:

* third-party app themes

Music information may use:

* album-art-derived mini palette

---

# 15. Playback behavior

Locking must never:

* pause music
* stop queue
* reset player state

Playback continues normally.

Example:

```text
Playing Track

↓

Lock Enabled

↓

Track continues
```

---

# 16. Error handling

## Unlock failure

Behavior:

* remain locked
* keep playback active
* allow retry

---

## Input system failure

Fallback:

* enable last working input method
* show notification after recovery

---

## Corrupted lock state

Fallback:

```text
Default:

Unlocked
```

---

# 17. Acceptance criteria

Implementation is complete when:

* lock mode prevents accidental touch input
* playback continues
* hardware controls work correctly
* unlock is reliable
* lock state survives correctly when configured
* UI remains responsive
* power usage is reduced where possible
* no security assumptions are made

---

# 18. Future improvements

Possible additions:

* physical hold switch support
* gesture customization
* auto-lock timer
* pocket detection
* lock screen widgets
* always-on playback display mode
* hardware key remapping

---

# 19. Architectural recommendations

## Lock State Manager

Create:

```text
Lock State Manager

├── lock()
├── unlock()
├── isLocked()
├── setMode()
├── savePreference()
└── handleInput()
```

---

## Input filtering

Recommended:

```text
Input Event

        |

Input Manager

        |

Lock Filter

        |

Allowed Events

```

Example:

Allowed while locked:

```text
VolumeChanged
PlaybackNext
PlaybackPause
UnlockRequest
```

Blocked:

```text
OpenSettings
ChangeTheme
DeleteTrack
```

---

# 20. Implementation notes for AI coding agent

When implementing:

* Do not create Lock Screen as an isolated app.
* Integrate it into the input architecture.
* Keep playback independent from lock state.
* Preserve hardware DAP-like controls.
* Avoid smartphone security patterns.
* Optimize for low power operation.

The final result should feel like a real dedicated audio player with a reliable hold function: music continues, accidental touches are ignored, and the user remains in control.
