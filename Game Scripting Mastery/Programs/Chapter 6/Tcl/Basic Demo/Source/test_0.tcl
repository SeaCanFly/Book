#	Chapter.

#		06-06
	
#	Abstract.

#		Simple Tcl script.

#	Date Created.

#		7.4.2002

#	Author.

#		Alex Varanese

# ---------------------------------------------------------------------------------------------

# Create some variables of varying data types

set IntVar 256
set FloatVar 3.14159
set StringVar "Tcl String"

# Test out some conditional logic

set X 0
set Logic ""
if { $X } {
	set Logic "X is true."
} else {
	set Logic "X is false."
}

# Print the variables out to make sure everything is working

puts "Random Stuff:"
puts "\tInteger: $IntVar"
puts "\t  Float: $FloatVar"
puts "\t String: \"$StringVar\""
puts "\t  Logic: $Logic"