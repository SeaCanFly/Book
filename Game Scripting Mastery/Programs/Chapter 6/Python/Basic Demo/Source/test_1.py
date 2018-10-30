#	Chapter.

#		06-06
	
#	Abstract.

#		A more complex Python script.

#	Date Created.

#		9.30.2002

#	Author.

#		Alex Varanese

# ---- Module Imports -------------------------------------------------------------------------

import HostAPI						# Import the host API

# ---- Functions ------------------------------------------------------------------------------

# A function that calls back the host application's RepeatString () function

def PrintStuff ():
	
	# Print some stuff to show we're alive
	
	print "\tPrintStuff was called from the host."
	
	# Call the host API function RepeatString () and print out it's return value

	RepCount = HostAPI.RepeatString ( "String repetition", 4 )
	print "\tString was printed", RepCount, "times."
		
# A function for calculating the maximum of two integers

def GetMax ( X, Y ):
	
	# Print out the command name and parameters
	
	print "\tGetMax was called from the host with", X, "and", Y
		
	# Perform the maximum check
		
	if X > Y:
		return X
	else:
		return Y