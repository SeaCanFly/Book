--	Chapter.

--		06-02
	
--	Abstract.

--		Simple Lua script.

--	Date Created.

--		6.22.2002

--	Author.

--		Alex Varanese

-----------------------------------------------------------------------------------------------

-- Create a full name string

FirstName = "Alex";
LastName = "Varanese";
FullName = "Name: " .. FirstName .. " " .. LastName;

-- Now put the floating point value of pi into a string

Pi = 3.14159;
PiString = "Pi: " .. Pi;			-- Numeric values can be automatically coerced to strings

-- Test some logic

X = 0;						-- Try setting this to nil instead of zero
if X then
	Logic = "X is true.";			-- Remember, only nil is considered false in Lua
else
	Logic = "X is false.";
end

-- Now call our exported C function for printing the strings

PrintStringList ( "Random Strings:", "" );	-- The extra empty string is just to create a blank line
PrintStringList ( FullName, PiString, Logic );