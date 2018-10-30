// NPC 4 Script
// Written by Alex Varanese

// Move the NPC over a bit

SetCharLoc 320 216

// Move the NPC back and forth vertically

SetCharDir "Down"
MoveChar 0, 64
SetCharDir "Up"
MoveChar 0, -128
SetCharDir "Down"
MoveChar 0, 64
Pause 480