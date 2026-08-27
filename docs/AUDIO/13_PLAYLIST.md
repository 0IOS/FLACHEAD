# FLACHEAD AUDIO PLAYLIST ARCHITECTURE

Document

22_AUDIO_PLAYLIST_ARCHITECTURE.md

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

This document defines the architecture for persistent user-created playlists in FLACHEAD.

The playlist system covers:

- playlist creation
- playlist naming
- playlist deletion
- track insertion
- track removal
- playlist ordering
- duplicate tracks
- playlist editing
- playlist playback
- playlist-to-queue conversion
- missing tracks
- library changes
- metadata changes
- persistence
- corruption recovery
- import/export considerations
- UI interaction boundaries.

The primary objective is:

PROVIDE A SIMPLE, RELIABLE, OFFLINE PLAYLIST SYSTEM THAT STORES USER INTENT WITHOUT COUPLING PLAYLISTS TO CURRENT FILE PATHS OR CURRENT LIBRARY SORTING.

---

# 2. CORE PRINCIPLE

A playlist is:

PERSISTENT USER ORGANIZATION.

A queue is:

TEMPORARY PLAYBACK ORDER.

A library is:

THE CURRENTLY DISCOVERED MUSIC COLLECTION.

These are separate concepts.

---

# 3. ARCHITECTURAL RELATIONSHIP

The intended relationship is:

                    LIBRARY
                       │
                       ▼
                  TRACK IDs
                       │
                       ▼
                +-------------+
                | PLAYLIST    |
                +-------------+
                       │
                       ▼
                QUEUE CREATION
                       │
                       ▼
                PLAYBACK ENGINE

A playlist does not directly control the decoder.

---

# 4. PLAYLIST OWNERSHIP

The playlist service owns:

- playlist records
- playlist names
- playlist ordering
- playlist membership
- playlist metadata
- playlist modification timestamps.

The library service owns:

- track existence
- track metadata
- track availability
- track identity.

The queue service owns:

- active playback order
- current queue entry
- shuffle
- repeat.

---

# 5. PLAYLIST IDENTITY

Every playlist must have a stable:

playlist_id.

The ID must not depend on:

- playlist name
- creation path
- track count.

---

# 6. PLAYLIST NAME

A playlist has a user-visible:

name.

Names are presentation data.

The playlist ID remains the actual identity.

---

# 7. PLAYLIST NAME UNIQUENESS

Playlist names do not need to be globally unique unless the product explicitly requires it.

If duplicate names are allowed:

the UI must still be able to distinguish playlists reliably.

---

# 8. RECOMMENDED NAME POLICY

For simplicity:

playlist names should be unique within the user's playlist collection.

If a requested name already exists:

the UI may request another name.

---

# 9. PLAYLIST RECORD

Conceptual model:

Playlist

    playlist_id
    name
    created_at
    modified_at
    track_count

---

# 10. PLAYLIST ENTRY

Conceptual model:

PlaylistEntry

    playlist_entry_id
    playlist_id
    track_id
    position
    added_at

---

# 11. ENTRY IDENTITY

Every playlist entry should have its own identity.

This allows the same track to appear multiple times if the product permits duplicates.

---

# 12. DUPLICATE TRACKS

Recommended initial policy:

allow duplicate tracks.

A playlist may contain:

A

B

A

C.

This preserves explicit user intent.

---

# 13. OPTIONAL DUPLICATE PREVENTION

If the UI later provides:

"Do not add duplicates"

that should be a user-facing policy.

It should not be an implicit database constraint.

---

# 14. PLAYLIST ORDER

Playlist order is explicit.

It must not depend on:

- library sort order
- filename order
- album sort
- filesystem order.

---

# 15. POSITION

Each playlist entry has a logical position.

Example:

1. Track A
2. Track B
3. Track C

---

# 16. ORDER MODIFICATION

When a track is moved:

positions must be updated deterministically.

---

# 17. MOVE OPERATION

Supported operations should include:

MOVE_TO_TOP

MOVE_UP

MOVE_DOWN

MOVE_TO_BOTTOM

or:

MOVE_TO_POSITION.

---

# 18. INSERTION

Adding a track normally appends it to the playlist.

The UI may optionally provide:

ADD_TO_TOP

or:

INSERT_AT_POSITION.

---

# 19. REMOVE

Removing a track removes the playlist entry.

It must not delete the underlying music file.

---

# 20. PLAYLIST DELETE

Deleting a playlist deletes:

playlist metadata

and:

playlist membership records.

It must never delete music files.

---

# 21. LIBRARY DELETE VS PLAYLIST DELETE

Deleting a playlist:

does not delete music.

Deleting a music file:

may make a playlist entry unavailable.

---

# 22. TRACK ID REFERENCE

Playlist entries should reference:

track_id.

They should not depend solely on:

absolute file paths.

---

# 23. PATH CHANGES

If a file is moved and the library preserves its track_id:

the playlist entry remains valid automatically.

---

# 24. METADATA CHANGES

If a track title or album changes:

the playlist membership remains unchanged.

The UI simply displays the current metadata.

---

# 25. TRACK REMOVAL

If a track disappears permanently:

the playlist entry should become:

UNAVAILABLE.

---

# 26. UNAVAILABLE ENTRY

An unavailable entry should retain:

playlist position

track identity

and:

historical metadata if useful.

---

# 27. WHY RETAIN UNAVAILABLE ENTRIES

This allows:

- removable storage
- temporary missing files
- library reconciliation
- restoration when storage returns.

---

# 28. RESTORED TRACK

If the library later resolves the same track_id:

the playlist entry becomes playable again.

---

# 29. UNKNOWN TRACK

If identity cannot be recovered:

the entry remains unavailable.

Do not automatically replace it with a similarly named file.

---

# 30. FUZZY MATCHING

Automatic fuzzy replacement is prohibited by default.

For example:

Playlist:

Song A

Missing file:

Song A

New file:

Song A

The system must not assume they are identical merely because their names match.

---

# 31. PLAYLIST PLAYBACK

When the user selects:

PLAY PLAYLIST

the playlist is converted into queue entries.

---

# 32. PLAYLIST DOES NOT BECOME QUEUE

The playlist itself remains unchanged.

The queue is a playback snapshot derived from it.

---

# 33. QUEUE SNAPSHOT

At playlist playback time:

playlist entries

↓

ordered track IDs

↓

queue entries.

---

# 34. PLAYLIST MODIFICATION DURING PLAYBACK

Editing the playlist must not automatically reorder the current queue.

---

# 35. EXAMPLE

Playlist:

A B C

Start playback.

Queue:

A B C

Then edit playlist:

A C D.

The active queue should remain:

A B C

unless the user explicitly requests queue synchronization.

---

# 36. WHY

The queue represents:

what the user currently intends to hear.

The playlist represents:

what the user wants saved for later.

---

# 37. PLAY NEXT FROM PLAYLIST

Selecting a playlist track with:

PLAY NEXT

should insert it into the active queue.

It should not modify the playlist unless the user explicitly chooses:

ADD TO PLAYLIST.

---

# 38. ADD TO PLAYLIST

Adding a track to a playlist is a persistent operation.

It should not automatically alter the current queue.

---

# 39. PLAYLIST CONTEXT

The UI may expose:

PLAY

PLAY NEXT

ADD TO QUEUE

REMOVE FROM PLAYLIST.

These must remain distinct actions.

---

# 40. PLAYLIST ORDER DURING QUEUE CREATION

The generated queue should initially follow:

playlist order.

---

# 41. SHUFFLE

If shuffle is active:

the queue may create a shuffled playback order.

The playlist itself remains unchanged.

---

# 42. SHUFFLE INVARIANT

Shuffling a playlist for playback must NEVER rewrite the playlist's stored order.

---

# 43. REPEAT

Repeat behavior belongs to the queue/playback system.

The playlist does not own repeat state.

---

# 44. PLAYLIST LOOP

A playlist may be played repeatedly through:

QUEUE REPEAT ALL.

This does not require modifying the playlist.

---

# 45. PLAYLIST SIZE

Playlists may contain:

one

or:

many

tracks.

An empty playlist is valid.

---

# 46. EMPTY PLAYLIST

An empty playlist should display a clear empty state.

---

# 47. EMPTY PLAYLIST PLAYBACK

Attempting to play an empty playlist should:

do nothing

and:

show an appropriate UI state.

It must not crash.

---

# 48. PLAYLIST SORTING

Playlist order is user-defined.

Do not automatically alphabetize entries.

---

# 49. ALBUM ART

Playlist records should not store decoded artwork.

Artwork should be resolved through the existing artwork system.

---

# 50. PLAYLIST COVER ART

If the UI displays a playlist image:

it may use:

- first track artwork
- user-selected artwork
- generated collage.

The exact presentation is a UI policy.

---

# 51. ARTWORK FALLBACK

If no playlist artwork exists:

use the defined generic playlist placeholder.

---

# 52. DATABASE MODEL

A practical database can contain:

playlists

playlist_entries.

---

# 53. PLAYLISTS TABLE

Conceptual fields:

playlist_id

name

created_at

modified_at.

---

# 54. PLAYLIST ENTRIES TABLE

Conceptual fields:

playlist_entry_id

playlist_id

track_id

position

added_at.

---

# 55. FOREIGN KEYS

Playlist entries should reference their parent playlist.

---

# 56. TRACK REFERENCES

Playlist entries reference library track IDs.

---

# 57. DELETION POLICY

Deleting a playlist should cascade only to:

playlist_entries.

It must not cascade into:

tracks.

---

# 58. TRACK DELETION

Deleting a track from the library must not blindly delete playlist entries if the architecture supports temporary unavailability.

---

# 59. DATABASE TRANSACTION

Playlist mutations should use transactions.

Example:

moving an entry may modify several positions.

These changes should be atomic.

---

# 60. ATOMIC MOVE

A move operation should never leave:

two entries with the same position

or:

large unintended position gaps

unless the implementation explicitly uses sparse positions.

---

# 61. POSITION STRATEGY

A simple contiguous integer position model is sufficient initially.

---

# 62. LARGE PLAYLIST MOVE

For large playlists:

batch position updates in a single transaction.

---

# 63. WRITE FREQUENCY

Playlist editing generates relatively infrequent writes compared with playback position updates.

Correctness is therefore more important than aggressive write minimization.

---

# 64. SD-CARD IMPACT

Even so:

avoid writing repeatedly while the user is dragging a track through the list.

---

# 65. DRAG REORDER

If drag-and-drop reordering is supported:

persist only after the drag operation is committed.

Do not persist every pointer movement.

---

# 66. PLAYLIST RENAMING

Rename should be:

atomic

and:

persisted only after the user confirms the new name.

---

# 67. INVALID NAME

The playlist service should reject:

- empty names
- names exceeding configured limits
- invalid control characters.

---

# 68. NAME NORMALIZATION

Avoid destructive normalization.

A user-visible playlist name should remain as entered except for required validation.

---

# 69. RESERVED NAMES

The implementation may reserve special names such as:

Favorites

if those are implemented as system collections.

---

# 70. SYSTEM COLLECTIONS

If FLACHEAD provides built-in collections:

they should not necessarily be ordinary user playlists.

Examples:

Favorites

Recently Played

Most Played.

---

# 71. FAVORITES

Favorites are logically a collection of track IDs.

They may be implemented separately from user playlists.

---

# 72. RECENTLY PLAYED

Recently Played should come from playback history.

It should not be manually maintained as a playlist.

---

# 73. MOST PLAYED

Most Played should be derived from play statistics.

It should not require duplicating tracks into a playlist.

---

# 74. PLAYLIST SERVICE API

Conceptual operations:

create_playlist()

delete_playlist()

rename_playlist()

get_playlists()

get_playlist()

add_track()

remove_entry()

move_entry()

clear_playlist()

---

# 75. PLAYBACK API

Playlist playback should expose:

play_playlist(playlist_id)

or an equivalent service operation.

---

# 76. QUEUE API

The playlist service should provide queue-ready track references.

The queue service then owns the active playback snapshot.

---

# 77. NO DIRECT UI DATABASE ACCESS

The UI must not directly manipulate:

playlist tables.

---

# 78. COMMAND FLOW

Recommended:

UI

↓

Playlist Service

↓

Database

↓

Playlist State

↓

Queue Service

when playback is requested.

---

# 79. ERROR HANDLING

Playlist errors should be classified.

Examples:

PLAYLIST_NOT_FOUND

INVALID_NAME

TRACK_NOT_FOUND

DATABASE_ERROR

INVALID_POSITION

DUPLICATE_POLICY_ERROR.

---

# 80. PLAYLIST NOT FOUND

If a playlist ID does not exist:

return a controlled error.

Do not crash.

---

# 81. TRACK NOT FOUND

If a track ID cannot be resolved:

create/retain an unavailable playlist entry if appropriate.

---

# 82. DATABASE FAILURE

If playlist persistence fails:

the current application must remain usable.

---

# 83. DEFERRED PERSISTENCE

For simple playlist operations:

synchronous transactional persistence is acceptable.

For UI-heavy batch editing:

changes may be staged and committed once.

---

# 84. EDIT SESSION

A future implementation may use:

playlist edit session

where multiple changes are accumulated before commit.

---

# 85. CANCEL EDIT

If edit mode supports transactions:

CANCEL

should discard uncommitted playlist changes.

---

# 86. SAVE EDIT

SAVE

commits all changes atomically.

---

# 87. SIMPLE INITIAL IMPLEMENTATION

The initial implementation may commit each explicit user action immediately.

This is simpler and reliable for the Pi Zero W.

---

# 88. PLAYLIST IMPORT

Importing playlists may be supported later.

The core architecture must not depend on external playlist formats.

---

# 89. PLAYLIST EXPORT

Export may be supported later.

The internal playlist representation must remain independent of export format.

---

# 90. M3U SUPPORT

If M3U/M3U8 support is added:

external paths should be resolved conservatively.

Do not assume an imported path is equivalent to a current track ID.

---

# 91. IMPORT RESOLUTION

Import resolution priority should be:

1. known track identity where available
2. exact normalized path
3. conservative metadata/path matching
4. unresolved entry.

---

# 92. UNRESOLVED IMPORT

An unresolved imported track should not silently map to another track.

---

# 93. EXPORT

Export should preserve playlist order.

---

# 94. EXPORT MISSING TRACKS

The export policy for unavailable entries must be explicit.

Recommended:

omit unavailable entries and report them.

---

# 95. PLAYLIST BACKUP

Because playlists are user-created state:

they should be included in any future FLACHEAD configuration/data backup strategy.

---

# 96. DATABASE CORRUPTION

If the playlist database is corrupted:

FLACHEAD should attempt recovery.

---

# 97. RECOVERY PRIORITY

Do not let a corrupt playlist prevent:

- application startup
- library access
- audio playback.

---

# 98. RECOVERY OPTION

If recovery fails:

initialize a clean playlist database.

The underlying music library remains untouched.

---

# 99. BACKUP

A lightweight database backup may be maintained before destructive schema migrations.

---

# 100. MIGRATIONS

Playlist schema changes must use explicit migrations.

Do not silently reinterpret old database records.

---

# 101. VERSIONING

The playlist schema should have a version.

---

# 102. MIGRATION FAILURE

If migration fails:

preserve the old database where possible.

Do not overwrite it with a broken schema.

---

# 103. CONCURRENCY

Playlist operations may occur while:

- music is playing
- library scanning is running
- history is being written.

---

# 104. PLAYBACK CONCURRENCY

Playlist editing must not block active audio.

---

# 105. LIBRARY SCAN CONCURRENCY

A library scan may change track availability while a playlist is open.

The playlist should reflect availability updates safely.

---

# 106. UI STALE DATA

If a playlist screen is open while another operation changes it:

the UI must refresh from authoritative playlist state.

---

# 107. MODIFICATION GENERATION

A playlist may maintain:

modification_generation.

This allows stale UI updates to be rejected.

---

# 108. PLAYLIST UPDATE EVENT

Useful events:

PLAYLIST_CREATED

PLAYLIST_RENAMED

PLAYLIST_UPDATED

PLAYLIST_DELETED

PLAYLIST_TRACK_ADDED

PLAYLIST_TRACK_REMOVED

PLAYLIST_REORDERED.

---

# 109. EVENT OWNERSHIP

The playlist service publishes these events.

The UI subscribes.

The queue may react only when playback has explicitly requested a playlist snapshot.

---

# 110. NO AUTOMATIC QUEUE MUTATION

A playlist update must not automatically mutate the active queue.

---

# 111. EXAMPLE

User is playing:

A B C.

Playlist is edited from:

A B C

to:

A D C.

The current queue remains:

A B C.

---

# 112. REPLAY PLAYLIST

If the user selects:

PLAY PLAYLIST

again:

a new queue snapshot is created from the current playlist.

---

# 113. PLAYLIST SNAPSHOT CONSISTENCY

Queue creation should read a consistent playlist state.

---

# 114. ATOMIC SNAPSHOT

The queue should receive:

one ordered snapshot

rather than issuing multiple independent playlist queries.

---

# 115. LARGE PLAYLISTS

For very large playlists:

avoid loading unnecessary metadata.

The queue initially needs:

track IDs

and:

minimal display metadata.

---

# 116. PAGINATION

The playlist UI should support bounded loading for large playlists.

---

# 117. MEMORY

Do not decode artwork or audio for every playlist entry.

---

# 118. SEARCH INSIDE PLAYLIST

If supported:

search should operate on playlist entries without modifying the playlist.

---

# 119. FILTERING

Filtering a playlist view should not delete hidden entries.

---

# 120. SORTING VIEW

Temporary sorting of a playlist view should not modify persistent playlist order unless explicitly requested.

---

# 121. USER INTENT

The distinction is:

VIEW SORT

versus:

REORDER PLAYLIST.

These must not be confused.

---

# 122. PLAYLIST CLEAR

CLEAR PLAYLIST removes all entries.

It does not delete the playlist object.

---

# 123. DELETE PLAYLIST

DELETE PLAYLIST removes the playlist itself.

---

# 124. CONFIRMATION

Destructive playlist operations should require explicit user confirmation where appropriate.

---

# 125. DELETE TRACK FROM PLAYLIST

This should be less destructive than:

DELETE FILE.

The UI must distinguish the two clearly.

---

# 126. OFFLINE OPERATION

All playlist operations must work without:

- internet
- account
- cloud database
- remote service.

---

# 127. DATA OWNERSHIP

Playlist data belongs to the local FLACHEAD user.

It should not be uploaded automatically.

---

# 128. ACCEPTANCE TEST: CREATE

Create playlist:

My Music.

Verify:

playlist_id is generated and persisted.

---

# 129. ACCEPTANCE TEST: ADD

Add:

A B C.

Verify:

playlist contains:

A B C.

---

# 130. ACCEPTANCE TEST: ORDER

Move C to the top.

Verify:

C A B.

---

# 131. ACCEPTANCE TEST: REMOVE

Remove A.

Verify:

C B.

---

# 132. ACCEPTANCE TEST: DUPLICATE

Add B again.

Verify:

C B B

if duplicate tracks are enabled.

---

# 133. ACCEPTANCE TEST: PLAY

Play playlist.

Verify:

queue follows playlist order.

---

# 134. ACCEPTANCE TEST: SHUFFLE

Play playlist with shuffle.

Verify:

playlist order remains unchanged.

---

# 135. ACCEPTANCE TEST: MISSING TRACK

Remove B from storage.

Verify:

playlist remains intact and B becomes unavailable.

---

# 136. ACCEPTANCE TEST: RESTORE TRACK

Restore the same track identity.

Verify:

B becomes playable again.

---

# 137. ACCEPTANCE TEST: METADATA CHANGE

Change B's title.

Rescan.

Verify:

playlist still contains B and displays updated metadata.

---

# 138. ACCEPTANCE TEST: DELETE PLAYLIST

Delete playlist.

Verify:

playlist disappears.

Verify:

music files remain untouched.

---

# 139. ACCEPTANCE TEST: DATABASE FAILURE

Break playlist persistence.

Verify:

FLACHEAD still starts and can play music.

---

# 140. ACCEPTANCE TEST: LARGE PLAYLIST

Create a large playlist.

Verify:

playlist screen remains responsive.

---

# 141. ACCEPTANCE TEST: RESCAN

Run library scan while playlist is open.

Verify:

playlist membership is not unexpectedly reordered.

---

# 142. ACCEPTANCE TEST: QUEUE ISOLATION

Play playlist.

Modify playlist.

Verify:

active queue does not automatically change.

---

# 143. ACCEPTANCE TEST: RESTART

Create a playlist.

Restart FLACHEAD.

Verify:

playlist remains available.

---

# 144. ACCEPTANCE TEST: CORRUPTION

Corrupt playlist database.

Restart.

Verify:

FLACHEAD remains usable and playlist recovery/fallback occurs.

---

# 145. ACCEPTANCE CRITERIA

The playlist architecture is production-ready when:

- playlists have stable IDs
- playlist names are validated
- playlist order is persistent
- duplicate tracks are handled deterministically
- playlist entries reference track IDs
- playlist deletion never deletes music
- missing tracks remain safely represented
- restored tracks can become playable again
- metadata changes do not break membership
- playlist playback creates a queue snapshot
- shuffle does not alter playlist order
- repeat belongs to the queue
- active queues remain independent from later playlist edits
- playlist operations are transactional
- playlist database failures do not stop audio
- playlist data survives restart
- large playlists remain memory-bounded
- UI does not directly manipulate database tables
- playlist sorting and playlist reordering remain distinct concepts
- offline operation is fully supported.

---

# 146. AI CODING AGENT RULES

The coding agent MUST:

1. Treat playlists as persistent user organization.
2. Keep playlists separate from queues.
3. Keep playlists separate from library scanning.
4. Assign stable playlist IDs.
5. Assign stable playlist-entry IDs.
6. Reference tracks by track_id.
7. Never use title alone as track identity.
8. Preserve explicit playlist order.
9. Never use filesystem order as playlist order.
10. Allow duplicate entries unless an explicit duplicate policy says otherwise.
11. Never delete music when deleting a playlist.
12. Preserve unavailable entries when appropriate.
13. Never use fuzzy matching to silently replace missing tracks.
14. Generate queue snapshots from playlist state.
15. Never modify playlist order when shuffling playback.
16. Never automatically synchronize playlist edits into an active queue.
17. Keep queue mutations under queue-service ownership.
18. Use database transactions for playlist mutations.
19. Keep UI access through a playlist service.
20. Keep large playlist queries bounded.
21. Avoid decoding artwork for every playlist entry.
22. Avoid loading unnecessary metadata into RAM.
23. Keep playlist persistence independent from resume persistence.
24. Keep playlist persistence failure non-fatal.
25. Support schema migrations.
26. Make database corruption recoverable.
27. Keep playlist operations offline.
28. Emit playlist change events.
29. Protect against stale UI updates.
30. Never allow playlist state to block active audio playback.

---

# 147. FINAL PLAYLIST ARCHITECTURE

The intended architecture is:

                    MUSIC LIBRARY
                          │
                          ▼
                       TRACK ID
                          │
                          ▼
                 +----------------+
                 | PLAYLIST       |
                 | SERVICE        |
                 +----------------+
                    │          │
                    │          ▼
                    │      DATABASE
                    │
                    ▼
              ORDERED SNAPSHOT
                    │
                    ▼
                QUEUE SERVICE
                    │
                    ▼
              PLAYBACK ENGINE
                    │
                    ▼
                  DECODER
                    │
                    ▼
                 BUNNY DSP


Persistent playlist:

    Playlist
        │
        ├── Entry 1 → Track A
        ├── Entry 2 → Track B
        ├── Entry 3 → Track A
        └── Entry 4 → Track C


Playback:

    Playlist
        │
        ▼
    Queue Snapshot
        │
        ├── Queue Entry 1 → Track A
        ├── Queue Entry 2 → Track B
        ├── Queue Entry 3 → Track A
        └── Queue Entry 4 → Track C
        │
        ▼
    Shuffle / Repeat
        │
        ▼
    Playback Engine


The central invariant is:

A PLAYLIST IS PERSISTENT USER INTENT; A QUEUE IS A TEMPORARY PLAYBACK SNAPSHOT.

The second invariant is:

PLAYLIST ORDER MUST NEVER BE CHANGED BY SHUFFLE, LIBRARY SORTING, OR QUEUE OPERATIONS.

The third invariant is:

DELETING A PLAYLIST MUST NEVER DELETE THE UNDERLYING MUSIC FILES.

The fourth invariant is:

MISSING MUSIC MUST NOT DESTROY PLAYLIST STRUCTURE.

The fifth invariant is:

PLAYLIST EDITS MUST NOT SILENTLY MODIFY AN ACTIVE QUEUE.

Final policy:

Store playlists locally.

Use stable IDs.

Reference tracks through the library.

Preserve explicit order.

Allow duplicates unless explicitly disabled.

Represent unavailable tracks safely.

Convert playlists into queue snapshots for playback.

Keep shuffle and repeat in the queue system.

Keep playlist persistence transactional.

Keep playlist failures non-fatal.

Keep playlist operations independent from the audio output path.

And always preserve the distinction:

LIBRARY = WHAT EXISTS

PLAYLIST = WHAT THE USER SAVED

QUEUE = WHAT THE USER IS CURRENTLY PLAYING

PLAYBACK ENGINE = HOW AUDIO IS PLAYED