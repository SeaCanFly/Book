#	Chapter.

#		06-07
	
#	Abstract.

#		Handles most of the logic behind the alien head demo, but does so in Tcl rather
#		than C.

#	Date Created.

#		7.4.2002

#	Author.

#		Alex Varanese

# ---- Constnts --------------------------------------------------------------------------------

	# Tcl doesn't support constants, so we'll just use globals that sorta look like
	# them. :)

	set ALIEN_COUNT 	12;			# Number of aliens onscreen

	set MIN_VEL		2;			# Minimum velocity
	set MAX_VEL		8;			# Maximum velocity

	set ALIEN_WIDTH		128;			# Width of the alien sprite
	set ALIEN_HEIGHT	128;			# Height of the alien sprite
	set HALF_ALIEN_WIDTH	[ expr $ALIEN_WIDTH / 2 ]; # Half of the sprite width
	set HALF_ALIEN_HEIGHT	[ expr $ALIEN_HEIGHT / 2 ]; # Half of the sprite height

	set ALIEN_FRAME_COUNT	32;			# Number of frames in the animation
	set ALIEN_MAX_FRAME	[ expr $ALIEN_FRAME_COUNT - 1 ]; # Maximum valid frame
	
	set ANIM_TIMER_INDEX	0;			# Animation timer index
	set MOVE_TIMER_INDEX	1;			# Movement timer index

# ---- Globals --------------------------------------------------------------------------------

	set Aliens() 0;					# Sprites
	
	set CurrAnimFrame 0;				# Current frame in the alien animation

# ---- Functions ------------------------------------------------------------------------------

	# Initializes the demo

	proc Init {} {

		# Import the constants we'll need
		
		global ALIEN_COUNT;
		global ALIEN_WIDTH;
		global ALIEN_HEIGHT;
		global MIN_VEL;
		global MAX_VEL;
	
		# Import the alien array
	
		global Aliens;
		
		# Initialize the alien sprites
		
		# Loop through each alien in the table and initialize it
		
		for { set CurrAlienIndex 0; } { $CurrAlienIndex < $ALIEN_COUNT } { incr CurrAlienIndex; } {
	
			# Set the X, Y location
		
			set Aliens($CurrAlienIndex,X) [ GetRandomNumber 0 [ expr 639 - $ALIEN_WIDTH ] ];
			set Aliens($CurrAlienIndex,Y) [ GetRandomNumber 0 [ expr 479 - $ALIEN_HEIGHT ] ];
			
			# Set the X, Y velocity
			
			set Aliens($CurrAlienIndex,XVel) [ GetRandomNumber $MIN_VEL $MAX_VEL ];
			set Aliens($CurrAlienIndex,YVel) [ GetRandomNumber $MIN_VEL $MAX_VEL ];
			
			# Set the spin direction
			
			set Aliens($CurrAlienIndex,SpinDir) [ GetRandomNumber 0 2 ];
		}
	}
	
	# Creates and blits the next frame of the demo
	
	proc HandleFrame {} {

		# Import the constants we'll need
		
		global ALIEN_COUNT;
		global ANIM_TIMER_INDEX;
		global MOVE_TIMER_INDEX;
		global ALIEN_FRAME_COUNT;
		global ALIEN_MAX_FRAME;
		global HALF_ALIEN_WIDTH;
		global HALF_ALIEN_HEIGHT

		# Import our globals
	
		global Aliens;
		global CurrAnimFrame;	
				
		# Blit the background image
	
		BlitBG;

		# Increment the current frame in the animation
			
		if { [ GetTimerState $ANIM_TIMER_INDEX ] == 1 } {
			incr CurrAnimFrame;
			if { $CurrAnimFrame >= $ALIEN_FRAME_COUNT } {
				set CurrAnimFrame 0;
			}
		}
		
		# Blit each sprite

		for { set CurrAlienIndex 0; } { $CurrAlienIndex < $ALIEN_COUNT } { incr CurrAlienIndex; } {

			# Get the X, Y location

			set X $Aliens($CurrAlienIndex,X);
			set Y $Aliens($CurrAlienIndex,Y);

			# Get the spin direction and determine the final frame for this sprite
			# based on it.

			set SpinDir $Aliens($CurrAlienIndex,SpinDir);

			if { $SpinDir == 1 } {
				set FinalAnimFrame [ expr $ALIEN_MAX_FRAME - $CurrAnimFrame ];
			} else {
				set FinalAnimFrame $CurrAnimFrame;
			}		

			# Blit the sprite

			BlitSprite $FinalAnimFrame $X $Y;
		}
		
		# Blit the completed frame to the screen

		BlitFrame;
		
		# Move the sprites along their paths
		
		if { [ GetTimerState $MOVE_TIMER_INDEX ] == 1 } {
		
			for { set CurrAlienIndex 0; } { $CurrAlienIndex < $ALIEN_COUNT } { incr CurrAlienIndex; } {

				# Get the X, Y location

				set X $Aliens($CurrAlienIndex,X);
				set Y $Aliens($CurrAlienIndex,Y);

				# Get the X, Y velocities

				set XVel $Aliens($CurrAlienIndex,XVel);
				set YVel $Aliens($CurrAlienIndex,YVel);

				# Increment the paths of the aliens

				incr X $XVel
				incr Y $YVel
				set Aliens($CurrAlienIndex,X) $X
				set Aliens($CurrAlienIndex,Y) $Y		
				
				# Check for wall collisions
				
				if { $X > 640 - $HALF_ALIEN_WIDTH || $X < -$HALF_ALIEN_WIDTH } {
					set XVel [ expr -$XVel ];
				}
				if { $Y > 480 - $HALF_ALIEN_HEIGHT || $Y < -$HALF_ALIEN_HEIGHT } {
					set YVel [ expr -$YVel ];
				}
				set Aliens($CurrAlienIndex,XVel) $XVel
				set Aliens($CurrAlienIndex,YVel) $YVel
			}
		}
	}