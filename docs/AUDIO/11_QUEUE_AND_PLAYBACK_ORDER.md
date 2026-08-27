# FLACHEAD QUEUE AND PLAYBACK ORDER ARCHITECTURE

Document

11_QUEUE_AND_PLAYBACK_ORDER.md

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

This document defines the architecture and behavior of the FLACHEAD playback queue.

The queue system controls:

- current track
- upcoming tracks
- playback order
- next/previous behavior
- manual queue additions
- album playback
- artist playback
- playlist playback
- shuffle
- repeat
- queue editing
- queue persistence
- missing tracks
- library changes
- playback failures
- queue recovery
- interaction with the audio engine.

The primary objective is:

PROVIDE A DETERMINISTIC, LOW-MEMORY PLAYBACK QUEUE THAT CAN CONTROL TRACK ORDER WITHOUT COUPLING QUEUE STATE TO AUDIO DECODING.

---

# 2. CORE PRINCIPLE

The library answers:

"WHAT MUSIC EXISTS?"

The queue answers:

"WHAT SHOULD PLAY?"

The playback engine answers:

"HOW IS THE CURRENT TRACK PLAYED?"

These are separate systems.

The queue represents:

PLAYBACK INTENT.

It does not contain decoded audio.

It does not own the audio decoder.

It does not own the audio device.

It does not determine audio timing.

---

# 3. ARCHITECTURAL RELATIONSHIP

The intended flow is:

                    LIBRARY SERVICE
                           │
                           ▼
                    TRACK SELECTION
                           │
                           ▼
                     +-------------+
                     | QUEUE       |
                     +-------------+
                           │
                           ▼
                    PLAYBACK ENGINE
                           │
                           ▼
                        DECODER
                           │
                           ▼
                          PCM
                           │
                           ▼
                     BUNNY DSP

---

# 4. QUEUE OWNERSHIP

The queue service owns:

- queue entries
- ordering
- current queue position
- shuffle state
- repeat state
- queue modifications
- next/previous resolution.

The playback engine owns:

- decoder state
- audio position
- active PCM pipeline
- actual playback state
- opening the selected track
- decoding
- buffering
- audio output.

---

# 5. QUEUE ENTRY

A queue entry should conceptually contain:

QueueEntry

    queue_entry_id
    track_id
    source
    added_at
    original_index
    playback_index
    availability
    optional display snapshot
    metadata_snapshot

---

# 6. TRACK IDENTITY AND REFERENCES

A queue entry must reference:

track_id.

It should not rely only on:

- title
- filename
- album
- artist.

Queue entries should reference track_id.

Do not copy the entire TrackMetadata object into every queue entry.

---

# 7. QUEUE ENTRY ID

Every queue entry should have its own identity.

This is important because the same track may legitimately appear multiple times.

Example:

Track A

Track B

Track A

These are two distinct queue entries referencing the same track.

---

# 8. DUPLICATE QUEUE ENTRIES

Duplicate tracks are allowed.

The queue must not automatically deduplicate user selections.

Example:

01.flac

02.flac

01.flac

This must remain valid.

---

# 9. QUEUE SOURCES

Queue entries may originate from:

- individual track selection
- album playback
- artist playback
- playlist
- search result
- folder
- favorites
- recently played
- manual queue insertion
- autoplay.

---

# 10. SOURCE METADATA

The queue may retain the source that created an entry.

Example:

source = ALBUM

source_id = album_id

The source is informational unless explicitly used by queue logic.

This is useful for debugging and UI behavior.

---

# 11. QUEUE VS PLAYLIST

A playlist is persistent user organization.

A queue is temporary playback order.

Do not automatically treat them as the same object.

---

# 12. QUEUE VS PLAYBACK STATE

Queue state answers:

WHAT SHOULD PLAY?

Playback state answers:

WHAT IS CURRENTLY HAPPENING TO THE AUDIO?

These are separate.

---

# 13. SEPARATION OF CONCERNS

The queue never contains decoded audio.

The queue never creates decoder buffers.

The queue never directly controls the TANCHJIM BUNNY DSP.

The queue never performs arbitrary SQL directly.

The lyrics system uses the track identity from the queue.

The queue does not parse lyrics.

Artwork is resolved separately through metadata/artwork systems.

Metadata is resolved separately through track IDs.

---

# 14. QUEUE LIFETIME

The active queue normally exists for the current player session.

Optional persistence may restore it after restart.

---

# 15. CURRENT TRACK

The queue maintains a pointer/reference to the currently active queue entry.

This should be:

current_queue_index

or an equivalent stable queue-entry reference.

The playback engine receives the track ID represented by the current queue entry.

---

# 16. CURRENT TRACK VS TRACK ID

The current item should identify a queue entry, not merely a track.

This matters when:

the same track occurs more than once.

---

# 17. PLAYBACK STATES

The queue must support playback states such as:

- stopped
- loading
- playing
- paused
- seeking
- finished
- error.

These states belong to the playback engine, not the queue itself.

---

# 18. QUEUE STATES

The queue may conceptually be:

- empty
- ready
- active
- exhausted.

---

# 19. EMPTY QUEUE

An empty queue is a valid state.

It must not be treated as an application error.

An empty queue means:

there is no track scheduled for playback.

The playback engine must not attempt to decode a nonexistent queue entry.

---

# 20. PLAYBACK ORDER

Normal queue order is:

entry 0

entry 1

entry 2

entry 3

...

---

# 21. ADVANCE

When the current track completes:

the queue selects the next playable entry according to:

- repeat policy
- shuffle policy
- availability
- queue boundaries.

---

# 22. END-OF-TRACK

When the current track reaches its natural end:

the playback engine reports:

TRACK_FINISHED.

The queue manager resolves the next action.

---

# 23. NEXT RESOLUTION

Next-track resolution considers:

1. repeat mode
2. shuffle mode
3. current queue position
4. queue contents.

---

# 24. NEXT

User pressing:

NEXT

should advance according to the active playback order.

---

# 25. PREVIOUS

Previous behavior must be deterministic.

Recommended behavior:

If current playback position is beyond a configurable threshold:

PREVIOUS

restarts the current track.

Otherwise:

PREVIOUS

moves to the previous queue entry.

---

# 26. PREVIOUS THRESHOLD

A practical initial threshold may be:

3 seconds.

The exact value should remain configurable.

---

# 27. PREVIOUS AT TRACK START

If already near the beginning:

previous selects the preceding queue entry.

---

# 28. PREVIOUS AT FIRST TRACK

If no previous entry exists:

behavior depends on repeat mode.

With Repeat Queue:

wrap to the final entry.

Without Repeat Queue:

restart current track or remain at the first entry.

---

# 29. NEXT AT END

If the queue reaches its end:

behavior depends on repeat mode.

---

# 30. REPEAT MODES

The queue must support:

OFF

ONE

ALL

---

# 31. REPEAT OFF

When the final queue entry completes:

playback stops.

---

# 32. REPEAT ONE

When a track completes:

the same queue entry plays again.

The queue index does not advance.

---

# 33. REPEAT ONE AND NEXT

If the user explicitly presses NEXT:

the queue should advance.

Repeat One applies to automatic end-of-track behavior, not an explicit NEXT command.

---

# 34. REPEAT ALL

When the final queue entry completes:

return to the first playable entry.

---

# 35. REPEAT ALL WITH SHUFFLE

When shuffle is enabled:

repeat-all should repeat the shuffled sequence rather than simply returning to the original first item.

---

# 36. SHUFFLE

Shuffle should create a playback order.

It must not randomly mutate the actual queue every time NEXT is pressed.

---

# 37. SHUFFLE PRINCIPLE

Maintain:

canonical queue order

and:

playback order.

---

# 38. CANONICAL ORDER

The canonical order represents:

the user's queue.

Example:

A B C D E

---

# 39. SHUFFLE PLAYBACK ORDER

A shuffled playback sequence should be generated from queue entries.

Shuffle may produce:

C A E B D

The canonical queue remains:

A B C D E.

---

# 40. WHY SEPARATE ORDERS

This allows:

- deterministic previous behavior
- predictable queue display
- shuffle toggling
- queue editing
- repeat-all.

If shuffle physically rearranges the queue:

the UI may no longer represent the user's intended queue ordering.

---

# 41. SHUFFLE INDEX

The queue should maintain a position within:

playback_order.

---

# 42. SHUFFLE RANDOMNESS

Shuffle should use a proper pseudo-random generator.

Use an unbiased shuffle algorithm.

Fisher-Yates is appropriate.

Do not use:

track title hashes

or:

filesystem order

as a substitute for randomization.

---

# 43. SHUFFLE SEED

A session-level random seed is sufficient.

The implementation may use a runtime-generated random seed.

---

# 44. REPEATABLE DEBUGGING

For debugging/testing:

the shuffle system should support deterministic seeding.

---

# 45. SHUFFLE CURRENT TRACK

When enabling shuffle during playback:

the current track should remain current.

The remaining tracks are shuffled.

---

# 46. SHUFFLE DISABLE

When disabling shuffle:

future playback returns to canonical queue order.

The current track should not unexpectedly restart.

---

# 47. SHUFFLE TOGGLE

Toggling shuffle must not silently replace the current track.

---

# 48. SHUFFLE UNIQUENESS

Each queue entry should normally appear once per shuffle cycle.

---

# 49. SHUFFLE REPEAT

When the shuffle cycle is exhausted:

generate another randomized traversal.

Avoid immediately replaying the same current entry unless the queue contains only one entry.

---

# 50. SHUFFLE AND REPEAT

Repeat Queue + Shuffle means:

repeat the shuffled traversal after all entries have been played.

---

# 51. REPEAT ONE + SHUFFLE

Repeat One takes precedence for automatic completion of the current entry.

---

# 52. QUEUE DISPLAY

The UI may display:

- current track
- played entries
- upcoming entries.

---

# 53. QUEUE UI ORDER

When shuffle is enabled:

the UI may display either:

canonical order with shuffle indicator

or:

actual playback order.

The choice must be consistent across the application.

---

# 54. RECOMMENDED UI

Show actual upcoming playback order while making shuffle state visually clear.

---

# 55. QUEUE SNAPSHOT

The queue may maintain an immutable-ish snapshot for a playback session while UI operations modify the queue through controlled commands.

The queue should be capable of generating a consistent snapshot for the UI.

A queue snapshot may include:

- queue entries
- current index
- playback order
- repeat mode
- shuffle state.

---

# 56. SNAPSHOT SAFETY

The UI should not directly mutate internal queue structures.

---

# 57. QUEUE EDITING

The user may:

- remove an entry
- move an entry
- clear the queue
- add entries
- add an album
- add a playlist.

---

# 58. REMOVE ENTRY

Removing a future queue entry must not affect the currently playing audio.

Removing a queue entry removes that queue entry only.

If the same track occurs twice:

removing one occurrence must not remove the other.

---

# 59. REMOVE CURRENT ENTRY

If the user removes the current queue entry:

do not immediately kill audio unless explicitly requested by the interaction semantics.

The playback engine may finish the current track.

The queue should determine the next valid entry afterward.

Recommended behavior:

If another entry exists after the current entry:

advance to it.

Otherwise:

use the previous valid entry if appropriate.

If the queue becomes empty:

stop playback.

---

# 60. CLEAR QUEUE

Clearing the queue should remove future entries.

The current track may:

- continue playing
- or stop

depending on the explicit UI command.

The default should avoid unexpectedly interrupting active playback.

---

# 61. CLEAR QUEUE DURING PLAYBACK

The implementation must define whether:

- current playback continues until the track ends
- playback stops immediately.

Recommended behavior:

preserve the currently playing track until it finishes, while removing future queue entries.

A separate STOP action should be used for immediate playback termination.

---

# 62. CLEAR QUEUE POLICY

Recommended:

CLEAR QUEUE removes future tracks.

Current playback continues.

When the current track completes:

there is no next queue entry.

---

# 63. STOP AND CLEAR

If the user explicitly selects:

STOP AND CLEAR

then:

stop playback

and:

clear queue.

---

# 64. ADD TO QUEUE

Adding tracks should append them after the current queue content unless another insertion position is explicitly selected.

---

# 65. PLAY NOW

"Play Now" should replace or reconfigure the queue according to the product's defined behavior.

Recommended:

selected track becomes current

and:

the selected context becomes the new queue.

---

# 66. PLAY NEXT

"Play Next" inserts the selected entry immediately after the current playback entry.

---

# 67. ADD TO END

"Add to Queue" appends the selected entries.

---

# 68. INSERT TRACK

The queue should support insertion at a specific logical position.

---

# 69. REORDER

The queue should support reordering entries.

---

# 70. REORDER CURRENT

Moving the current entry must not unexpectedly restart playback.

---

# 71. REORDER FUTURE ENTRIES

Future queue entries can be reordered without interrupting current playback.

---

# 72. DRAG REORDER

The UI may provide drag-based queue reordering.

The queue manager receives the final mutation.

---

# 73. QUEUE MOVE

Moving an entry should update:

canonical order

and:

playback order

according to shuffle policy.

---

# 74. MOVE DURING SHUFFLE

The implementation must define whether moving an entry changes:

only canonical order

or:

both canonical and current playback order.

Recommended:

update both in a deterministic manner while preserving the currently playing item.

---

# 75. REMOVE DURING SHUFFLE

Removing an entry must remove it from:

canonical order

and:

playback order.

---

# 76. ADD DURING SHUFFLE

Adding a track should not unexpectedly interrupt playback.

Recommended:

append it to the canonical queue and insert it into the future shuffled order according to a deterministic policy.

---

# 77. SHUFFLE INSERTION

The newly added track may be inserted randomly among future playback entries.

The current track remains unchanged.

---

# 78. REPEAT ONE + QUEUE EDIT

Repeat-one should continue repeating the current queue entry even if future entries are edited.

---

# 79. REPEAT ALL + QUEUE EDIT

If entries are added or removed:

the next cycle should reflect the updated queue.

---

# 80. ALBUM PLAYBACK

Selecting:

PLAY ALBUM

should generate queue entries from the album's tracks.

---

# 81. ALBUM ORDER

Album tracks should be ordered by:

disc number

then:

track number.

---

# 82. MISSING ALBUM TRACK

If an album track is unavailable:

skip it safely.

Do not crash the queue.

---

# 83. PLAY ARTIST

PLAY ARTIST should create a deterministic artist track sequence.

---

# 84. PLAYLIST PLAYBACK

Playlist order should be respected.

The queue should not automatically alphabetize playlist entries.

---

# 85. FOLDER PLAYBACK

If folder playback is supported:

sort using explicit deterministic rules.

Do not depend on filesystem enumeration order.

---

# 86. SEARCH PLAYBACK

Playing search results should preserve the search result ordering at queue creation time.

---

# 87. FAVORITES PLAYBACK

Favorites should use the defined favorite ordering.

---

# 88. RECENTLY PLAYED

Recently played queue generation should preserve the history query order.

---

# 89. SEARCH RESULT

Playing a search result may:

- start it immediately
- establish a queue context
- optionally preserve existing queue.

This behavior must be explicitly defined by the player UX.

---

# 90. USER-SELECTED TRACK

A direct selection from the library may replace or modify queue state depending on the player's configured behavior.

Recommended:

replace the current queue with the selected playback context when explicitly using PLAY.

ADD TO QUEUE remains additive.

---

# 91. COMMAND MODEL

Queue mutations should be command-driven.

Examples:

QUEUE_ADD

QUEUE_REMOVE

QUEUE_MOVE

QUEUE_CLEAR

QUEUE_PLAY_NEXT

QUEUE_SET_REPEAT

QUEUE_SET_SHUFFLE

QUEUE_SKIP_NEXT

QUEUE_SKIP_PREVIOUS.

---

# 92. SINGLE OWNER

Prefer one queue service as the authoritative owner of queue mutations.

---

# 93. QUEUE API

Conceptual API:

add(track_id)

insert(position, track_id)

remove(queue_entry_id)

clear()

play_next(track_id)

next()

previous()

current()

set_repeat(mode)

set_shuffle(enabled)

snapshot()

---

# 94. API RESULT

Mutating operations should return enough information for the caller to update UI state.

---

# 95. CONCURRENT MODIFICATION

The UI must not directly modify queue arrays from another thread.

Queue mutations must be synchronized with playback control.

---

# 96. AUDIO ENGINE ACCESS

The audio engine should request:

NEXT TRACK

or:

LOAD TRACK

from the queue service.

It should not directly manipulate queue internals.

---

# 97. QUEUE DECISION

The queue decides:

WHAT plays next.

The audio engine decides:

HOW to play it.

---

# 98. QUEUE AND LYRICS

The queue identifies the current track.

The lyrics system uses that track identity.

The queue does not parse lyrics.

---

# 99. QUEUE AND METADATA

The queue references track IDs.

Metadata is resolved separately.

---

# 100. QUEUE AND ARTWORK

Artwork is resolved separately through metadata/artwork systems.

---

# 101. QUEUE AND AUDIO DECODER

The queue never creates decoder buffers.

The playback engine performs decoder setup.

---

# 102. QUEUE AND AUDIO DEVICE

The queue never directly controls the TANCHJIM BUNNY DSP.

---

# 103. QUEUE AND DATABASE

The queue may persist its state through the storage layer.

It should not perform arbitrary SQL directly.

---

# 104. QUEUE TITLE

The UI may display:

Now Playing

Queue

Up Next.

These are presentation concepts.

---

# 105. QUEUE UI

The queue UI should receive snapshots/events.

It must not inspect queue internals.

---

# 106. QUEUE EVENTS

Recommended events:

QUEUE_CHANGED

CURRENT_ENTRY_CHANGED

QUEUE_EXHAUSTED

REPEAT_MODE_CHANGED

SHUFFLE_CHANGED.

---

# 107. PLAYBACK EVENTS

The queue may consume:

PLAYBACK_STARTED

PLAYBACK_FINISHED

PLAYBACK_ERROR

PLAYBACK_STOPPED.

---

# 108. EVENT ORDER

A normal automatic transition should conceptually be:

PLAYBACK_FINISHED

QUEUE RESOLVES NEXT

CURRENT_ENTRY_CHANGED

PLAYBACK_LOAD_REQUEST

PLAYBACK_STARTED.

---

# 109. PLAYBACK FAILURE

If a track fails to open/decode:

the queue should normally attempt the next playable entry.

---

# 110. FAILURE LOOP PROTECTION

The queue must not endlessly retry the same failed track.

---

# 111. FAILED ENTRY

A failed entry may be temporarily marked:

FAILED_FOR_SESSION.

---

# 112. NEXT AFTER FAILURE

Skip to the next eligible entry.

---

# 113. ALL TRACKS FAILED

If every remaining entry fails:

stop playback safely.

Report an appropriate error state.

---

# 114. FAILURE HISTORY

A playback failure should be logged.

It should not automatically be counted as a completed play.

---

# 115. MISSING FILE

If a queued file disappears:

mark the queue entry unavailable.

Skip it when necessary.

---

# 116. AUTOMATIC SKIP

Automatic skipping must have a termination condition.

---

# 117. SKIP LOOP PROTECTION

If every remaining queue entry fails:

do not loop indefinitely.

Stop and expose an appropriate error state.

---

# 118. INVALID TRACK

If a queue entry references a track that no longer exists:

the queue must handle it safely.

---

# 119. MISSING TRACK POLICY

Recommended behavior:

skip unavailable tracks during automatic playback.

The UI may show:

Track unavailable

rather than silently hiding the entry.

---

# 120. USER VISIBILITY

A broken queue entry must not crash playback.

---

# 121. INVALID QUEUE ENTRY

A broken queue entry must not crash playback.

---

# 122. TRANSITION FAILURE

If PLAYBACK_LOAD_REQUEST fails:

the queue may resolve the next entry.

---

# 123. ERROR DURING AUTO-ADVANCE

If a track fails to open:

queue resolution should attempt the next valid entry.

---

# 124. ERROR DURING MANUAL PLAY

If a manually selected track fails:

report the error.

Do not silently jump through unrelated tracks unless explicitly configured.

---

# 125. LIBRARY RESCAN

A library rescan must not silently reorder existing queue entries.

---

# 126. LIBRARY TRACK REMOVAL

If a library track is removed:

existing queue entries referencing it should become:

UNAVAILABLE.

---

# 127. LIBRARY TRACK RESTORED

If the track later becomes available again:

the queue entry may become playable again if its identity remains valid.

---

# 128. QUEUE STABILITY

A background library scan must not unexpectedly rebuild the active queue.

---

# 129. METADATA CHANGES

If metadata changes:

the queue entry should continue referencing the same track.

Its displayed metadata may update.

---

# 130. PATH CHANGES

If the library recognizes a moved file as the same track:

the queue entry remains valid.

---

# 131. UNKNOWN IDENTITY

If the library cannot establish continuity:

the queue entry should fail safely rather than playing a potentially incorrect file.

---

# 132. QUEUE ORDER AND LIBRARY SORT

Library sorting must not retroactively reorder a manually constructed queue.

---

# 133. QUEUE IS SNAPSHOT-BASED

When an album is added to the queue:

the queue captures the album's track selection/order at that time.

A later library sort change must not rearrange it.

---

# 134. QUEUE EDITING DURING PLAYBACK

All queue modifications must be safe while audio is playing.

---

# 135. PRELOAD

The queue may tell the playback engine which track is next.

The audio engine may optionally preload metadata or decoder state.

---

# 136. DECODER PRELOAD

Decoder preloading must not consume excessive Pi Zero W memory.

---

# 137. NO FULL QUEUE DECODING

Do not initialize decoders for every queued track.

---

# 138. QUEUE MEMORY

Queue entries should be lightweight.

Do not store full audio data in the queue.

Recommended queue entry memory:

small fixed metadata + track ID.

---

# 139. METADATA SNAPSHOT

A queue entry may store a small metadata snapshot for stable display.

It should not store large artwork buffers.

---

# 140. ARTWORK

Artwork should be loaded through the artwork/cache system.

The queue should reference artwork identity rather than storing decoded images.

---

# 141. LARGE QUEUE

A queue containing thousands of tracks should store lightweight references.

Do not duplicate complete metadata for every entry.

---

# 142. QUEUE LIMIT

The architecture should avoid unnecessary hard limits.

However, memory usage must remain bounded.

---

# 143. AUDIO BUFFERING

Queue management must remain independent of audio buffer sizes.

---

# 144. CROSSFADE

If crossfade is added later:

the queue remains responsible for track ordering.

The playback engine handles the actual overlap.

---

# 145. GAPLESS PLAYBACK

For gapless playback:

the queue provides the next track without changing its responsibility.

The playback engine handles decoder transition.

---

# 146. QUEUE PERSISTENCE

Queue persistence is optional.

If implemented:

store enough state to reconstruct:

- queue entries
- current entry
- playback order
- repeat mode
- shuffle state.

Queue persistence is useful for:

- application restart
- crash recovery
- user continuity.

---

# 147. PERSISTENCE POLICY

The current queue may be persisted periodically and at important mutations.

---

# 148. PERSISTENT QUEUE VS RESUME

Queue restoration and track resume are separate.

A restored queue does not automatically imply:

resume playback immediately.

---

# 149. QUEUE RESTORE

On startup:

load queue state

then:

validate track availability.

---

# 150. QUEUE RESTORE FAILURE

If the queue database is corrupted:

start with an empty queue.

Do not prevent normal application startup.

---

# 151. CURRENT QUEUE ENTRY RESTORE

If the previously current entry cannot be resolved:

the queue should select the next sensible playable entry or remain stopped.

---

# 152. SHUFFLE RESTORE

If shuffle persistence is enabled:

restore the playback order when valid.

Do not generate a new shuffle order every startup unless the policy explicitly says so.

---

# 153. REPEAT RESTORE

Repeat mode may be persisted as a player preference.

---

# 154. QUEUE CLEAR ON EXIT

Whether the queue survives restart should be an explicit product decision.

It must not be an accidental side effect of application shutdown.

---

# 155. DEFAULT QUEUE POLICY

Recommended initial behavior:

- queue survives during the current session
- queue persistence is optional
- resume persistence is independent.

---

# 156. WRITE FREQUENCY

Do not write the queue to storage on every playback-position update.

---

# 157. PERSIST ON MUTATION

Persist after:

- add
- remove
- reorder
- clear
- repeat change
- shuffle change.

---

# 158. CURRENT POSITION PERSISTENCE

Current queue position may be persisted.

---

# 159. PLAYBACK POSITION

Exact audio playback position belongs to the playback/resume system.

Do not store high-frequency position updates in the queue database.

---

# 160. CRASH RECOVERY

After an application crash:

the queue may be restored from its last consistent snapshot.

---

# 161. PARTIAL PERSISTENCE

If queue persistence fails:

the in-memory queue should continue operating.

---

# 162. PERSISTENCE FAILURE

Persistence failure must not stop audio playback.

---

# 163. ATOMIC PERSISTENCE

Queue state should be written using a safe update strategy.

Avoid leaving partially written queue state.

---

# 164. VERSIONED QUEUE DATA

Persistent queue data should contain a schema/version identifier.

---

# 165. MIGRATION

Future queue schema changes should support migration or safe invalidation.

---

# 166. PLAYBACK GENERATION

Each active playback session should have a generation identifier.

---

# 167. PURPOSE OF GENERATION

This prevents delayed asynchronous events from affecting the wrong queue entry.

---

# 168. EXAMPLE

Queue entry A:

generation 100.

User skips to B:

generation 101.

A delayed completion callback from A must not cause the queue to advance from B.

---

# 169. ATOMIC TRACK CHANGE

Track changes should be treated as a coordinated operation:

1. identify current queue entry
2. update playback generation
3. select next queue entry
4. resolve track
5. load playback engine
6. start playback
7. publish state.

---

# 170. TRACK LOAD FAILURE

If step 5 fails:

the queue attempts the next candidate.

---

# 171. USER SKIP DURING LOAD

If the user presses NEXT while a track is loading:

the current load operation must be cancellable or invalidated.

---

# 172. STALE LOAD

A track load associated with an old playback generation must not become active after a newer command.

---

# 173. RACE CONDITION PROTECTION

The queue and playback engine must prevent:

NEXT

then:

PREVIOUS

from resulting in the older asynchronous load winning.

---

# 174. COMMAND SERIALIZATION

Playback-order-changing commands should be serialized through the playback/queue control path.

Track transitions should be serialized through the playback control system.

---

# 175. USER INPUT

Rapid NEXT presses should not create uncontrolled concurrent decoder loads.

---

# 176. RAPID NEXT

The system should converge on the latest valid requested queue entry.

---

# 177. RAPID PREVIOUS

Same rule applies to PREVIOUS.

---

# 178. CONCURRENT NEXT COMMANDS

Multiple NEXT commands must be serialized.

---

# 179. DOUBLE PRESS

If NEXT is pressed twice quickly:

the behavior must be deterministic.

The queue should process commands in order or coalesce them according to the input system.

---

# 180. CURRENT TRACK REPLACEMENT

Selecting another track while playback is active creates a new playback generation.

---

# 181. STALE EVENT PROTECTION

Events should carry enough context to ensure they belong to the current playback generation.

---

# 182. QUEUE EMPTY

An empty queue is a valid state.

It must not be treated as an application error.

The UI should display an explicit empty state.

---

# 183. END OF QUEUE

End of queue is also a valid state.

Playback may stop normally.

---

# 184. AUTO-ADVANCE

Auto-advance occurs only when:

the current track genuinely reaches completion.

Automatic advance must happen only after the playback engine confirms track completion.

---

# 185. STOP

STOP should stop playback.

It does not necessarily clear the queue.

---

# 186. PAUSE

Pause must not advance the queue.

PAUSE does not alter queue position.

---

# 187. SEEK TO END

Seeking to the end must use the playback engine's completion semantics.

Do not infer completion solely from a UI slider value.

---

# 188. PLAYBACK ERROR VS COMPLETION

Decoder failure:

PLAYBACK_ERROR.

Natural end:

TRACK_COMPLETED.

They are different events.

---

# 189. HISTORY INTERACTION

The queue informs playback history when a track meaningfully starts/completes.

History does not decide queue order.

---

# 190. RESUME INTERACTION

Resume persistence saves the current queue entry's track position.

It does not decide what the next queue entry is.

---

# 191. ACCEPTANCE TEST: NORMAL ORDER

Create:

A B C.

Play A.

Verify:

NEXT B.

NEXT C.

---

# 192. ACCEPTANCE TEST: ADD

Add three tracks.

Verify:

A

B

C.

---

# 193. ACCEPTANCE TEST: DUPLICATE

Queue:

A A B.

Verify:

both A entries remain independently playable.

---

# 194. ACCEPTANCE TEST: PLAY NEXT

Current:

A.

Use PLAY NEXT with C.

Verify:

A

C

B.

---

# 195. ACCEPTANCE TEST: REMOVE

Queue:

A B C.

Remove B.

Verify:

A

C.

---

# 196. ACCEPTANCE TEST: REORDER

Move C before A.

Verify:

C

A

B.

---

# 197. ACCEPTANCE TEST: NEXT

Current:

A.

Press NEXT.

Verify:

B becomes current.

---

# 198. ACCEPTANCE TEST: PREVIOUS

Play C.

Press PREVIOUS.

Verify:

B becomes current.

---

# 199. ACCEPTANCE TEST: PREVIOUS THRESHOLD

Play B.

Seek to the middle.

Press PREVIOUS.

Verify:

B restarts instead of moving to A.

---

# 200. ACCEPTANCE TEST: REPEAT OFF

Queue:

A B C.

Finish C.

Verify:

playback stops.

---

# 201. ACCEPTANCE TEST: REPEAT ONE

Enable repeat-one.

Finish A.

Verify:

A plays again.

---

# 202. ACCEPTANCE TEST: REPEAT ALL

Queue:

A B C.

Finish C.

Verify:

A plays.

---

# 203. ACCEPTANCE TEST: SHUFFLE

Queue:

A B C D E.

Enable shuffle.

Verify:

all entries appear exactly once before the cycle repeats.

---

# 204. ACCEPTANCE TEST: SHUFFLE CURRENT TRACK

Start C.

Enable shuffle.

Verify:

C remains playing.

---

# 205. ACCEPTANCE TEST: SHUFFLE DISABLE

Disable shuffle during playback.

Verify:

current track remains unchanged.

---

# 206. ACCEPTANCE TEST: REMOVE FUTURE

Queue:

A B C.

Play A.

Remove C.

Verify:

B remains next.

---

# 207. ACCEPTANCE TEST: CLEAR QUEUE

Queue:

A B C.

Play A.

Clear queue.

Verify:

A continues according to clear-queue policy and B/C are not played afterward.

---

# 208. ACCEPTANCE TEST: MISSING FILE

Queue:

A B C.

Delete B.

Finish A.

Verify:

B is skipped safely and C plays.

---

# 209. ACCEPTANCE TEST: PLAYBACK FAILURE

Make B undecodable.

Finish A.

Verify:

B fails gracefully and C is attempted.

---

# 210. ACCEPTANCE TEST: ALL FAILED

Make all remaining tracks unavailable.

Verify:

playback stops cleanly.

---

# 211. ACCEPTANCE TEST: LIBRARY RESCAN

Start playback.

Run a library scan.

Verify:

queue order remains unchanged.

---

# 212. ACCEPTANCE TEST: METADATA CHANGE

Change the title of the current track.

Rescan.

Verify:

playback continues and the queue entry remains valid.

---

# 213. ACCEPTANCE TEST: RAPID NEXT

Rapidly press NEXT several times.

Verify:

the final valid requested track becomes current without stale loads overriding it.

---

# 214. ACCEPTANCE TEST: RAPID NEXT/PREVIOUS

Rapidly alternate:

NEXT

PREVIOUS

NEXT.

Verify:

the final command wins deterministically.

---

# 215. ACCEPTANCE TEST: STALE EVENT

Generate a delayed event from an old playback generation.

Verify:

it cannot modify the current track.

---

# 216. ACCEPTANCE TEST: QUEUE RESTORE

Persist a queue.

Restart FLACHEAD.

Verify:

the queue can be reconstructed if queue persistence is enabled.

---

# 217. ACCEPTANCE TEST: CORRUPTED QUEUE

Corrupt the stored queue state.

Restart.

Verify:

FLACHEAD starts safely with an empty/recoverable queue.

---

# 218. ACCEPTANCE TEST: PERSISTENCE FAILURE

Simulate storage failure.

Verify:

playback continues using in-memory queue state.

---

# 219. ACCEPTANCE TEST: LARGE QUEUE

Create a queue containing thousands of lightweight track references.

Verify:

memory usage remains reasonable.

---

# 220. ACCEPTANCE CRITERIA

The queue architecture is production-ready when:

- queue state is independent of the library
- duplicate tracks are supported
- queue entries have independent identities
- next/previous behavior is deterministic
- repeat-off works
- repeat-one works
- repeat-all works
- shuffle works
- shuffle does not unexpectedly restart the current track
- shuffle does not corrupt logical ordering
- queue edits are safe during playback
- missing files are skipped safely
- decoder failures do not create infinite retry loops
- rapid user commands cannot produce stale playback
- track transitions are serialized
- stale playback events are rejected
- library rescans do not reorder the queue
- metadata changes do not unnecessarily invalidate queue entries
- queue memory usage remains bounded
- queue persistence failure does not stop playback
- queue persistence is optional and recoverable
- large queues remain memory-efficient
- active playback remains independent from UI rendering
- playback generation prevents stale asynchronous events
- empty queues are handled normally
- end-of-queue behavior is explicit
- queue restoration is independent of track resume
- lyrics remain independent
- audio decoding remains independent
- the TANCHJIM BUNNY DSP remains outside queue ownership.

---

# 221. AI CODING AGENT RULES

The coding agent MUST:

1. Keep queue state separate from library state.
2. Keep queue state separate from decoder state.
3. Keep queue state separate from audio output.
4. Treat the queue as playback intent.
5. Represent queue entries independently from track identity.
6. Allow duplicate track IDs in different queue entries.
7. Give each queue occurrence its own entry identity.
8. Store track IDs rather than complete track objects.
9. Preserve explicit queue ordering.
10. Preserve logical queue ordering while shuffled.
11. Never rely on filesystem enumeration order.
12. Implement explicit repeat modes.
13. Implement deterministic shuffle.
14. Use an unbiased shuffle algorithm.
15. Preserve the current track when toggling shuffle.
16. Prevent shuffle from duplicating or omitting entries.
17. Serialize queue mutations.
18. Serialize track transitions.
19. Protect against stale asynchronous playback events.
20. Use playback generations for track transitions.
21. Prevent stale decoder loads from becoming active.
22. Protect against duplicate transition commands.
23. Skip unavailable tracks safely.
24. Bound failed-track retries.
25. Prevent infinite automatic-skip loops.
26. Keep queue edits safe during playback.
27. Never let a library rescan silently rebuild the active queue.
28. Never let library sorting reorder an existing queue.
29. Keep queue persistence independent from resume persistence.
30. Never block the audio output thread on queue database writes.
31. Never let queue persistence failure stop playback.
32. Never let queue errors crash playback.
33. Keep queue entries lightweight.
34. Never store decoded audio inside queue objects.
35. Never store large artwork buffers inside queue entries.
36. Avoid unnecessary metadata duplication.
37. Keep queue memory usage appropriate for the Pi Zero W.
38. Treat manual STOP differently from natural completion.
39. Treat decoder errors differently from normal completion.
40. Make rapid NEXT/PREVIOUS operations deterministic.
41. Keep empty queue and end-of-queue states valid.
42. Make the queue recoverable after database corruption or application failure.
43. Keep UI access snapshot/event based.
44. Never make the queue directly control the TANCHJIM BUNNY DSP.
45. Never make lyrics responsible for queue progression.
46. Never let a metadata change invalidate queue membership.
47. Make playback transitions deterministic.
48. Keep queue persistence separate from audio timing.
49. Never persist high-frequency playback-position updates through the queue system.

---

# 222. FINAL QUEUE ARCHITECTURE

The intended architecture is:

                         LIBRARY SERVICE
                               │
                               ▼
                       USER SELECTION
                               │
                               ▼
                     +------------------+
                     | QUEUE SERVICE    |
                     |                  |
                     | Canonical Queue  |
                     | Playback Order   |
                     | Repeat Mode      |
                     | Shuffle State    |
                     | Current Entry    |
                     +------------------+
                               │
                               ▼
                     PLAYBACK CONTROL
                               │
                               ▼
                     +------------------+
                     | PLAYBACK ENGINE  |
                     +------------------+
                               │
                               ▼
                           DECODER
                               │
                               ▼
                              PCM
                               │
                               ▼
                         BUNNY DSP


The queue maintains two conceptual orders:

CANONICAL ORDER

A B C D E

and, when shuffle is active:

PLAYBACK ORDER

C A E B D

The current queue entry is tracked independently from the track ID so that:

A B A

remains three distinct queue entries.

The central invariant is:

THE QUEUE DECIDES WHAT PLAYS NEXT; THE PLAYBACK ENGINE DECIDES HOW IT PLAYS.

The second invariant is:

LIBRARY RESCANS MUST NEVER SILENTLY REORDER AN EXISTING QUEUE.

The third invariant is:

A QUEUE ENTRY MAY REFERENCE A TRACK, BUT IT IS NOT THE TRACK ITSELF.

The fourth invariant is:

STALE ASYNCHRONOUS PLAYBACK EVENTS MUST NEVER OVERRIDE NEWER USER COMMANDS.

The fifth invariant is:

A MISSING OR UNDECODABLE TRACK MUST NEVER CRASH OR TRAP THE PLAYBACK ENGINE IN AN INFINITE RETRY LOOP.

The sixth invariant is:

PLAYBACK POSITION NEVER BELONGS TO THE QUEUE.

The seventh invariant is:

QUEUE FAILURE MUST NOT CRASH THE AUDIO ENGINE.

Final policy:

Keep the queue authoritative for playback order.

Keep the library authoritative for available music.

Keep the playback engine authoritative for active audio.

Keep shuffle deterministic.

Keep repeat explicit.

Keep duplicate queue entries valid.

Keep queue edits safe during playback.

Keep asynchronous transitions generation-safe.

Keep queue persistence optional and independent.

Keep queue state lightweight.

Use stable track IDs.

Allow duplicates.

Separate logical order from shuffled playback order.

Serialize track transitions.

Support repeat and shuffle explicitly.

Handle missing files safely.

Persist queue state only at sensible mutation points.

Never persist high-frequency audio timing through the queue.

Keep queue, metadata, lyrics, decoder, and DSP responsibilities separate.

And always prioritize:

AUDIO PLAYBACK > PLAYBACK CONTROL > UI RESPONSIVENESS > BACKGROUND QUEUE/PERSISTENCE WORK.

And always preserve the separation:

LIBRARY = AVAILABLE TRACKS

PLAYLIST = USER-CREATED COLLECTION

QUEUE = PLAYBACK INTENT

PLAYBACK ENGINE = AUDIO EXECUTION

DECODER = AUDIO TRANSFORMATION

AUDIO OUTPUT = HARDWARE DELIVERY
