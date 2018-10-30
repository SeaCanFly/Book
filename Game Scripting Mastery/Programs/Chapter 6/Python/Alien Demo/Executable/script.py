#	Chapter.

#		06-08
	
#	Abstract.

#		Handles most of the logic behind the alien head demo, but does so in Python
#		rather than C.

#	Date Created.

#		9.30.2002

#	Author.

#		Alex Varanese

# ---- Module Imports -------------------------------------------------------------------------

import HostAPI						# Import the host API

# ---- Constnts --------------------------------------------------------------------------------

# Python doesn't support constants, so we'll just use globals that sorta look like them. :)

ALIEN_COUNT 		= 12				# Number of aliens onscreen

MIN_VEL			= 2				# Minimum velocity
MAX_VEL			= 8				# Maximum velocity

ALIEN_WIDTH		= 128 				# Width of the alien sprite
ALIEN_HEIGHT		= 128				# Height of the alien sprite
HALF_ALIEN_WIDTH	= ALIEN_WIDTH / 2 		# Half of the sprite width
HALF_ALIEN_HEIGHT	= ALIEN_HEIGHT / 2		# Half of the sprite height

ALIEN_FRAME_COUNT	= 32				# Number of frames in the animation
ALIEN_MAX_FRAME		= ALIEN_FRAME_COUNT - 1		# Maximum valid frame

ANIM_TIMER_INDEX	= 0				# Animation timer index
MOVE_TIMER_INDEX	= 1				# Movement timer index

# ---- Globals --------------------------------------------------------------------------------

Aliens = []						# Sprites
	
CurrAnimFrame = 0					# Current frame in the alien animation

# ---- Functions ------------------------------------------------------------------------------

# Initializes the demo

def Init ():

	# Import our "constants "

	global ALIEN_COUNT
	global ALIEN_WIDTH
	global ALIEN_HEIGHT
	global MIN_VEL
	global MAX_VEL
	
	# Import the Aliens list
	
	global Aliens
	
	# Loop through each alien of the list and initialize it

	CurrAlienIndex = 0
	while CurrAlienIndex < ALIEN_COUNT:
	
		# Set a random X, Y location
	
		X = HostAPI.GetRandomNumber ( 0, 639 - ALIEN_WIDTH )
		Y = HostAPI.GetRandomNumber ( 0, 479 - ALIEN_HEIGHT )
		
		# Set a random X, Y velocity
		
		XVel = HostAPI.GetRandomNumber ( MIN_VEL, MAX_VEL )
		YVel = HostAPI.GetRandomNumber ( MIN_VEL, MAX_VEL )
		
		# Set a random spin direction
		
		SpinDir = HostAPI.GetRandomNumber ( 0, 2 )
		
		# Add the values to a new list
		
		CurrAlien = [ X, Y, XVel, YVel, SpinDir ]
		
		# Nest the new alien within the alien list
		
		Aliens.append ( CurrAlien )
		
		# Move to the next alien
		
		CurrAlienIndex = CurrAlienIndex + 1

# Creates and blits the next frame of the demo

def HandleFrame ():

	# Import our "constants"

	global ALIEN_COUNT
	global ANIM_TIMER_INDEX
	global MOVE_TIMER_INDEX
	global ALIEN_FRAME_COUNT
	global ALIEN_MAX_FRAME
	global HALF_ALIEN_WIDTH
	global HALF_ALIEN_HEIGHT
	
	# Import the globals
	
	global Aliens
	global CurrAnimFrame
	
	# Blit the background

	HostAPI.BlitBG ()
	
	# Update the current frame of animation
	
	if HostAPI.GetTimerState ( ANIM_TIMER_INDEX ):
		CurrAnimFrame = CurrAnimFrame + 1
		if CurrAnimFrame > ALIEN_MAX_FRAME:
			CurrAnimFrame = 0
			
	# Loop through each alien and draw it
			
	CurrAlienIndex = 0
	while CurrAlienIndex < ALIEN_COUNT:
	
		# Get the X, Y location
	
		X = Aliens [ CurrAlienIndex ][ 0 ]
		Y = Aliens [ CurrAlienIndex ][ 1 ]
		
		# Get the spin direction
		
		SpinDir = Aliens [ CurrAlienIndex ][ 4 ]
		
		# Calculate the final animation frame
		
		if SpinDir:
			FinalAnimFrame = ALIEN_MAX_FRAME - CurrAnimFrame
		else:
			FinalAnimFrame = CurrAnimFrame
		
		# Draw the alien and move to the next
	
		HostAPI.BlitSprite ( FinalAnimFrame, X, Y )
		CurrAlienIndex = CurrAlienIndex + 1
		
	# Blit the completed frame to the screen
		
	HostAPI.BlitFrame ()
	
	# Loop through each alien and move it, checking for collisions
	
	CurrAlienIndex = 0
	while CurrAlienIndex < ALIEN_COUNT:
	
		# Get the X, Y location
	
		X = Aliens [ CurrAlienIndex ][ 0 ]
		Y = Aliens [ CurrAlienIndex ][ 1 ]
		
		# Get the X, Y velocity
		
		XVel = Aliens [ CurrAlienIndex ][ 2 ]
		YVel = Aliens [ CurrAlienIndex ][ 3 ]
		
		# Move the alien along its path
		
		X = X + XVel
		Y = Y + YVel
		
		# Check for collisions
		
		if X < 0 - HALF_ALIEN_WIDTH or X > 640 - HALF_ALIEN_WIDTH:
			XVel = -XVel

		if Y < 0 - HALF_ALIEN_WIDTH or Y > 480 - HALF_ALIEN_HEIGHT:
			YVel = -YVel
			
		# Update the positions
		
		Aliens [ CurrAlienIndex ][ 0 ] = X
		Aliens [ CurrAlienIndex ][ 1 ] = Y
		Aliens [ CurrAlienIndex ][ 2 ] = XVel
		Aliens [ CurrAlienIndex ][ 3 ] = YVel
		
		# Move to the next alien
	
		CurrAlienIndex = CurrAlienIndex + 1