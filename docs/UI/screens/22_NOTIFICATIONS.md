# 22_NOTIFICATIONS.md

# FLACHEAD UI — Notifications

## 1. Purpose

The Notifications system provides a lightweight, non-disruptive way to surface system and app events without breaking the music-first experience of FLACHEAD.

Notifications are for short-lived, informational messages only:

* track synced
* library scan completed
* Bluetooth/audio device connected or disconnected
* charging state changed
* storage warning
* playback error summary
* theme applied
* settings saved

Notifications must not behave like full-screen alerts unless the event is critical. On a Pi Zero W, this layer must stay inexpensive to render, simple to animate, and easy to dismiss.

---

## 2. Design goals

* Preserve music playback and current screen context.
* Never interrupt playback unless the issue is critical.
* Keep visual footprint small.
* Use concise messaging.
* Allow queued notifications with strict limits.
* Support both passive auto-dismiss and user dismissal.
* Render as an overlay, not a separate screen.

Notifications should feel like a premium digital audio player system message, not a mobile OS toast clone.

---

## 3. Scope

This document covers:

* in-app notification banners
* transient toasts
* critical warning banners
* notification queue behavior
* notification priority and stacking
* interactions with overlays and playback UI

Out of scope:

* full modal dialogs
* confirmation flows
* long-form error pages
* push notification infrastructure
* remote messaging

Those belong in `23_DIALOG_SYSTEM.md` or `30_ERROR_STATES.md`.

---

## 4. Dependencies

Notifications depend on these subsystems:

* `Theme Engine` for styling and color selection
* `Configuration Service` for notification preferences
* `Playback Engine` for player events
* `Media Session Manager` for track/session updates
* `Library Database` for scan/index events
* `Metadata Service` for tag-related notices
* `Lyrics Engine` for lyric-fetch status
* `Storage Monitor` for disk warnings
* `Battery/Power Monitor` for charging and low power events
* `Overlay Manager` for rendering priority above screens

Recommended internal architecture:

```text
App / System Event
    ↓
Notification Service
    ↓
Priority Classifier
    ↓
Notification Queue
    ↓
Overlay Renderer
```

---

## 5. Notification types

### 5.1 Info

Low urgency, non-blocking.
Examples:

* “Library updated”
* “Saved”
* “Connected”

Behavior:

* auto-dismiss
* short duration
* minimal visual weight

### 5.2 Warning

Moderate urgency.
Examples:

* “Storage almost full”
* “No lyrics found”
* “Unsupported tag format”

Behavior:

* slightly longer visibility
* may include action affordance
* can be manually dismissed

### 5.3 Critical

High urgency, but still should avoid full-screen takeover unless necessary.
Examples:

* audio device disconnected
* playback failed
* storage unavailable
* corrupted media file detected

Behavior:

* persistent until acknowledged or recovered
* may escalate into dialog if user action is required

### 5.4 Success

Confirmation of completed actions.
Examples:

* “Theme applied”
* “Track added to queue”
* “Downloaded successfully”

Behavior:

* brief auto-dismiss
* subtle styling

---

## 6. Layout

Notifications should appear as compact overlays near the top edge of the portrait display, below any persistent status region if one exists.

### Preferred layout structure

```text
┌──────────────────────────┐
│ Status area / safe margin │
├──────────────────────────┤
│ Notification banner      │
│ ─ title                  │
│ ─ short message          │
│ ─ optional action text   │
├──────────────────────────┤
│ Current screen remains   │
│ visible underneath       │
└──────────────────────────┘
```

### Banner rules

* Use a compact height.
* Keep line count low.
* Limit to one primary message and one secondary line.
* Do not obstruct playback controls unless essential.
* Keep margins large enough for touch dismissal.

### Content priorities

1. Title or category label
2. Short message
3. Optional action hint
4. Optional icon

### Text limits

* Title: 1 line
* Message: 1–2 lines
* Action hint: 1 line max

If content exceeds limits, trim aggressively or route to a dialog instead.

---

## 7. Behavior and interaction

### 7.1 Auto-dismiss

Most notifications should disappear automatically after a short duration.

Recommended timing:

* Info: 1.5–2.5 seconds
* Success: 2–3 seconds
* Warning: 3–5 seconds
* Critical: persistent or until acknowledged

### 7.2 Manual dismiss

Users can dismiss notifications via:

* tap on notification
* swipe gesture if supported
* hardware back button only if mapped globally and no critical action is required

### 7.3 Tap action

Some notifications may support a primary action:

* open settings
* jump to playback
* view storage page
* retry scan

This should remain optional and rare.

### 7.4 Queue behavior

Notifications arriving while another is visible should be queued.

Rules:

* Only one visible notification at a time on this hardware target.
* Merge identical repeated messages within a short interval.
* Coalesce spammy events.
* Critical notifications may preempt informational ones.

---

## 8. Priority rules

Priority order:

1. Critical system error
2. Critical playback error
3. Storage / power warning
4. Actionable application warning
5. Success confirmation
6. Informational update

Priority handling rules:

* Higher priority can preempt lower priority.
* Lower priority must not overwrite an active critical banner.
* Repeated low-value events should be suppressed.
* If the queue is saturated, drop the oldest low-priority item first.

---

## 9. Visual style

Notifications must inherit theme behavior based on context:

### System and home context

* wallpaper-derived accents
* subtle contrast
* soft panel treatment

### Music context

* album-art-derived accents when safe
* avoid fighting the player theme
* keep readable against dynamic palettes

### Third-party app context

* respect the app theme if it provides one
* otherwise fall back to FLACHEAD system defaults

### Visual constraints

* no heavy blur
* no shadow stacking
* no complex translucency if it impacts FPS
* keep animation simple and inexpensive

Recommended styling:

* rounded container
* thin border or soft outline
* strong text contrast
* restrained iconography

---

## 10. Animation

Animation must be minimal and fast.

### Entry

* short slide/fade in
* duration: 120–180 ms

### Exit

* short fade or slide out
* duration: 100–150 ms

### Queue transition

* no dramatic motion
* replace or stack with minimal movement
* avoid reflow-heavy transitions

### Constraints

* No spring-heavy effects.
* No parallax.
* No overshoot that wastes frames.
* Never animate layout on the entire screen if only the overlay changes.

For Pi Zero W, the notification animation should be almost invisible as a system cost.

---

## 11. Rendering budget

Notifications are overlay UI and must be cheap to draw.

### Budget targets

* Overlay appearance: under 100 ms perceived latency
* Repaint area: only notification region
* No full-screen redraw for toast events
* GPU/CPU work minimized
* Avoid repeated text layout computation where possible

### Implementation notes

* Cache notification templates.
* Pre-measure common text sizes.
* Reuse icon surfaces.
* Use dirty-rect rendering.
* Avoid loading assets at notification time.

---

## 12. Input handling

Notifications should respond to global input first, then local overlay input.

### Supported input

* touch tap to dismiss
* global back button to dismiss non-critical notifications
* select key only if notification exposes an action
* no complex gestures required

### Input priority

If a notification is visible:

* taps inside the banner interact with the banner
* taps outside should fall through to the underlying screen unless the notification is modal-critical
* hardware back should dismiss only when safe

---

## 13. Examples

### Example 1: Library scan complete

Title: `Library updated`
Message: `143 tracks indexed`
Behavior: info, auto-dismiss

### Example 2: Storage warning

Title: `Storage low`
Message: `Less than 500 MB remaining`
Behavior: warning, persistent until dismissed

### Example 3: Audio device disconnected

Title: `Playback paused`
Message: `TANCHJIM BUNNY DSP disconnected`
Behavior: critical, persistent, actionable

### Example 4: Theme applied

Title: `Theme saved`
Message: `Wallpaper colors updated`
Behavior: success, auto-dismiss

---

## 14. Error handling

Notification generation must fail safely.

### If the notification service fails

* do not crash the app
* log the event
* fall back to a minimal system banner or internal log entry

### If assets are missing

* render text-only notification
* use generic icon placeholder if needed
* never block notification display because of missing artwork

### If queue memory is exhausted

* drop oldest low-priority item
* preserve critical events
* refuse new low-priority notifications before critical ones

### If the current screen is busy

* postpone informational notifications
* show critical notifications immediately unless a modal dialog is already active

---

## 15. Accessibility and usability

* Use large enough text for portrait 2.8" display.
* Maintain strong contrast.
* Avoid tiny secondary labels.
* Make dismissal touch targets easy to hit.
* Do not rely only on color to indicate severity.
* Use concise, plain wording.

---

## 16. Acceptance criteria

The implementation is acceptable when:

* notifications appear without interrupting music playback
* only one non-critical notification is visible at a time
* critical notifications persist until handled
* notifications use small, cheap overlays rather than full-screen interruptions
* messages are short and readable on a 2.8" portrait screen
* notification rendering does not noticeably degrade UI responsiveness
* repeated spammy events are coalesced or suppressed
* notification styling follows theme context correctly
* dismissal works via touch and safe global input
* the system remains stable under rapid event bursts

---

## 17. Future improvements

* grouped notification history
* swipe stack support
* action buttons for selected notifications
* richer event categories
* compact notification center in a future screen
* haptic feedback integration if hardware is added later
* per-app notification filters
* user-selectable quiet mode

---

## 18. Architectural recommendations

### 18.1 Notification service module

Create a dedicated service responsible for:

* event intake
* priority classification
* deduplication
* expiration handling
* queue management

### 18.2 Overlay manager integration

Notifications should be rendered through the global overlay manager, not as a separate screen. This ensures consistent stacking with:

* volume overlay
* brightness overlay
* dialogs
* error banners

### 18.3 Event bus

Use an internal event bus or message dispatcher so screens and services can emit notifications without tight coupling.

Recommended event flow:

```text
Subsystem Event
    ↓
Event Bus
    ↓
Notification Service
    ↓
Queue / Priority Logic
    ↓
Overlay Manager
    ↓
Renderer
```

### 18.4 Data model

A notification object should minimally contain:

* id
* type
* title
* message
* priority
* duration
* dismissible
* action label
* action callback reference
* source module
* timestamp

### 18.5 Performance-first rendering

* Keep notification templates reusable.
* Avoid dynamic layout calculations every frame.
* Cache iconography and color tokens.
* Render only when content changes.

---

## 19. Implementation notes for the AI coding agent

When implementing this screen/system:

* treat notifications as a global overlay subsystem
* keep the logic separate from screen code
* prioritize queuing, deduplication, and performance
* preserve playback continuity
* do not make notifications too verbose
* keep visual behavior consistent with the rest of FLACHEAD

The design should feel like a mature digital audio player operating system: calm, fast, and unobtrusive.
