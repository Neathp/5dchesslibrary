# 5D-Chess-General-Library
For simulating or manipulating 5D chess games/boards.

## Terminology:
Terms are subject to change, say so if you find them confusing or want to change them to something else.
**5DGame**: *Class* - A game or subgame of 5D chess. Has methods for verification and movement, but does not inherently guarantee legality.

**Metaboard**: *1D Array* - An array in 5DGame, with each int index referencing a unique timeline, turn, rank, and file position.

**Board**: *int* - A single chess board with pieces in specific positions. Addressed internally as an int.

**Headboard**: *General Term* - A board that is the farthest right on its own timeline (it can be played.) 

**Piece**: *Const int/char* - Can be used as a general term, but is technically an int associated with a piece type, possibly with different values for color/moved variants.

**(Local) Position**: *General Term* - Refers to the position of a piece from its board's perspective.

**(Global) Location**: *General Term* - Refers to the position of something from the perspective of the metaboard.

**Turn**: *General Term* - Two successive boards/moves, the first being white and the second being black. Can be numbered and abbreviated as T1, T2, etc.

**Half-Turn**: *General Term* - A single side's board or move. Can be numbered and abbreviated as T1-W, T1-B, etc.

**Fullboard**: *General Term* - The Metaboard's internal 1D array representation of a game, including timelines, turns, and individual boards.

## Technical Details

### Metaboard and Piece Representation
The Metaboard is a 1D array, with indices referencing a single 4D location. Indices work on a base system, with:
* Files addressed from 0 to 8, 
* Ranks addressed from 9 to 90 in multiples of 9,
* Turns addressed from 99 to (99 * maxTurns) in multiples of 99, 
* And timelines addressed from 99( maxTurns + 1 ) to maxTimelines(99(maxTurns + 1)) in multiples of 99(maxTurns + 1). 


## Paths of Development:

### Metaboard Representation

There are several ways the metaboard could be treated:

The first variant are what I call "*superarray*" methods, since they single-handedly contain the entire game in a single array without the concept of a board. Some versions might be:
1. A large 4D array, composed of timeline, turn, rank, file.
2. A 3D array, composed of timeline, turn, position.
3. An array which interprets one number as timeline, turn, and position.

Implementations can allow easy, fast, and intuitive access of specific locations, as well as fast verification of existing positions. However, the superarray may use tons of memory and reallocations may be very expensive, even if done infrequently. A 128 turn, 128 timeline metaboard with 8x8 boards and 1B squares would use 64\*128\*256 = 2087152 bytes, or 2MB. A 128 turn, 128 timeline metaboard with 10x10 boards and 4B squares would use 13631488B, or 13MB. Without further features, a full board search is necessary to find pieces on a board. Determining the existence of timelines, turns, and pieces would have to be done with a full search across all spaces without additional features.

A second variant would be "container array" methods, which would hold pointers to other arrays. For example, there could be one array containing pointers to every timeline array, which could contain pointers to turn arrays, and potentially so on. This would save memory and prevent large areas from having to be contigous, as well as allowing easier implementation of a board class. However, it might make navigation more confusing and would likely make checking the existence of specific timelines/turns slower when compared to the superarray method. Pointers also use up a decent amount of memory, so a nearly full superarray would be more memory-efficient than an equivalent collection of container arrays. Exact numbers and thresholds would depend upon implementation.



Other variants, obviously, could use different elements of each approach in different ways.

### Headboards
Could store an array with pieces and their locations to allow quick access for headboards specifically. In the case of something like a superarray, both arrays would have to be updated and kept consistent.

## Boards
Implementation of boards as a class would allow extra information to be stored in them, such as check status, king locations, piece arrays, headboard status, etc. This would obviously use extra memory, and might potentially make accessing boards slower (although not by much,) depending on how they are implemented. Without them, storing this data might be more difficult, but might open up other avenues of managing the metaboard.

### Array Padding
It may be cheaper to "pad" arrays, such as boards, with permanently null elements to make verifying valid moves easier. Since chess pieces can only move so many spaces, an 8x8 block with a buffer of 2 above and below, as well as 1 left and right, should be enough to handle most cases. Instead of doing two comparisons for whether both indices are within 8x8, it would be possible to simply increment from a given piece in steps of 1 or 2 and determine that way if the moves are valid. However, this would use ~50% more memory per board.

## Common Code Practice:
* K&R indention style (brackets generally on another line).
* Use 4 spaces for indents.
* Try to limit lines to at most 80 chars.
* Try to use bools for returning success/failures. If that's not impossible, try using NULL or -1 (if the result should be positive.) If using an int as a "truth" value, C common practice is 0 = success and not 0 = failure.
* Javadocs style comments for functions are not necessary. Use comments at the head of functions to clarify return behavior or parameters.
* Avoid commenting potential function descriptions into the code unless they've been discussed or others may miss it. As of right now, though, do what you want.

## Possible Additions:
* Timeline advantage counter for the metaboard
