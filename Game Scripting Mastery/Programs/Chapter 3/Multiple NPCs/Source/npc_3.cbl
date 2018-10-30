// NPC 3 Script
// Written by Alex Varanese

// Move the NPC in a larger square pattern, pausing slightly at each corner

SetCharLoc 384 100
SetCharDir "Left"
MoveChar -80 0
Pause 180

MoveChar -8 8
SetCharDir "Down"
MoveChar 0 80
Pause 180

MoveChar 8 8
SetCharDir "Right"
MoveChar 160 0
Pause 180

MoveChar 8 -8
SetCharDir "Up"
MoveChar 0 -80
Pause 180

MoveChar -8 -8
SetCharDir "Left"
MoveChar -80 0