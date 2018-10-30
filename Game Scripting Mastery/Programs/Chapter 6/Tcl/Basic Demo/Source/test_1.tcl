#	Chapter.

#		06-06
	
#	Abstract.

#		A more complex Tcl script.

#	Date Created.

#		7.4.2002

#	Author.

#		Alex Varanese

# ---- Globals --------------------------------------------------------------------------------

	set GlobalInt 256
	set GlobalString "Look maw..."

# ---- Functions ------------------------------------------------------------------------------

	# Create a new command that calls back the host application's RepeatString command

	proc PrintStuff {} {
	
		# Print some stuff to show we're alive
	
		puts "\tPrintStuff was called from the host."
		
		# Call the host API command RepeatString and print out it's return value
		
		set RepCount [ RepeatString "String repetition." 4 ]
		puts "\tString was printed $RepCount times."
	}
		
	# Create a new command for calculating the maximum of two integers

	proc GetMax { X Y } {
	
		# Print out the command name and parameters
	
		puts "\tGetMax was called from the host with $X, $Y."
		
		# Perform the maximum check
		
		if { $X > $Y } {
			return $X
		} else {
			return $Y
		}
	}

	# Notice that there isn't any code outside of functions; this lets us load the script without
	# anything happening, so we can then call it's functions whenever we need them.
	# However, if our library of Tcl functions needed any sort of initialization, this would
	# be a good place to do it since it'd automatically be run once at the time the script is
	# loaded.