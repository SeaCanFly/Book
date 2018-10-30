--	Chapter.

--		06-03
	
--	Abstract.

--		Handles most of the logic behind the alien head demo, but does so in Lua rather
--		than C.

--	Date Created.

--		6.23.2002

--	Author.

--		Alex Varanese

---- Constants --------------------------------------------------------------------------------

	-- Lua doesn't support constants, so we'll just use globals that sorta look like
	-- them. :)

	ALIEN_COUNT 		= 12;			-- Number of aliens onscreen

	MIN_VEL			= 2;			-- Minimum velocity
	MAX_VEL			= 8;			-- Maximum velocity

	ALIEN_WIDTH		= 128;			-- Width of the alien sprite
	ALIEN_HEIGHT		= 128;			-- Height of the alien sprite
	HALF_ALIEN_WIDTH	= ALIEN_WIDTH / 2;	-- Half of the sprite width
	HALF_ALIEN_HEIGHT	= ALIEN_HEIGHT / 2;	-- Half of the sprite height

	ALIEN_FRAME_COUNT	= 32;			-- Number of frames in the animation
	ALIEN_MAX_FRAME		= ALIEN_FRAME_COUNT - 1;	-- Maximum valid frame
	
	ANIM_TIMER_INDEX	= 0;			-- Animation timer index
	MOVE_TIMER_INDEX	= 1;			-- Movement timer index
	
---- Global Variables -------------------------------------------------------------------------

	Aliens = {};					-- Sprites
	
	CurrAnimFrame = 0;				-- Current frame in the alien animation
	
---- Functions --------------------------------------------------------------------------------

	-- Initializes the demo

	function Init ()
	
		-- Initialize the alien sprites
		
		-- Loop through each alien in the table and initialize it
		
		for CurrAlienIndex = 1, ALIEN_COUNT do

			-- Create a new table to hold all of the alien's fields

			local CurrAlien = {};
		
			-- Set the X, Y location
		
			CurrAlien.X = GetRandomNumber ( 0, 639 - ALIEN_WIDTH );
			CurrAlien.Y = GetRandomNumber ( 0, 479 - ALIEN_HEIGHT );
			
			-- Set the X, Y velocity
			
			CurrAlien.XVel = GetRandomNumber ( MIN_VEL, MAX_VEL );
			CurrAlien.YVel = GetRandomNumber ( MIN_VEL, MAX_VEL );
			
			-- Set the spin direction
			
			CurrAlien.SpinDir = GetRandomNumber ( 0, 2 );
			
			-- Copy the reference to the new alien into the table
			
			Aliens [ CurrAlienIndex ] = CurrAlien;
			
		end
	
	end
	
	-- Creates and blits the next frame of the demo

	function HandleFrame ()
	
		-- Blit the background image
	
		BlitBG ();
		
		-- Blit each sprite

		for CurrAlienIndex = 1, ALIEN_COUNT do

			-- Get the X, Y location

			local X = Aliens [ CurrAlienIndex ].X;
			local Y = Aliens [ CurrAlienIndex ].Y;

			-- Get the spin direction and determine the final frame for this sprite
			-- based on it.

			local SpinDir = Aliens [ CurrAlienIndex ].SpinDir;

			if SpinDir == 1 then
				FinalAnimFrame = ALIEN_MAX_FRAME - CurrAnimFrame;
			else
				FinalAnimFrame = CurrAnimFrame;
			end

			-- Blit the sprite

			BlitSprite ( FinalAnimFrame, X, Y );

		end
		
		-- Blit the completed frame to the screen
		
		BlitFrame ();

		-- Increment the current frame in the animation
		
		if GetTimerState ( ANIM_TIMER_INDEX ) == 1 then
			CurrAnimFrame = CurrAnimFrame + 1;
			if CurrAnimFrame >= ALIEN_FRAME_COUNT then
				CurrAnimFrame = 0;
			end
		end

		-- Move the sprites along their paths
		
		if GetTimerState ( MOVE_TIMER_INDEX ) == 1 then
			for CurrAlienIndex = 1, ALIEN_COUNT do

				-- Get the X, Y location

				local X = Aliens [ CurrAlienIndex ].X;
				local Y = Aliens [ CurrAlienIndex ].Y;

				-- Get the X, Y velocities

				local XVel = Aliens [ CurrAlienIndex ].XVel;
				local YVel = Aliens [ CurrAlienIndex ].YVel;

				-- Increment the paths of the aliens

				X = X + XVel;
				Y = Y + YVel;
				Aliens [ CurrAlienIndex ].X = X;
				Aliens [ CurrAlienIndex ].Y = Y;
				
				-- Check for wall collisions
				
				if X > 640 - HALF_ALIEN_WIDTH or X < -HALF_ALIEN_WIDTH then
					XVel = -XVel;
				end
				if Y > 480 - HALF_ALIEN_HEIGHT or Y < -HALF_ALIEN_HEIGHT then
					YVel = -YVel;
				end
				Aliens [ CurrAlienIndex ].XVel = XVel;
				Aliens [ CurrAlienIndex ].YVel = YVel;

			end
		end
		
	end