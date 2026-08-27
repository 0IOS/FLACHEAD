# FLACHEAD AUDIO LYRICS ARCHITECTURE

Document

09_LYRICS.md

Category

Audio System

Status

Production Specification

Target Hardware

- Raspberry Pi Zero W
- ARMv6
- Raspberry Pi OS
- 2.8" portrait TFT touchscreen
- TANCHJIM BUNNY DSP
- Offline FLAC playback

---

# 1. PURPOSE

This document defines the architecture for lyrics handling in FLACHEAD. It defines how FLACHEAD loads, parses, stores, synchronizes, displays, and updates lyrics against actual audio playback.

The lyrics system is responsible for:

- discovering lyrics
- loading lyrics
- parsing lyrics
- supporting synchronized lyrics
- supporting unsynchronized lyrics
- normalizing lyric timestamps
- associating lyrics with tracks
- caching lyrics
- exposing lyrics to the UI
- synchronizing displayed lyrics with playback
- handling seeking
- handling pause/resume
- handling track transitions
- handling missing or malformed lyrics
- keeping lyric processing independent from audio decoding
- performance constraints

The primary objective is:

PROVIDE RELIABLE OFFLINE LYRICS SYNCHRONIZATION WITHOUT ADDING WORK TO THE AUDIO THREAD OR MAKING PLAYBACK DEPEND ON LYRICS.

---

# 2. CORE PRINCIPLE

Lyrics are:

TIMED PRESENTATION DATA.

Lyrics are not:

- AUDIO DATA
- the source of playback timing
- audio decoding
- playback timing
- queue order
- audio output

The playback engine is authoritative for time.

Lyrics are a:

PLAYBACK-SYNCHRONIZED PRESENTATION LAYER.

The audio engine remains authoritative.

---

# 3. SOURCE OF TRUTH

The authoritative timing source is:

AUDIO PLAYBACK POSITION.

Not:

- UI animation time
- wall-clock time
- lyric parser time
- the moment a lyric line was displayed
- lyrics timer time

---

# 4. ARCHITECTURAL RELATIONSHIP

The intended flow is:

                         TRACK
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
       METADATA / FILE            LYRICS SOURCE
                                        │
                                        ▼
                                LYRICS DISCOVERY
                                        │
                                        ▼
                                LYRICS LOADER
                                        │
                                        ▼
                                LYRICS PARSER
                                        │
                                        ▼
                            NORMALIZED LYRIC MODEL
                                        │
                                        ▼
                                 LYRICS CACHE
                                        │
                                        ▼
                            SYNCHRONIZATION ENGINE
                                        │
                              PLAYBACK POSITION
                                        │
                                        ▼
                                       UI

The lyrics system observes playback.

It does not drive playback.

---

# 5. LYRICS OWNERSHIP

The lyrics subsystem owns:

- lyrics discovery
- lyrics loading
- lyrics parsing
- normalized lyric representation
- timestamp normalization
- lyric indexing
- lyric caching
- synchronization calculations
- current-line determination
- synchronization lookup

The playback engine owns:

- actual playback position
- pause/resume state
- seeking
- track transitions
- playback timing source
- queue ordering
- track selection

The UI owns:

- lyric rendering
- scrolling
- highlighting
- empty/error states
- font
- layout
- wrapping
- animation

---

# 6. LYRICS DOES NOT OWN

Lyrics does not own:

- FLAC decoding
- PCM buffering
- audio output
- audio decoding
- queue management
- playlist management
- artwork rendering
- playback timing source
- queue ordering
- track selection

---

# 7. OFFLINE-FIRST

Lyrics must work without:

- internet access
- remote APIs
- cloud accounts
- network services

No network request should be required during playback.

---

# 8. NETWORK LYRICS

Online lyrics retrieval may be considered a future feature.

If added later:

- network retrieval must be a separate service
- they must be optional
- playback must never depend on them

Network lyric failure must not affect local lyrics or playback.

No network lyric functionality should be silently enabled by the base offline player.

---

# 9. LOCAL LYRICS

Initial implementation should prioritize locally available lyrics.

Possible sources include:

- external `.lrc` files
- embedded lyric metadata where supported
- application-managed lyric files

---

# 10. LYRIC TYPES

FLACHEAD should conceptually support:

1. Synchronized lyrics
2. Unsynchronized lyrics

Synchronized lyrics contain timestamps.

Unsynchronized lyrics contain text without timing information.

The normalized model should distinguish:

SYNCED

and:

UNSYNCED.

---

# 11. EMBEDDED LYRICS

Lyrics may exist inside the audio file's metadata.

The metadata system should expose them to the lyrics subsystem without forcing the UI to parse the FLAC file.

Embedded lyrics have the strongest direct association with the audio file.

They should generally be preferred.

---

# 12. EXTERNAL LYRICS

External lyrics may be stored beside the audio file.

Common formats may include:

- `.lrc`
- `.txt`

The architecture should be extensible to other formats.

---

# 13. LYRIC TYPES

FLACHEAD may support:

1. embedded lyrics
2. external lyric files
3. manually imported lyrics
4. future online retrieval, if ever added

The initial architecture must work fully offline.

---

# 14. LRC

LRC is the primary synchronized external lyric format.

LRC lyrics associate text with timestamps.

Conceptually:

timestamp → lyric line.

Example conceptual structure:

[00:12.50]First lyric line

[00:16.20]Second lyric line

The parser converts timestamps into internal time values.

---

# 15. UNSYNCHRONIZED LYRICS

Plain `.txt` lyrics may contain no timestamps.

These are valid lyrics but cannot provide automatic line synchronization.

Unsynchronized lyrics are simply ordered text lines.

They cannot reliably determine which line is currently being sung.

Therefore:

do not invent timing.

---

# 16. SYNCED LYRIC MODEL

A synchronized lyric line should contain at minimum:

- timestamp
- text
- sequence/order
- index

---

# 17. LYRICS MODEL

Conceptual model:

LyricsDocument

    track_id
    source
    language
    synchronization
    lines
    metadata
    signature

---

# 18. LYRIC LINE

Conceptual model:

LyricLine

    timestamp
    start_time
    end_time
    text
    index

---

# 19. START TIME

Each synchronized lyric line has a start timestamp.

---

# 20. END TIME

End time may be explicitly provided or inferred.

Recommended initial behavior:

infer the end of a line from the next line's start time.

---

# 21. LAST LINE

The final lyric line may remain active until:

- the track ends
- lyrics are closed
- a later explicit end timestamp exists

After the last timestamp:

the last line may remain active until:

track completion

or:

a configured end threshold.

---

# 22. TIMESTAMP UNIT

Internally, timestamps should use a consistent high-resolution unit.

Recommended:

milliseconds.

Avoid floating-point timestamps for synchronization logic.

---

# 23. PREFERRED TIMING REPRESENTATION

Because audio playback is frame-based:

the preferred internal representation is:

timestamp_frames.

This allows direct comparison with:

current_audio_frame.

---

# 24. TIMESTAMP CONVERSION

For a lyric timestamp:

timestamp_seconds

convert using:

timestamp_frames =
timestamp_seconds × sample_rate

The conversion must use a defined rounding policy.

---

# 25. SAMPLE RATE CHANGE

Lyric timing must remain logically time-based.

If output sample rate changes between tracks:

the lyric document must be interpreted against the track's actual timeline.

---

# 26. LYRIC TIMESTAMP PRECISION

Typical lyric timestamps may have:

centisecond precision.

The audio engine may operate at:

sample-level precision.

Lyrics therefore do not need sample-perfect timing.

---

# 27. INTERNAL TIME

The lyrics subsystem must not use floating-point seconds unnecessarily.

Prefer integer-based timing where practical.

Use:

integer audio frames

or:

integer microseconds

internally.

---

# 28. PARSER PRECISION

If an LRC timestamp contains:

minutes

seconds

hundredths

convert it to milliseconds.

---

# 29. TIMESTAMP PARSING

The parser must reject malformed timestamps safely.

---

# 30. INVALID TIMESTAMP

An invalid lyric timestamp must not crash playback.

Options:

- ignore malformed timestamp
- retain text as unsynchronized
- record warning.

The affected line may be discarded or treated as unsynchronized according to parser policy.

The chosen behavior must be deterministic.

---

# 31. INVALID ORDER

Lyric timestamps may not be sorted.

The parser should normalize them into ascending order.

---

# 32. DUPLICATE TIMESTAMPS

Multiple lyric lines may share the same timestamp.

The parser must preserve them.

For identical timestamps:

preserve source order.

The system should preserve deterministic ordering.

---

# 33. MULTIPLE TIMESTAMPS

LRC may associate multiple timestamps with one line.

Example conceptually:

[00:10.00][00:20.00]Repeated lyric.

The parser should create separate timed lyric entries.

This should become two timed entries pointing to the same text.

---

# 34. METADATA TAGS

LRC files may contain metadata tags such as:

- artist
- title
- album
- author
- offset

These should not override the library's authoritative track metadata.

Metadata should be parsed separately from lyric lines.

---

# 35. LRC OFFSET

Some lyric formats support a global timing offset.

LRC may define a global timing offset.

If supported:

apply it during normalization.

Do not modify raw source text.

Apply the offset exactly once.

Do not repeatedly add it during synchronization.

Conceptually:

effective_timestamp =
source_timestamp + lyric_offset.

Clamp or validate according to policy.

---

# 36. NEGATIVE OFFSET

Negative offsets may move a lyric line earlier.

The resulting timestamp must not produce unsafe arithmetic.

---

# 37. NEGATIVE TIMESTAMPS

After applying offsets:

timestamps before zero should be clamped or discarded according to the normalization policy.

They must not cause unsigned integer underflow.

---

# 38. DISPLAY PRECISION

The UI should not visually update at audio-frame frequency.

Lyric state updates should occur only when necessary.

The UI may display lyric timing with appropriate precision if debugging tools require it.

Normal user-facing display does not need millisecond precision.

---

# 39. NORMALIZATION

Lyrics should undergo minimal normalization.

Safe operations may include:

- newline normalization
- removal of parser-only timestamp markers
- trimming accidental line-ending artifacts.

---

# 40. DO NOT MODIFY LYRICS TEXT

Do not automatically rewrite:

- punctuation
- capitalization
- spelling
- intentional whitespace.

---

# 41. EMPTY LINES

Empty timed lines may represent:

- intentional pauses
- spacing
- formatting.

Empty lyric lines may be preserved where they affect timing or presentation.

Do not automatically discard them without a policy.

However:

excessive empty lines should not create unnecessary synchronization work.

---

# 42. WHITESPACE

Leading/trailing whitespace may be normalized for display.

Internal spacing should generally be preserved.

---

# 43. UNICODE

Lyrics may contain:

- Unicode scripts
- accented characters
- emoji
- punctuation
- symbols

The system must support UTF-8 safely.

Lyrics must support Unicode.

---

# 44. MULTILINGUAL LYRICS

The system must not assume:

English

or:

Latin characters.

Lyrics may contain Unicode and multiple languages.

The parser must preserve Unicode text.

---

# 45. ENCODING

UTF-8 should be the preferred internal representation.

If another encoding is encountered:

the implementation may use safe detection/fallback.

---

# 46. ENCODING DETECTION

The implementation may support safe detection/conversion of common encodings.

---

# 47. INVALID UTF-8

Invalid sequences must not crash the application.

They should be replaced or safely discarded according to the text-decoding policy.

---

# 48. INVALID TEXT

Malformed text should never crash:

- parser
- library
- UI
- playback

---

# 49. LONG LINES

Very long lyric lines should be handled without allocating unbounded memory.

The UI may wrap or truncate them visually.

The underlying lyric text should remain intact within configured safety limits.

---

# 50. HUGE LYRIC FILE

A pathological lyric file must not consume the entire Pi Zero W memory budget.

The parser should use reasonable limits.

---

# 51. MATCHING ALGORITHM

For current playback frame:

find the latest lyric line whose timestamp is:

<= current_frame.

That line is the active lyric line.

---

# 52. EXAMPLE

Given:

10.0 s → Line A

15.0 s → Line B

20.0 s → Line C

At:

14.0 s

active line:

A.

At:

17.0 s

active line:

B.

At:

25.0 s

active line:

C.

---

# 53. BEFORE FIRST TIMESTAMP

If playback is before the first lyric timestamp:

there is no active synchronized lyric line.

The UI may display:

- blank state
- upcoming first line
- configured placeholder

but must not pretend the first line is already active.

---

# 54. AFTER LAST TIMESTAMP

After the last timestamp:

the last line may remain active until:

track completion

or:

a configured end threshold.

---

# 55. ACTIVE LINE INDEX

The lyric matcher should maintain:

current_line_index.

This avoids searching the entire lyric list on every UI frame.

---

# 56. FORWARD PLAYBACK

During normal playback:

if current_frame moves forward:

advance the lyric index when the next timestamp is reached.

---

# 57. CURRENT LINE LOOKUP

For sorted timestamps:

the engine should efficiently locate the greatest timestamp less than or equal to the current playback position.

---

# 58. MONOTONIC PLAYBACK

During normal forward playback:

the current line generally advances monotonically.

---

# 59. BINARY SEARCH

After a seek or large position jump:

binary search may efficiently locate the active lyric line.

This is preferable to scanning thousands of lyric entries sequentially.

For large lyric documents:

binary search is appropriate.

---

# 60. NORMAL PLAYBACK SEARCH

During ordinary playback:

incremental advancement is preferable.

The engine may maintain the current line index and advance forward instead of performing a full binary search every update.

---

# 61. SEEK SEARCH

After a seek:

perform a direct indexed lookup.

Do not assume the old lyric index remains correct.

---

# 62. SMALL DOCUMENTS

For very small lyric files:

linear search may be sufficient.

The implementation should prioritize simplicity unless profiling shows a need for optimization.

---

# 63. TRACK ASSOCIATION

Lyric requests should be associated with a stable:

track_id

Do not identify lyrics only by:

- title
- artist
- filename

---

# 64. LYRIC GENERATION

Lyrics should be associated with the same:

track_id

and:

playback generation

as the active track.

---

# 65. WHY GENERATIONS MATTER

Example:

Track A lyrics are loaded.

User presses NEXT.

Track B becomes active.

A delayed lyric event from Track A must not overwrite Track B's lyrics.

Generation checks prevent this.

---

# 66. REQUEST GENERATION

Asynchronous lyric requests should use a request generation or equivalent cancellation mechanism.

This prevents delayed position events from the previous track from updating the new track's lyrics.

---

# 67. TRACK CHANGE

On track change:

1. invalidate old lyric state
2. load/select new lyric document
3. reset active line
4. associate lyrics with new track ID
5. associate lyrics with new generation
6. wait for playback position
7. begin synchronization

---

# 68. TRACK CHANGE ORDER

Recommended:

new track selected

↓

playback generation increments

↓

old lyric context invalidated

↓

new audio opens

↓

new lyric discovery begins

↓

new lyrics become active when ready.

---

# 69. SEEK

On seek:

do not reload the lyric file unnecessarily.

Instead:

reposition the lyric matcher.

---

# 70. SEEK MATCHING

After a seek:

current_line_index =
latest line timestamp <= new_position.

---

# 71. BACKWARD SEEK

If playback jumps backward:

the lyric index must also move backward.

Do not only support forward advancement.

---

# 72. FORWARD SEEK

If playback jumps forward:

skip all lyric lines whose timestamps are before the new position.

Never animate through every skipped lyric line after a large seek.

Jump directly to the correct active line.

---

# 73. RAPID SEEK

Rapid seek operations should use the latest authoritative playback position.

Obsolete lyric updates must be discarded.

---

# 74. SEEK ORDER

Recommended sequence:

audio seek requested

↓

audio position changes

↓

playback generation/position state updates

↓

lyrics engine recalculates active line

↓

UI updates.

---

# 75. PAUSE

On pause:

the active lyric line remains associated with the paused position.

The lyric state should not continue advancing.

---

# 76. PAUSE ORDER

Recommended:

audio paused

↓

playback state updates

↓

lyrics stops advancing.

---

# 77. PAUSE CLOCK

The lyric system must not use an independent timer that continues running while audio is paused.

---

# 78. RESUME

On resume:

lyrics continue from the new authoritative audio position.

---

# 79. RESUME ORDER

Recommended:

audio resumes

↓

playback clock advances

↓

lyrics synchronization continues.

---

# 80. AUDIO POSITION

The lyric engine should receive position updates from:

the playback/audio timing subsystem.

---

# 81. POSITION UPDATE FREQUENCY

The audio engine may expose position updates at a moderate frequency.

For example:

10–30 updates per second

is generally sufficient for lyric display.

The exact value should be benchmarked.

Approximately:

20–30 synchronization evaluations per second.

The exact rate may be adjusted for performance.

---

# 82. AUDIO BUFFER EFFECT

The logical decoder position may be ahead of what is audible.

Therefore lyric timing should use:

audible playback position

rather than:

decoder decode position.

---

# 83. ALSA QUEUE EFFECT

If ALSA has queued audio:

the decoder may have already produced future PCM.

Lyrics must not use:

decoder progress

as the displayed position.

---

# 84. AUTHORITATIVE POSITION

The playback timing architecture must expose a position that accounts for output buffering as accurately as practical.

Lyrics should use the most accurate playback position exposed by the playback engine.

---

# 85. DSP LATENCY

The TANCHJIM BUNNY DSP may introduce output latency.

The architecture should allow a configurable synchronization offset.

---

# 86. SYNC OFFSET

Conceptually:

display_position = audio_position + lyrics_offset.

---

# 87. OFFSET RANGE

The offset may be positive or negative.

---

# 88. DEFAULT OFFSET

Default:

0 ms.

---

# 89. USER CALIBRATION

A future setting may allow the user to adjust lyric timing.

---

# 90. OFFSET PERSISTENCE

If implemented:

the offset may be stored globally or per track/source.

---

# 91. DO NOT MODIFY AUDIO TIMING

Lyrics offset affects display only.

It must never alter:

decoder timing

or:

audio output.

---

# 92. SEEK RE-ANCHOR

After a seek:

lyrics must immediately re-anchor to:

new audible timeline.

---

# 93. PAUSE RE-ANCHOR

After pause:

the lyric state must stop advancing.

---

# 94. RESUME RE-ANCHOR

After resume:

lyric updates continue from:

actual audio position.

---

# 95. TRACK START

At track start:

active lyric index should be reset.

Audio playback may begin immediately.

Lyrics can load independently.

The matcher should determine the correct initial line based on:

actual start position.

---

# 96. RESUME START

If playback resumes from a saved position:

lyrics must immediately jump to the corresponding line.

---

# 97. CROSSFADE

If crossfade is ever implemented:

lyrics must remain associated with the currently foreground track.

Do not merge two lyric timelines into one ambiguous state.

Crossfade is outside the initial architecture.

---

# 98. GAPLESS TRANSITION

For gapless transitions:

Track A lyrics must end with Track A.

Track B lyrics must begin with Track B.

The generation/track identity boundary remains authoritative even if there is no audible silence.

---

# 99. CLOCK MONOTONICITY

Synchronization should use a monotonic playback timeline.

Do not depend on wall-clock time.

---

# 100. SYSTEM CLOCK CHANGES

Changing the system clock must not affect lyric synchronization.

---

# 101. SLEEP / RESUME

If the system sleeps or playback pauses:

lyrics must resume from actual playback position.

---

# 102. BUFFER UNDERRUN

An audio underrun should not cause lyrics to advance according to wall-clock time.

The playback position remains authoritative.

---

# 103. PLAYBACK ERROR

On playback error:

freeze/reset lyrics according to the player state.

Do not continue advancing lyric state independently.

---

# 104. TRACK END

When playback naturally reaches the end:

lyrics may remain on the final line briefly according to UI behavior.

Once the next track starts:

the old lyrics must be cleared.

---

# 105. NO AUDIO THREAD UI WORK

The audio output thread should NOT directly render lyrics.

---

# 106. AUDIO THREAD RESPONSIBILITY

The audio system provides:

current playback position.

The lyric system consumes that position.

The audio thread should only expose the current playback position.

It should not parse or render lyrics.

---

# 107. LYRIC THREAD RESPONSIBILITY

A lyric subsystem may:

- parse lyrics
- build indexes
- match current position
- publish lyric state

It should not block audio output.

---

# 108. UI RESPONSIBILITY

The UI:

- receives active lyric state
- renders it
- handles scrolling/visual presentation.

---

# 109. THREADING

Recommended separation:

MAIN/UI THREAD

- rendering
- user scrolling
- lyric view interaction
- font selection
- layout
- wrapping

LYRICS WORKER

- file discovery
- parsing
- cache operations
- synchronization calculations

PLAYBACK CONTROL

- authoritative position/state

AUDIO OUTPUT THREAD

- audio only

---

# 110. AUDIO THREAD RESTRICTION

The audio output thread must never:

- parse LRC
- read lyric files
- allocate large strings
- perform database queries
- perform lyric lookup or text layout

---

# 111. UI THREAD RESTRICTION

The UI thread must not synchronously parse a large lyric file.

---

# 112. THREAD COMMUNICATION

Use a safe shared state or message mechanism for:

playback position

and:

track generation.

---

# 113. DATA RACE

The implementation must prevent races between:

track change

seek

pause

lyrics update.

---

# 114. PLAYBACK GENERATION

A playback generation counter is recommended.

Example:

Track A:

generation 42

Track B:

generation 43.

---

# 115. WHY GENERATIONS

Generation numbers make stale asynchronous results easy to reject.

---

# 116. POSITION EVENT

A lightweight position event can be sent to the lyrics controller.

---

# 117. EVENT COALESCING

If position updates arrive faster than the UI can consume:

coalesce them.

Only the newest relevant position is necessary.

---

# 118. SEEK EVENT

Seek should trigger an immediate lyric recalculation.

---

# 119. TRACK CHANGE EVENT

Track change should invalidate the previous lyrics state immediately.

---

# 120. LYRICS LOAD EVENT

After lyrics parsing completes:

publish:

LYRICS_READY

or:

LYRICS_UNAVAILABLE.

---

# 121. LYRICS ERROR EVENT

Parser failures may publish:

LYRICS_ERROR.

The error should remain non-fatal.

---

# 122. EVENT EXAMPLE

PLAYBACK_TRACK_CHANGED

↓

LyricsController clears current lyrics

↓

loads lyrics for new track

↓

LYRICS_READY

↓

position updates select active line.

---

# 123. STALE LOAD PROTECTION

Lyrics loading must use the active track/playback generation.

---

# 124. CURRENT TRACK VALIDATION

Before publishing loaded lyrics:

verify the requested track still matches the active track.

---

# 125. LYRICS STATE MACHINE

Conceptually:

NO_LYRICS

↓

DISCOVERING

↓

LOADING

↓

PARSING

↓

READY

or:

MISSING

or:

INVALID

or:

ERROR.

---

# 126. STATE: NO_LYRICS

No lyric document is associated with the current track.

---

# 127. STATE: DISCOVERING

The system is searching configured lyric sources.

---

# 128. STATE: LOADING

A lyric source is being read.

---

# 129. STATE: PARSING

The loaded data is being converted into the normalized model.

---

# 130. STATE: READY

Valid lyrics are available.

---

# 131. STATE: MISSING

No lyric source was found.

---

# 132. STATE: INVALID

A source exists but cannot be parsed correctly.

If the entire lyric file cannot be parsed:

return:

LYRICS_INVALID.

---

# 133. STATE: ERROR

An unexpected I/O or internal error occurred.

---

# 134. UI BEHAVIOR

The UI may show:

Loading lyrics

No lyrics

Lyrics unavailable

depending on the state.

None of these states should stop audio playback.

---

# 135. SYNC STATES

When lyrics are ready:

the synchronization engine may be:

WAITING

or:

ACTIVE.

---

# 136. WAITING

Playback position is before the first lyric timestamp.

---

# 137. ACTIVE

Playback position is at or after at least one lyric timestamp.

---

# 138. AFTER LAST LINE

After the final lyric timestamp:

the final line remains active unless the UI specifies another behavior.

---

# 139. LINE TRANSITIONS

When playback crosses a lyric timestamp:

current_line_index changes.

---

# 140. UI EVENT

The lyrics engine may emit:

LYRIC_LINE_CHANGED.

---

# 141. EVENT CONTENT

The event may contain:

- track_id
- playback generation
- line index
- timestamp

---

# 142. EVENT RATE

Only emit when the active line changes.

Do not emit the same line every frame.

---

# 143. LYRIC STATE

Recommended conceptual state:

LyricPlaybackState

    generation
    track_id
    active_line_index
    current_line
    previous_line
    next_line
    position_frames
    synchronized

---

# 144. IMMUTABLE SNAPSHOT

The UI should receive a stable lyric snapshot.

Avoid sharing mutable parser structures directly with rendering code.

---

# 145. CURRENT LINE

The current line should be clearly distinguishable from:

previous

and:

upcoming

lyrics.

The lyrics controller should expose:

current_line_index

and:

current_line_text.

---

# 146. PREVIOUS LINE

The UI may show one or more previous lines.

The number is a presentation decision.

---

# 147. UPCOMING LINES

The UI may show upcoming lines.

The lyric engine should provide them without requiring the renderer to understand timestamp parsing.

---

# 148. WINDOWED ACCESS

Do not copy the entire lyric document into every UI update.

Return references or lightweight snapshots.

---

# 149. PREVIOUS/NEXT LINES

The UI may request a window:

previous N lines

+

current line

+

next N lines.

---

# 150. FULL LYRICS VIEW

The UI should not require the lyrics service to return a fully rendered screen.

The service returns structured lyric data.

---

# 151. MINI LYRICS

If a mini-player shows lyrics:

it should use the same lyrics controller state.

Do not create a second synchronization system.

---

# 152. FULL PLAYER

The full music player should consume the same authoritative lyric state.

---

# 153. SCREEN TRANSITIONS

Opening or closing the lyrics view must not restart lyrics parsing or playback.

---

# 154. SCROLLING

Automatic lyric scrolling belongs to the UI.

Lyric scrolling should be driven by:

active lyric index/state.

Not by arbitrary timer animations.

---

# 155. AUTO-SCROLL

For synchronized lyrics:

the UI may automatically scroll so the active line remains visible.

When active line changes:

the UI may smoothly move the lyric viewport.

The animation may be independent of audio timing.

However:

the target position must always correspond to the authoritative lyric line.

Auto-scroll should be UI-driven.

It must not alter lyric timestamps.

---

# 156. USER SCROLL

If the user manually scrolls lyrics:

automatic scrolling may temporarily pause.

The active line must still be tracked internally.

---

# 157. RETURN TO LIVE

The UI may provide a way to return to:

LIVE LYRIC POSITION.

A user action such as:

CENTER ON CURRENT

may return the view to the active line.

When activated:

scroll to the current active line.

---

# 158. WORD-LEVEL SYNCHRONIZATION

The initial architecture should support:

LINE-LEVEL SYNCHRONIZATION.

Word-level synchronization may be added later.

---

# 159. WORD-LEVEL EXTENSION

The model should avoid making future word-level timing impossible.

Potential future structure:

LyricLine

    timestamp
    text
    words[]

Each word could contain:

start_frame
end_frame
text

If karaoke-style word timing is supported later:

it should extend the lyric model without changing the audio engine.

---

# 160. INITIAL WORD-LEVEL IMPLEMENTATION

Do not implement word-level synchronization unless explicitly required.

It increases:

- parser complexity
- memory usage
- UI complexity
- timing complexity
- rendering complexity

---

# 161. LRC SUPPORT

If LRC is supported:

the parser should handle standard line timestamps.

---

# 162. RENDERING

Lyrics rendering is handled by the UI renderer.

The UI renderer owns:

- font
- layout
- wrapping
- scrolling
- highlighting
- animation

The lyric engine owns:

- timing
- parsing
- active-line selection

---

# 163. HIGHLIGHTING

Current-line highlighting is presentation.

The lyric engine only determines:

which line is current.

The UI may highlight the current line using:

- accent color
- weight
- brightness
- size

The exact style belongs to UI documentation.

---

# 164. LYRIC ENGINE INDEPENDENCE

The synchronization engine should not know how text is rendered.

Font selection and glyph rendering belong to the UI/text rendering subsystem.

Manual scrolling must not alter the underlying lyric timestamps.

---

# 165. COLOR

FLACHEAD's music-player visual theme may derive from album artwork.

Lyrics should follow the music-player theme defined by the UI system.

The lyric subsystem should not own theme decisions.

---

# 166. FONT

Font selection belongs to the UI.

The lyric model stores text only.

Font/glyph caching belongs to the text rendering system.

---

# 167. TEXT WRAPPING

Wrapping must not alter:

logical lyric-line identity.

One logical line may occupy multiple visual rows.

Long lyric lines may wrap visually.

Wrapping must not change their timing.

---

# 168. SCREEN SIZE

The 2.8" portrait TFT has limited space.

The renderer must optimize:

- line count
- font size
- spacing
- scrolling

without changing timing behavior.

---

# 169. TOUCH INTERACTION

If the lyric screen allows touch interaction:

touch behavior belongs to the UI/input system.

It must not directly manipulate decoder state.

---

# 170. SEEK FROM LYRICS

If tapping a lyric line seeks to its timestamp:

the UI sends:

SEEK(timestamp)

to the playback engine.

The lyric engine itself should not directly control ALSA.

---

# 171. LYRIC TAP VALIDATION

The selected line must belong to:

the active track

and:

current generation.

---

# 172. UNSYNCHRONIZED LYRIC UI

For unsynchronized lyrics:

the UI may display the lyrics as a scrollable document.

It must not pretend that a particular line is synchronized.

---

# 173. UNSYNCHRONIZED STATE

Recommended:

synchronized = false.

There should be no fake:

active_line_index

based on elapsed time.

No active line should be falsely highlighted.

---

# 174. FALLBACK DISPLAY

If no lyrics exist:

display a deterministic:

NO LYRICS

state.

Do not repeatedly attempt expensive lyric discovery during every frame.

---

# 175. LOADING DISPLAY

While lyrics are being discovered:

display:

LOADING LYRICS

only when the loading state is relevant.

---

# 176. ERROR DISPLAY

If a lyric source is malformed:

the UI may show:

LYRICS UNAVAILABLE

while debug logs contain the actual parser error.

---

# 177. DEBUG INFORMATION

Debug mode should expose:

- lyric source
- synchronized/unsynchronized state
- number of lines
- active line index
- current playback frame
- next lyric timestamp
- previous lyric timestamp
- parser warnings
- applied offset

---

# 178. LYRIC EFFECTS

Effects such as:

- fade
- highlight
- karaoke coloring
- scrolling

belong to the UI.

---

# 179. CACHE

Parsed lyric documents may be cached.

The cache must be invalidated when:

- source file changes
- lyric file changes
- metadata association changes

---

# 180. CACHE KEY

A local lyric cache key may include:

audio_track_id

+

lyric_source_path

+

source_modification_time.

---

# 181. CACHE KEY (DETAILED)

For an external lyric file:

use:

canonical path

+

file size

+

modification time.

Embedded lyrics should be associated with the relevant audio file/version signature.

---

# 182. CACHE HIT

If the lyrics source has not changed:

reuse the normalized lyrics.

---

# 183. CACHE MISS

On cache miss:

load

↓

parse

↓

normalize

↓

cache

↓

publish.

---

# 184. CACHE INVALIDATION

Cached lyrics must be invalidated when:

- source path changes
- source size changes
- modification time changes
- source signature changes
- lyric file changes
- embedded metadata changes
- source disappears
- parser version changes where necessary.

---

# 185. EXTERNAL FILE CHANGE

If an external `.lrc` file changes:

a later library/lyrics scan should detect the change.

---

# 186. LIVE MONITORING

Continuous filesystem monitoring is optional.

It is not required for the initial implementation.

---

# 187. CACHE MEMORY

Lyrics are generally much smaller than artwork.

However:

large lyric files and large libraries should still use bounded caching.

---

# 188. LRU CACHE

An LRU cache is appropriate for normalized lyric documents.

---

# 189. CACHE EVICTION

Least-recently-used lyric documents may be removed when memory limits are reached.

---

# 190. DISK CACHE

A persistent normalized lyric cache may be used.

It is optional.

---

# 191. CACHE FORMAT

The exact on-disk cache format is implementation-specific.

It must be:

- local
- recoverable
- versioned if necessary.

---

# 192. CACHE CORRUPTION

If the lyrics cache is corrupt:

delete/rebuild that cache entry.

Do not treat the track as corrupt.

---

# 193. CACHE VERSION

If the lyrics parser changes:

old cached representations may need invalidation.

A parser-version field may be stored with cached lyrics.

---

# 194. DATABASE FAILURE

Lyrics cache failure must not stop audio playback.

---

# 195. LARGE FILE PROTECTION

Do not allow an accidentally huge text file to consume uncontrolled RAM.

---

# 196. LYRICS PRELOAD

Lyrics may be loaded alongside the current track's metadata.

---

# 197. NEXT TRACK

Optionally preload lyrics for the next track.

Do not preload lyrics for the entire queue.

---

# 198. PRELOAD PRIORITY

Current-track lyrics have higher priority than next-track lyrics.

---

# 199. PRELOAD LIMIT

Do not preload lyrics for the entire queue.

---

# 200. QUEUE INDEPENDENCE

Lyrics do not modify queue order.

Queue entries do not contain complete lyric documents.

They contain track identity.

---

# 201. PLAYLIST INDEPENDENCE

Lyrics do not modify playlist membership.

---

# 202. METADATA INTEGRATION

The lyrics service may use metadata for discovery.

It must not overwrite authoritative track metadata.

---

# 203. LYRICS AND ARTWORK

Lyrics and artwork may both use:

track_id.

They must not depend on each other.

---

# 204. LYRICS AND AUDIO ENGINE

The audio engine exposes timing.

The lyrics engine consumes timing.

---

# 205. LYRICS AND SEEK

Seek events must notify or invalidate the lyrics synchronization state.

---

# 206. LYRICS SOURCE PRIORITY

Recommended order:

1. explicit user-selected lyrics source
2. valid embedded synchronized lyrics
3. valid external synchronized lyrics
4. embedded unsynchronized lyrics
5. external unsynchronized lyrics
6. no lyrics

The exact preference can be configurable.

---

# 207. LOCAL LYRIC FILE

A local lyric file should normally be associated with:

the same track identity.

Common naming may be based on the audio filename.

---

# 208. WRONG LYRIC FILE

The system should avoid blindly applying a lyric file merely because its filename matches.

Where practical, verify:

- title
- artist
- album
- file association

---

# 209. LYRIC MATCHING

The matching algorithm should prioritize:

strong file association

over:

approximate metadata similarity.

---

# 210. USER OVERRIDE

If the user manually selects a lyric source:

that choice should take precedence for the current track.

---

# 211. PERSISTENT SOURCE SELECTION

A future implementation may persist a user's preferred lyric source.

This is optional.

---

# 212. LYRICS LANGUAGE SELECTION

If multiple lyric languages exist:

the UI may allow language selection.

The core architecture must support more than one lyric representation.

---

# 213. MULTIPLE LYRIC SOURCES

A track may have:

embedded lyrics

and:

external LRC.

These should not be treated as duplicate track records.

---

# 214. SOURCE IDENTITY

Each lyric source should have an internal identity.

Example:

EMBEDDED

EXTERNAL_LRC

EXTERNAL_TEXT.

---

# 215. RECOMMENDED DISCOVERY ORDER

Recommended initial order:

1. explicit track-associated lyric source
2. external `.lrc` file matching the audio filename
3. embedded lyrics where supported
4. application-managed lyric source
5. no lyrics

---

# 216. FILENAME MATCH

For:

`Track.flac`

a likely external lyrics candidate is:

`Track.lrc`.

---

# 217. DIRECTORY SEARCH

A limited directory search may be used for common album-level lyric sources.

Avoid uncontrolled recursive searching.

---

# 218. PATH SECURITY

Lyrics discovery must remain within configured music/library roots.

---

# 219. SYMLINKS

The implementation must define whether symbolic links are followed.

Do not unintentionally access arbitrary filesystem locations.

---

# 220. EXTERNAL FILE VALIDATION

Before loading an external lyric file:

validate:

- path
- file type
- file size
- access permissions

---

# 221. SIZE LIMIT

Lyrics files should have a reasonable maximum size.

Extremely large files should not be loaded blindly into RAM.

---

# 222. TRACK MATCHING

External lyric files should be associated conservatively.

Preferred resolution:

1. exact known track association
2. exact corresponding filename
3. configured safe fallback.

---

# 223. AMBIGUOUS MATCH

If multiple possible lyric files exist:

do not silently choose an unrelated file.

Use deterministic priority.

---

# 224. SAME-NAME MATCH

For:

song.flac

and:

song.lrc

the `.lrc` file is a strong candidate.

---

# 225. WRONG LYRIC FILE

The system should avoid blindly applying a lyric file merely because its filename matches.

Where practical, verify:

- title
- artist
- album
- file association

---

# 226. FAST TRACK SKIPPING

If the user rapidly skips through tracks:

obsolete lyric requests should be cancelled or rejected.

---

# 227. REQUEST COALESCING

Duplicate requests for the same track/source may be coalesced.

---

# 228. LYRIC LOADING

Lyrics may load asynchronously after the track starts.

The track must not wait unnecessarily for lyric parsing.

---

# 229. LATE LYRIC LOAD

If lyrics become available after playback has started:

the lyric matcher should immediately synchronize them against:

current audio position.

---

# 230. LATE LOAD EXAMPLE

Playback:

60 s

Lyrics finish loading:

at 65 s.

The system should immediately determine:

which line is active at 65 s.

It must not start from line 1.

---

# 231. LYRIC DISAPPEARANCE

If the lyric source becomes invalid:

the current lyric document may remain active until the track changes.

Do not cause visual instability during playback.

---

# 232. TRACK CHANGE (CLEAR)

On track change:

old lyrics become invalid immediately.

Do not display Track A lyrics under Track B.

---

# 233. LYRIC CLEAR

During transition:

the UI may temporarily display:

No Lyrics

or:

Loading Lyrics.

It must not display stale lyrics as current.

---

# 234. LYRIC STATE GENERATION

Each lyric state update should contain:

generation

and:

track_id.

This makes stale updates rejectable.

---

# 235. UI EVENT VALIDATION

The UI should ignore lyric events whose:

generation != active generation.

---

# 236. POSITION RACE

Possible race:

1. Track A position update is queued.
2. Track B becomes active.
3. Track A lyric update arrives late.

The event must be discarded.

---

# 237. EVENT ORDERING

The system must not assume asynchronous events always arrive in the order they were generated.

Generation/track identity provides protection.

---

# 238. AUDIO CLOCK

The lyric engine should use the same authoritative playback clock used by:

- progress UI
- elapsed time
- remaining time
- lyrics
- queue transition timing

---

# 239. CLOCK CONSISTENCY

The following should agree:

- progress bar
- elapsed time
- remaining time
- lyrics
- queue transition timing

All should derive from the same playback timeline.

---

# 240. NO INDEPENDENT LYRIC TIMER

Do not implement:

lyric_timer += delta_time

as the primary synchronization mechanism.

It will drift from actual audio.

---

# 241. NO POLLING LOOP

Do not create a dedicated high-frequency lyric thread.

---

# 242. EVENT-DRIVEN MODEL

Prefer:

playback position updates

+

UI update scheduling.

---

# 243. SYNCHRONIZATION FREQUENCY

The implementation should update often enough for visually accurate lyric transitions without wasting CPU.

---

# 244. PERFORMANCE

Lyric synchronization must be lightweight.

The Pi Zero W should not spend significant CPU time searching lyric lists every frame.

---

# 245. INDEXING

Lyrics should be sorted once during parsing.

Build an efficient timestamp index.

---

# 246. NORMAL PLAYBACK COMPLEXITY

Normal lyric advancement should approach:

O(1)

per lyric transition.

---

# 247. SEEK COMPLEXITY

Seeking to a lyric line should ideally use:

O(log n)

binary search.

---

# 248. UI UPDATE LIMIT

Do not trigger a complete UI redraw for every audio frame.

Only publish lyric-state changes when:

- active line changes
- relevant position display changes
- seek occurs
- track changes
- lyric source changes

---

# 249. LYRIC PREVIEW

The UI may display:

previous/current/next.

The lyric engine can expose a small window around:

active_line_index.

---

# 250. MEMORY EFFICIENCY

Store lyric text in a compact representation.

Avoid unnecessary duplicate strings.

---

# 251. STRING OWNERSHIP

The normalized lyrics document owns its lyric text.

UI references should not duplicate text unnecessarily.

---

# 252. LIFETIME

A lyric document remains alive while referenced by:

- active player
- cache
- lyrics UI.

---

# 253. RESOURCE RELEASE

When no longer referenced:

the lyric document may be released or evicted.

---

# 254. HISTORY

Lyrics access should not create playback-history entries.

---

# 255. SEARCH

Lyrics may optionally be searchable in the future.

This is not required for the initial architecture.

---

# 256. DATABASE STORAGE

Lyrics need not be fully stored in the main track database.

A separate cache may be used.

---

# 257. RECOMMENDED STORAGE

Store:

- source identity
- source signature
- parsed representation
- synchronization state.

---

# 258. FORMATS

The architecture should allow additional synchronized formats later.

Possible examples:

- enhanced LRC
- other timestamped lyric formats

---

# 259. FORMAT ABSTRACTION

The parser interface should conceptually be:

detect_format()

parse()

normalize()

LRC-specific parsing should not contaminate the synchronization engine.

All supported formats should produce the same normalized lyric model.

---

# 260. FUTURE EMBEDDED LYRICS

If embedded lyrics are supported later:

they should feed into the same normalization pipeline.

---

# 261. CONFIGURATION

Lyrics configuration may control:

- automatic lyric loading
- external lyric search
- preferred source
- synchronization offset
- auto-scroll behavior

---

# 262. USER OFFSET

A future user-configurable lyric offset may be supported.

User offset should be applied after source normalization.

---

# 263. OFFSET SEPARATION

Source LRC offset and user playback offset must remain logically separate.

---

# 264. RESET OFFSET

User offset may be reset independently without modifying source lyrics.

---

# 265. PERSISTENCE

Per-track lyric offsets may optionally be persisted.

---

# 266. PERSISTENCE KEY

A per-track offset should use stable track identity.

---

# 267. DISPLAY PRECISION

The UI may display lyric timing with appropriate precision if debugging tools require it.

Normal user-facing display does not need millisecond precision.

---

# 268. TARGET

Normal lyric synchronization should consume negligible CPU compared with:

audio decoding

or:

rendering.

---

# 269. LIMITS

The implementation should update often enough for visually accurate lyric transitions without wasting CPU.

---

# 270. KARAOKE

If karaoke-style word timing is supported later:

it should extend the lyric model without changing the audio engine.

---

# 271. DEBUGGING

Debug logs may include:

LYRICS_DISCOVERY

LYRICS_LOAD

LYRICS_PARSE

LYRICS_READY

LYRICS_MISSING

LYRICS_ERROR

LYRIC_LINE_CHANGED

LYRICS_CACHE_HIT

LYRICS_CACHE_MISS.

---

# 272. LOGGING RESTRICTION

Do not log lyric state on every frame.

---

# 273. DEBUG OVERLAY

A development/debug UI may expose:

- current playback time
- current lyric index
- current lyric timestamp
- next lyric timestamp
- applied offset

---

# 274. ATOMIC SOURCE WRITES

If users or external tools modify lyrics:

the loader should avoid consuming partially written files where practical.

---

# 275. FILE DISAPPEARANCE

If a lyric file disappears while loading:

return a missing/error state safely.

---

# 276. FILE MODIFICATION

If the lyric file changes while being read:

the implementation should detect inconsistency where practical and retry or invalidate the result.

---

# 277. PERMISSION ERROR

Permission failures should be logged but not propagated into playback.

---

# 278. I/O ERROR

If a lyric file cannot be read:

return a safe error state.

---

# 279. PARSER ERROR

A malformed lyric line should not invalidate otherwise valid lines unless the document is structurally unusable.

---

# 280. PARTIAL PARSE

If part of a lyric file is valid:

use the valid portion where safe.

---

# 281. ERROR HANDLING

Lyric errors should be localized.

---

# 282. LYRIC LOADING INDEPENDENCE

Lyrics must never be required for playback.

Missing lyrics are not an audio error.

Playback must continue normally.

Malformed lyrics should be isolated from playback.

The track remains playable.

---

# 283. ACCEPTANCE TEST: NORMAL PLAYBACK

Play a synchronized lyric track.

Verify:

line changes occur at expected timestamps.

---

# 284. ACCEPTANCE TEST: PAUSE

Pause during a lyric line.

Verify:

the line remains stable.

---

# 285. ACCEPTANCE TEST: RESUME

Resume.

Verify:

line synchronization continues correctly.

---

# 286. ACCEPTANCE TEST: FORWARD SEEK

Seek forward across several lines.

Verify:

the displayed lyric jumps directly to the correct line.

---

# 287. ACCEPTANCE TEST: BACKWARD SEEK

Seek backward.

Verify:

the lyric index moves backward correctly.

---

# 288. ACCEPTANCE TEST: RAPID SEEK

Perform rapid seek operations.

Verify:

obsolete lyric states do not appear.

---

# 289. ACCEPTANCE TEST: TRACK CHANGE

Track A → Track B.

Verify:

Track A lyrics disappear immediately.

Track B lyrics become authoritative.

---

# 290. ACCEPTANCE TEST: LATE LOAD

Start playback before lyrics finish loading.

Verify:

lyrics synchronize to current playback position when loaded.

---

# 291. ACCEPTANCE TEST: MISSING LYRICS

Play a track without lyrics.

Verify:

audio playback remains completely functional.

---

# 292. ACCEPTANCE TEST: MALFORMED LYRICS

Use malformed timestamps/text.

Verify:

application remains stable.

---

# 293. ACCEPTANCE TEST: MULTI-TIMESTAMP

Use a lyric line with multiple timestamps.

Verify:

each timestamp produces the expected active-line state.

---

# 294. ACCEPTANCE TEST: OFFSET

Use lyrics with a timing offset.

Verify:

effective timing is correct.

Offset is applied exactly once.

Only lyric display timing changes.

Audio playback remains unchanged.

---

# 295. ACCEPTANCE TEST: HIGH-RESOLUTION AUDIO

Test lyric synchronization during:

96 kHz / 24-bit

playback where supported.

Verify:

lyrics remain synchronized with audible playback.

---

# 296. ACCEPTANCE TEST: STALE LOAD

Start loading lyrics for A.

Switch to B before A finishes.

Verify A's lyrics never appear for B.

---

# 297. ACCEPTANCE TEST: UNSYNCED

Load a plain text lyric file.

Verify lyrics display without fake synchronization.

---

# 298. ACCEPTANCE TEST: UNICODE

Use multilingual lyrics.

Verify:

correct storage

correct rendering

correct scrolling.

Text is preserved correctly.

---

# 299. ACCEPTANCE TEST: EXTERNAL CHANGE

Modify an LRC file.

Rescan/reload.

Verify:

new lyric data is used.

Old cached lyrics are not returned.

---

# 300. ACCEPTANCE TEST: CACHE

Open lyrics repeatedly.

Verify:

cached data avoids unnecessary reparsing.

Cache reuse occurs.

---

# 301. ACCEPTANCE TEST: CORRUPT CACHE

Corrupt a lyrics cache entry.

Verify:

the entry is rebuilt without affecting playback.

---

# 302. ACCEPTANCE TEST: DSP OFFSET

Configure a lyric offset.

Verify:

only lyric display timing changes.

Audio playback remains unchanged.

---

# 303. ACCEPTANCE TEST: SYSTEM CLOCK

Change system time during playback.

Verify:

lyrics remain synchronized to playback.

---

# 304. ACCEPTANCE TEST: UNDERRUN

Simulate delayed audio output.

Verify:

lyrics follow playback position rather than wall-clock assumptions.

---

# 305. ACCEPTANCE TEST: LARGE FILE

Load an unusually large lyric file.

Verify:

memory usage remains bounded and the UI remains responsive.

---

# 306. ACCEPTANCE TEST: DUPLICATE TIMESTAMPS

Use multiple lyric lines at the same timestamp.

Verify:

source order is preserved.

---

# 307. ACCEPTANCE TEST: PLAYBACK DURING LOAD

Delay lyric loading.

Verify:

audio playback remains unaffected.

---

# 308. ACCEPTANCE TEST: AUDIO THREAD

Perform lyric loading during playback.

Verify:

no lyric parsing occurs on the audio thread.

---

# 309. ACCEPTANCE TEST: RAPID TRACK CHANGES

Rapidly switch between tracks.

Verify:

only the active track's lyrics are published.

---

# 310. ACCEPTANCE TEST: NO NETWORK

Disable network connectivity.

Verify:

local lyrics continue to function.

---

# 311. ACCEPTANCE TEST: LYRIC FAILURE

Force lyric loading failure.

Verify:

audio remains playable.

---

# 312. ACCEPTANCE CRITERIA

The lyrics architecture is production-ready when:

- lyrics are associated with stable track IDs
- embedded lyrics are supported
- external LRC lyrics are supported
- unsynchronized lyrics are supported
- timestamps are converted to precise internal units
- Unicode is preserved
- malformed lyric data is non-fatal
- missing lyrics are non-fatal
- playback position is authoritative
- seeking updates lyrics immediately
- backward seek works
- rapid seeks do not produce stale lyric states
- pause freezes lyric state
- resume continues correctly from actual playback
- track changes invalidate stale lyrics
- playback generations prevent stale lyric loads
- lyrics are cached
- cache invalidation is deterministic
- artwork and lyrics remain separate
- UI rendering remains independent from lyric parsing
- the audio output thread never performs lyric work
- DSP latency can be compensated without altering audio
- system clock changes do not affect synchronization
- queue and playlist behavior remain independent
- no internet connection is required
- lyrics never drive playback timing
- synchronized lyrics select the correct active line
- unsynchronized lyrics remain explicitly unsynchronized
- late lyric loading synchronizes immediately
- lyric events are generation-aware
- huge lyric files cannot exhaust RAM
- normal playback does not repeatedly scan the full lyric list
- progress, elapsed time, and lyrics share one authoritative timeline
- synchronization remains lightweight
- lyric processing never runs on the audio thread
- lyric processing never blocks playback
- automatic scrolling remains a UI responsibility
- future lyric formats can use the same normalized model

---

# 313. AI CODING AGENT RULES

The coding agent MUST:

1. Treat playback position as the authoritative lyrics clock.
2. Never use wall-clock time or an independent lyric timer for lyric synchronization.
3. Store lyric timestamps in a precise integer representation.
4. Prefer audio-frame-based timing internally.
5. Normalize lyric timestamps during parsing.
6. Sort lyric entries deterministically.
7. Support backward and forward seeks.
8. Re-index lyrics after seeks.
9. Use binary search for large position jumps.
10. Use incremental advancement during normal playback.
11. Keep lyric state associated with stable track IDs.
12. Keep lyric state associated with playback generation.
13. Reject stale lyric events.
14. Never allow old-track lyrics to appear on a new track.
15. Keep lyric parsing outside the audio output thread.
16. Keep UI rendering outside the audio output thread.
17. Keep lyric parsing off the UI thread for large files.
18. Support asynchronous lyric loading.
19. Synchronize late-loaded lyrics immediately against the current position.
20. Never require lyrics for playback.
21. Never use network lyrics as a playback dependency.
22. Handle malformed timestamps safely.
23. Handle malformed Unicode safely.
24. Bound lyric-file memory usage.
25. Preserve unsynchronized lyrics as unsynchronized.
26. Do not fabricate timing for unsynchronized lyrics.
27. Keep theme/layout decisions inside the UI layer.
28. Keep seek control inside the playback command system.
29. Never directly manipulate ALSA from the lyric subsystem.
30. Keep lyric indexing efficient on the Pi Zero W.
31. Test synchronization against actual audible playback rather than decoder progress.
32. Keep lyrics independent from audio decoding, queue management, and playlist management.
33. Support embedded lyrics, external LRC files, and unsynchronized text lyrics.
34. Parse timestamps into integer time units.
35. Preserve Unicode text.
36. Cache parsed lyrics and invalidate caches when their source changes.
37. Use playback generations to prevent stale lyric loads.
38. Clear previous lyrics immediately on track change.
39. Recalculate lyrics immediately after seeking.
40. Freeze lyric progression during pause.
41. Never let the audio output thread parse lyrics or perform lyric file I/O.
42. Coalesce unnecessary position updates.
43. Keep lyric rendering separate from lyric parsing.
44. Allow configurable synchronization offset.
45. Keep synchronization offset separate from audio timing.
46. Support multilingual Unicode lyrics.
47. Keep lyrics cache failures non-fatal.
48. Never let lyrics architecture compromise audio playback stability.
49. Treat lyrics as presentation data.
50. Use the playback clock as the authoritative timeline.
51. Apply source offsets exactly once.
52. Use request generations for asynchronous loading.
53. Continue from the real playback position after resume.
54. Reset lyric context on track change.
55. Use efficient timestamp lookup.
56. Prefer binary search or monotonic index advancement where appropriate.
57. Avoid per-sample lyric processing.
58. Emit line-change events rather than repeated frame events.
59. Keep lyric cache memory bounded.
60. Never let lyric errors stop audio playback.
61. Keep artwork and lyrics independent.
62. Keep queue and lyrics independent.
63. Keep future network lyrics isolated from the offline core.
64. Never allow user-facing lyric features to compromise audio timing.

---

# 314. FINAL LYRICS ARCHITECTURE

The intended architecture is:

                         TRACK
                           │
                           ▼
                   LYRICS DISCOVERY
                           │
                           ▼
                     LYRICS LOADER
                           │
                           ▼
                     LYRICS PARSER
                           │
                           ▼
                NORMALIZED LYRIC MODEL
                           │
                           ▼
                      LYRICS CACHE
                           │
                           ▼
                  PLAYBACK POSITION
                           │
                           ▼
                 SYNCHRONIZATION ENGINE
                           │
                     CURRENT LINE
                           │
                           ▼
                           UI

---

Lyric discovery is separate:

                     TRACK
                       │
              ┌────────┴────────┐
              ▼                 ▼
        EMBEDDED LYRICS     LOCAL LYRICS
              │                 │
              └────────┬────────┘
                       ▼
                  LYRIC PARSER
                       │
                       ▼
                NORMALIZED MODEL
                       │
                       ▼
                  TIMESTAMP INDEX
                       │
                       ▼
                  LYRIC MATCHER

---

The central invariant is:

THE AUDIO TIMELINE IS AUTHORITATIVE; LYRICS FOLLOW IT.

The second invariant is:

NO LYRIC STATE FROM AN INVALIDATED TRACK OR PLAYBACK GENERATION MAY REACH THE ACTIVE UI.

The third invariant is:

SEEKING CHANGES THE LYRIC INDEX, NOT THE AUDIO CLOCK.

The fourth invariant is:

PAUSING AUDIO PAUSES LYRIC PROGRESSION BECAUSE BOTH DEPEND ON THE SAME PLAYBACK TIMELINE.

The fifth invariant is:

UNSYNCHRONIZED LYRICS MUST NEVER BE PRESENTED AS SYNCHRONIZED.

The sixth invariant is:

MISSING OR MALFORMED LYRICS MUST NEVER STOP AUDIO PLAYBACK.

The seventh invariant is:

AUDIO THREADS MUST NEVER PERFORM LYRICS I/O, PARSING, OR UI WORK.

---

Final policy:

Keep lyric parsing asynchronous.

Keep lyric matching lightweight.

Keep timing frame-based.

Keep track identity explicit.

Keep generation identity explicit.

Use audio position as the sole authoritative clock.

Make seeking immediately re-anchor lyrics.

Make track transitions invalidate old lyric state.

Never let lyrics interfere with audio playback.

And ensure the lyric system remains completely functional without an internet connection.

Always preserve the separation:

AUDIO CLOCK = PLAYBACK TRUTH

LYRIC FILE = SOURCE

LYRICS PARSER = EXTRACTION

NORMALIZED TIMELINE = APPLICATION MODEL

LYRICS CACHE = PERSISTENT/IN-MEMORY DATA

SYNCHRONIZATION ENGINE = CURRENT-LINE CALCULATION

UI = VISUAL PRESENTATION

AUDIO ENGINE = PLAYBACK EXECUTION
