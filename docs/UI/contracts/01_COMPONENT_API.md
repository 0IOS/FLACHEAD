# 01_COMPONENT_API.md

# FLACHEAD UI — Component API Contract

## 1. Purpose

This document defines the implementation contract for all FLACHEAD UI components.

Components are the smallest reusable building blocks of the interface.

Examples:

* Buttons
* Cards
* Lists
* Sliders
* Progress bars
* Text elements
* Album artwork containers

Components provide:

* rendering
* local interaction
* layout behavior

Components do not contain:

* application logic
* audio logic
* navigation logic
* database access

Primary principle:

> Components display state; they do not own system behavior.

---

# 2. Component architecture

Every component follows the same lifecycle.

```text id="c1m8q4"
Component

 |

 +-- State

 |

 +-- Layout

 |

 +-- Render

 |

 +-- Input Handling

 |

 +-- Cleanup
```

---

# 3. Base component interface

Every component should implement:

```cpp id="q7m3x9"
class Component
{

public:

    virtual void create();

    virtual void update();

    virtual void render();

    virtual void handleInput();

    virtual void destroy();

};
```

---

# 4. Component lifecycle

Lifecycle:

```text id="p8x4m2"
Created

↓

Initialized

↓

Visible

↓

Updated

↓

Hidden

↓

Destroyed
```

---

Methods:

## create()

Called once.

Used for:

* initialization
* loading resources
* preparing state

---

## update()

Called when state changes.

Used for:

* updating internal values
* animations

Avoid:

* heavy processing

---

## render()

Called by renderer.

Responsible for:

* drawing itself

Must not:

* modify global state
* trigger services

---

## handleInput()

Receives:

* touch events
* navigation events

Returns:

```text id="m5q9x3"
Consumed

or

Ignored
```

---

## destroy()

Releases:

* memory
* resources
* temporary assets

---

# 5. Component state

Components may have local state.

Example:

Button:

```text id="w8k2p6"
pressed

hovered

enabled
```

---

Components cannot own:

```text id="n3m7q5"
Current Song

Volume

Settings

Library Data
```

---

# 6. Component properties

Components receive configuration.

Example:

```text id="v4x9m1"
Button

text

icon

position

size

style

enabled
```

---

Properties should be:

* simple
* serializable
* reusable

---

# 7. Layout system

Components use calculated layout.

Example:

```text id="j6m2x8"
Component

position:

x

y


size:

width

height
```

---

The component does not decide:

* screen placement
* navigation structure

---

# 8. Rendering rules

Components must:

* render only themselves
* use theme tokens
* respect visibility

---

Bad:

```text id="a9q5m3"
Button

changes application theme
```

---

Good:

```text id="k7x2p8"
Button

requests

theme.accent
```

---

# 9. Input handling

Input priority:

```text id="f8m4q2"
Global Controls

↓

Screen

↓

Component
```

---

Example:

Volume button:

Handled globally.

Music list:

Handled by component.

---

# 10. Component communication

Components communicate through:

* events
* callbacks
* state updates

Avoid:

```text id="s3n8m5"
Button

directly calls

Playback Engine
```

---

Correct:

```text id="h7q2x9"
Button

↓

Event

↓

Media Session
```

---

# 11. Common component types

## Button

Used for:

* actions
* navigation
* controls

Properties:

```text id="z5m1q8"
label

icon

action

enabled
```

---

## Card

Used for:

* grouped information

Examples:

* album card
* settings card

---

## List

Used for:

* libraries
* queues
* menus

Requirements:

* virtualization support
* scrolling optimization

---

## Slider

Used for:

* volume
* brightness
* progress

---

## Text

Used for:

* titles
* labels
* metadata

Requirements:

* font caching
* truncation

---

# 12. Component performance rules

Components must:

* avoid allocations during rendering
* cache resources
* update only when needed

---

Avoid:

```text id="m6x8p4"
Every frame:

Create text surface
```

---

Prefer:

```text id="q9n3m7"
Text changed

↓

Regenerate surface
```

---

# 13. Component memory rules

Components should release:

* textures
* surfaces
* temporary buffers

when unused.

---

Long-lived components:

* navigation bar
* mini player
* system controls

may keep cached resources.

---

# 14. Accessibility considerations

Even on a small device:

Support:

* readable text sizes
* clear contrast
* large enough touch targets

Recommended:

Minimum touch target:

~40px

---

# 15. Error handling

Component failure should not crash UI.

Example:

Missing icon:

```text id="u2m8x5"
Show fallback icon
```

---

Invalid data:

```text id="b5q9m1"
Show placeholder
```

---

# 16. Testing requirements

Each component should test:

* rendering
* input handling
* resizing
* missing data
* theme changes

---

# 17. Acceptance criteria

A component implementation is complete when:

* follows lifecycle
* has no business logic
* uses theme tokens
* handles input correctly
* cleans resources
* performs well on Pi Zero W

---

# 18. Architectural recommendations

Base class:

```text id="x8m3q6"
Component

├── create()

├── update()

├── render()

├── handleInput()

├── setVisible()

├── setPosition()

└── destroy()
```

---

Component registry:

```text id="p4n7m2"
Component Registry

├── register()

├── create()

├── destroy()

└── cache()
```

---

# 19. Implementation notes for AI coding agent

When implementing:

* Keep components dumb.
* Put logic in services.
* Reuse existing components.
* Avoid screen-specific components unless necessary.
* Optimize every component for Raspberry Pi Zero W.
* Never allow one component failure to crash FLACHEAD.

Components should behave like reliable hardware modules: simple, reusable, and predictable.
