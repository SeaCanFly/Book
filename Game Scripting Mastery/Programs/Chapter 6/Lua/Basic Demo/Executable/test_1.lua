--	Chapter.

--		06-02
	
--	Abstract.

--		A Lua script that provides a few random functions.

--	Date Created.

--		6.22.2002

--	Author.

--		Alex Varanese

---- Globals ----------------------------------------------------------------------------------

	GlobalInt = 256;
	GlobalFloat = 2.71828;
	GlobalString = "I'm an obtuse man...";

---- Functions --------------------------------------------------------------------------------

	-- Manually computes exponents in the form of X ^ Y

	function Exponent ( X, Y )

		-- First, let's just print out the parameters

		PrintStringList ( "Calculating " .. X .. " to the power of " .. Y );

		-- Now manually compute the result

		Exponent = 1;
		if Y < 0 then
			Exponent = -1;		-- Just return -1 for all negative exponents
		elseif Y ~= 0 then
			for Power = 1, Y do
				Exponent = Exponent * X;
			end
		end

		-- Return the final value to C	

		return Exponent;

	end

	-- "Multiplies" a string; in other words, repeats a string a number of times

	function MultiplyString ( String, Factor )

		-- As with the above function, print out the parameters

		PrintStringList ( "Multiplying string \"" .. String .. "\" by " .. Factor );

		-- Multiply the string

		NewString = "";
		for X = 1, Factor do
			NewString = NewString .. String;
		end

		-- Return the multiplied string to C

		return NewString;

	end

	-- Notice that there isn't any code outside of functions; this lets us load the script without
	-- anything happening, so we can then call it's functions whenever we need them.
	-- However, if our library of Lua functions needed any sort of initialization, this would
	-- be a good place to do it since it'd automatically be run once at the time the script is
	-- loaded.