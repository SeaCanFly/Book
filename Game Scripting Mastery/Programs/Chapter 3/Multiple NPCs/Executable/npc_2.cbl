// NPC 2 Script
// Written by Alex Varanese

// Move the NPC in a square pattern

SetCharLoc 208 180
SetCharDir "Right"
MoveChar 40 0
MoveChar 8 8
SetCharDir "Down"
MoveChar 0 80
MoveChar -8 8
SetCharDir "Left"
MoveChar -80 0
MoveChar -8 -8
SetCharDir "Up"
MoveChar 0 -80
MoveChar 8 -8
SetCharDir "Right"
MoveChar 40 0
Pause 800