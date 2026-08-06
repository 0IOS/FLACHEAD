# 04_DSP_STATUS_UI.md

# FLACHEAD UI — DSP Status UI Specification

## 1. Purpose

This document defines how FLACHEAD represents the status of the external audio processing hardware.

FLACHEAD is designed around high-quality offline audio playback using the:

```text
TANCHJIM BUNNY DSP
```

The UI must communicate audio hardware status clearly while keeping playback uninterrupted.

Primary principle:

> Audio hardware problems should be visible, but they should never unnecessarily interrupt listening.

---

# 2. Design goals

DSP status handling must be:

* reliable
* informative
* non-intrusive
* playback-safe
* hardware-independent

---

# 3. DSP architecture

Data flow:

```text id="x7m4q9"
DSP Hardware

↓

Audio Device Manager

↓

DSP Status Service

↓

State Manager

↓

UI Components
```

---

The UI does not communicate directly with the DSP.

---

# 4. DSP states

Supported states:

```text id="m5x8q3"
Disconnected

Detecting

Initializing

Ready

Streaming

Error

Fallback Mode
```

---

# 5. Startup detection

During boot:

```text id="v8m3x5"
Initializing Audio

Detecting DSP...
```

---

Success:

```text id="c6m4x9"
Audio System Ready
```

---

Failure:

```text id="r7m3x8"
DSP Not Found

Using fallback output
```

---

# 6. Normal operation

When DSP is working:

The UI should not constantly display status.

---

Optional:

Small status icon.

Example:

```text id="n8m4x6"
♫ DSP
```

---

Reason:

The user cares about music, not hardware monitoring.

---

# 7. Player screen integration

Full player may show:

```text id="w5m3x7"
Audio Output

BUNNY DSP
```

---

Information:

* device name
* connection state
* output mode

---

Avoid:

Technical debug information.

---

# 8. DSP connection loss

Condition:

DSP disconnects during playback.

Behavior:

```text id="a7m4x8"
Detect Failure

↓

Switch Output

↓

Notify User

↓

Continue Playback
```

---

Notification:

```text id="p6m8x2"
Audio device disconnected

Fallback output enabled
```

---

# 9. Fallback audio mode

If DSP unavailable:

FLACHEAD continues using:

```text id="u5m3x9"
Default Audio Output
```

---

The user can continue:

* playback
* browsing
* settings

---

# 10. Reconnection behavior

When DSP returns:

Display:

```text id="k8m4x5"
Audio device connected
```

---

Do not:

* interrupt current track
* restart playback unnecessarily

---

# 11. Audio settings integration

Settings screen:

```text id="d7m3x8"
Settings

↓

Audio

↓

Output Device
```

---

Display:

```text id="h5m8x2"
Current:

TANCHJIM BUNNY DSP
```

---

# 12. Error states

Possible errors:

## Device unavailable

```text id="s6m4x9"
DSP unavailable
```

---

## Unsupported mode

```text id="b8m3x5"
Audio mode unavailable
```

---

## Communication error

```text id="x7m4q2"
Audio connection unstable
```

---

# 13. Logging

DSP events should be logged.

Example:

```text id="d8m4x6"
Audio Log

├── Connected

├── Disconnected

├── Errors

└── Recovery Attempts
```

---

Logs should remain lightweight.

---

# 14. Performance requirements

DSP monitoring must:

* use minimal CPU
* run asynchronously
* never affect playback timing

---

Audio priority:

```text id="m5x8q3"
Playback

↓

DSP Communication

↓

UI Update
```

---

# 15. Theme behavior

DSP status follows:

System theme.

---

Do not create a separate audio hardware theme.

---

# 16. Acceptance criteria

Implementation is complete when:

* DSP status is displayed correctly
* connection failures recover
* playback survives disconnects
* fallback mode works
* hardware monitoring does not affect performance

---

# 17. Architectural recommendations

Required modules:

```text id="v8m3x5"
Audio Device Manager

├── Detect Device

├── Monitor Status

├── Manage Output

└── Report Events
```

---

Required UI components:

```text id="c6m4x9"
AudioStatusBadge

DSPInfoCard

DeviceNotification
```

---

# 18. Implementation notes for AI coding agent

When implementing:

* Treat DSP as an optional enhancement, not a single point of failure.
* Keep audio playback independent from UI.
* Never restart playback because hardware status changed.
* Cache device information.
* Use asynchronous device monitoring.

The DSP integration should make FLACHEAD sound better, not make it fragile.
