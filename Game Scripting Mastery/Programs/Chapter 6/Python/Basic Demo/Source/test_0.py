#	Chapter.

#		06-07
	
#	Abstract.

#		Simple Python script.

#	Date Created.

#		9.30.2002

#	Author.

#		Alex Varanese

# ---------------------------------------------------------------------------------------------

# Create some variables of varying data types

IntVar = 256
FloatVar = 3.14159
StringVar = "Python String"

# Test out some conditional logic

X = 0
Logic = ""
if X:
	Logic = "X is true"
else:
	Logic = "X is false"

# Print the variables out to make sure everything is working

print "Random Stuff:"
print "\tInteger:", IntVar
print "\t  Float:", FloatVar
print "\t String: " + '"' + StringVar + '"'
print "\t  Logic: " + Logic