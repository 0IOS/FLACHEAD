# 02_COMPONENT_IMPLEMENTATION_RULES.md

# FLACHEAD UI — Component Implementation Rules

## 1. Purpose

This document defines the rules for implementing FLACHEAD UI components.

Components are the smallest reusable building blocks of the interface.

Examples:

* buttons
* cards
* lists
* sliders
* progress indicators
* album artwork containers
* navigation elements

The goal is to ensure every component remains:

* reusable
* lightweight
* predictable
* easy to modify

Primary principle:

> Components describe how something behaves and renders, not why the application needs it.

---

# 2. Component architecture

Every component follows:

```text id="x7m4q9"
Component

├── Properties

├── State

├── Layout

├── Input Handling

├── Rendering

└── Cleanup
```

---

# 3. Component responsibilities

A component may:

* draw itself
* manage its internal visual state
* receive events
* expose callbacks

A component must not:

* control application flow
* access hardware directly
* modify global state
* communicate with unrelated systems

---

# 4. Component lifecycle

Every component follows:

```text id="m5x8q3"
Create

↓

Initialize

↓

Update

↓

Render

↓

Destroy
```

---

# 5. Component creation

Creation should:

* allocate required resources
* initialize default values
* register callbacks

---

Avoid:

* loading large assets
* database operations
* hardware access

during creation.

---

# 6. Component state

Component state contains only local information.

Examples:

Button:

```text id="v8m3x5"
Pressed

Hovered

Disabled
```

---

Slider:

```text id="c6m4x9"
Current Value

Dragging State
```

---

# 7. Layout rules

Components must support:

* fixed positioning
* dynamic sizing
* parent layouts

---

A component should not assume:

* screen resolution
* display size
* orientation

---

# 8. Rendering rules

Rendering must be:

* deterministic
* lightweight
* allocation-free

---

Every component exposes:

```text id="r7m3x8"
Render(Context)
```

---

The render function should only:

* draw
* update visuals

---

It should never:

* load files
* query databases
* perform heavy calculations

---

# 9. Input handling

Input flow:

```text id="n8m4x6"
Hardware Input

↓

Event System

↓

Component

↓

Component Response
```

---

Components receive processed input events.

---

# 10. Component events

Components may emit:

Examples:

```text id="w5m3x7"
BUTTON_CLICKED

VALUE_CHANGED

ITEM_SELECTED
```

---

Events go upward.

---

Components should not directly call parent functions.

---

# 11. Component hierarchy

Example:

```text id="a7m4x8"
Player Screen

├── Album Artwork

├── Track Information

├── Progress Bar

├── Controls

└── Volume Indicator
```

---

Parent controls:

* placement
* lifecycle

Child controls:

* own behavior

---

# 12. Reusability rules

Before creating a new component:

Check:

* does an existing component solve this?
* can it be extended?

---

Avoid duplicate components.

---

# 13. Styling rules

Components use:

```text id="p6m8x2"
Theme Values

+

Component Properties
```

---

Avoid:

Hardcoded colors.

Bad:

```text id="u5m3x9"
RGB(255,255,255)
```

---

Good:

```text id="k8m4x5"
Theme.TextPrimary
```

---

# 14. Animation rules

Animations must:

* be optional
* respect performance limits
* avoid blocking updates

---

Allowed:

* fades
* movement
* progress animations

---

Avoid:

* expensive effects
* particle systems
* heavy transformations

---

# 15. Resource rules

Components request resources through:

```text id="d7m3x8"
Resource Manager
```

---

Never:

```text id="h5m8x2"
Component

↓

Load File Directly
```

---

# 16. Component communication

Allowed:

```text id="s6m4x9"
Component

↓

Event

↓

Parent/System
```

---

Avoid:

```text id="b8m3x5"
Component A

↓

Directly Controls

↓

Component B
```

---

# 17. Performance rules

Components must:

* avoid unnecessary redraws
* reuse resources
* minimize allocations

---

Important for Pi Zero W:

A large number of small inefficient components can damage performance.

---

# 18. Error handling

Component failures should:

* fail gracefully
* display fallback state
* report errors

---

Example:

Missing image:

```text id="x7m4q2"
Display Placeholder
```

---

# 19. Testing requirements

Each component should test:

```text id="d8m4x6"
Rendering

Input

State Changes

Resize Behavior

Failure Cases
```

---

# 20. Acceptance criteria

A component is complete when:

* it follows lifecycle rules
* it is reusable
* it uses the theme system
* it communicates through events
* it performs correctly on target hardware

---

# 21. Implementation notes for AI coding agent

When implementing:

* Keep components small.
* Avoid putting business logic inside components.
* Prefer composition over inheritance.
* Use existing components whenever possible.
* Profile rendering cost on Raspberry Pi Zero W.

The component system is the foundation of FLACHEAD UI. A disciplined component architecture allows the interface to grow without becoming difficult to maintain.
