// NPC 1 Script
// Written by Alex Varanese

// Move the NPC up a bit

SetCharLoc 296 260

// Move the NPC back and forth horizontally, pausing at each turn

SetCharDir "Left"
MoveChar -96, 0
Pause 800
SetCharDir "Right"
MoveChar 192, 0
Pause 800
SetCharDir "Left"
MoveChar -96, 0