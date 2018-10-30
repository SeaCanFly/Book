// NPC 0 Script
// Written by Alex Varanese

// Move the NPC up a bit

SetCharLoc 296 40

// Move the NPC back and forth horizontally

SetCharDir "Right"
MoveChar 96, 0
SetCharDir "Left"
MoveChar -192, 0
SetCharDir "Right"
MoveChar 96, 0