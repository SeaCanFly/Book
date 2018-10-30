// RPG NPC Script
// A Command-Based Language Demo
// Written by Alex Varanese

// ---- Backing up

ShowTextBox "WELCOME TO THIS DEMO."
Pause 2400
ShowTextBox "THIS DEMO WILL CONTROL THE ONSCREEN NPC."
Pause 2400
ShowTextBox "LET'S START BY BACKING UP SLOWLY..."
Pause 2400
HideTextBox
Pause 800
MoveChar 0 -48
Pause 800

// ---- Walking in a square pattern

ShowTextBox "THAT WAS SIMPLE ENOUGH."
Pause 2400
ShowTextBox "NOW LET'S WALK IN A SQUARE PATTERN."
Pause 2400
HideTextBox
Pause 800
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

// Random movement with text box

ShowTextBox "WE CAN EVEN MOVE AROUND WITH THE TEXT BOX ACTIVE!"
Pause 2400
ShowTextBox "WHEEEEEEEEEEE!!!"
Pause 800
SetCharDir "Down"
MoveChar 12, 38
SetCharDir "Left"
MoveChar -40, 10
SetCharDir "Up"
MoveChar 7, 0
SetCharDir "Right"
MoveChar -28, -9
MoveChar 12, -8
SetCharDir "Down"
MoveChar 4, 37

MoveChar 12, 4

// Transition back to the start of the demo

ShowTextBox "THIS DEMO WILL RESTART MOMENTARILY..."
Pause 2400
SetCharLoc 296 208
SetCharDir "Down"