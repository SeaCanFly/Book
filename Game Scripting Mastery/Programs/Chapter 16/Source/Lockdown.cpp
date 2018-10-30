/*

	Project.

		Lockdown
	
	Abstract.

		Perhaps the most influential semi-complete game/demo of our time. An epic journey of
        tribulation and redemption, and an enduring testiment to the power of the human spirit.
        Plus it's got robots and lasers!

    Date Created.

	    9.21.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include "wrappuh.h"
    #include "xvm.h"

// ---- Constants -----------------------------------------------------------------------------

    // ---- Directions ------------------------------------------------------------------------

        #define NORTH                           0
        #define NORTH_EAST                      1
        #define EAST                            2
        #define SOUTH_EAST                      3
        #define SOUTH                           4
        #define SOUTH_WEST                      5
        #define WEST                            6
        #define NORTH_WEST                      7

        #define STRAIGHT_NORTH                  0
        #define STRAIGHT_SOUTH                  1
        #define STRAIGHT_EAST                   2
        #define STRAIGHT_WEST                   3

        #define DIR_COUNT                       8       // Direction count (including
        #define STRAIGHT_DIR_COUNT              4       // Straight direction count
                                                        // diagonals)

    // ---- Game Engine -----------------------------------------------------------------------

        #define FPS_LOCK                        60      // Maximum allowed FPS
        #define FPS_LOCK_FRAME_DUR              1000 / FPS_LOCK // Maximum duration of a frame
                                                                // in milliseconds

    // ---- Game States -----------------------------------------------------------------------

        #define GAME_STATE_TITLE                0       // Title screen
        #define GAME_STATE_HOW_TO_PLAY          1       // How to Play screen
        #define GAME_STATE_LOADING              2       // Loading screen
        #define GAME_STATE_PLAY                 3       // Gameplay
        #define GAME_STATE_CONGRATS             4       // The player won
        #define GAME_STATE_GAME_OVER            5       // The player lost
        #define GAME_STATE_END                  6       // End the game

    // ---- Title Screen Menu -----------------------------------------------------------------

        #define TITLE_MENU_NEW_GAME             0       // New game
        #define TITLE_MENU_EXIT                 1       // Exit

    // ---- Graphics --------------------------------------------------------------------------

        #define LIGHTS_OFF                      0       // Lights off flag
        #define LIGHTS_ON                       1       // Lights on flag
    
    // ---- Gameplay --------------------------------------------------------------------------

        #define MAX_ENERGY                      8       // Maximum energy

        #define CONGRATS_SCREEN_DELAY           2000    // Delay between the activation of the
                                                        // key panels and the congrats screen

    // ---- Doors -----------------------------------------------------------------------------

        #define DOOR_ANIM_FRAME_COUNT           3       // Number of door animation frames

    // ---- Keys ------------------------------------------------------------------------------

        #define RED                             0       // Red
        #define BLUE                            1       // Blue
        #define GREEN                           2       // Green
        #define YELLOW                          3       // Yellow

        #define KEY_SPRITE_WIDTH                128     // Key sprite width
        #define KEY_SPRITE_HEIGHT               96      // Key sprite height

        #define KEY_COUNT                       4       // Number of keys
        #define KEY_ANIM_FRAME_COUNT            8       // Number of key animtaion frames
        #define KEY_ANIM_SPEED                  80      // Key animation speed

        #define KEY_X                           256     // Key X location
        #define KEY_Y                           157 - KEY_FLOAT_SCALE    // Key Y location

        #define KEY_FLOAT_FRAME_STEP            0.5F    // Radian step for float animation
        #define KEY_FLOAT_SCALE                 20      // Scale of the key float animation

        #define KEY_PANEL_WIDTH                 119     // Key panel width
        #define KEY_PANEL_HEIGHT                118     // Key panel height

        #define KEY_PANEL_RED_X                 374     // Red key panel X location
        #define KEY_PANEL_RED_Y                 99      // Red key panel X location
        #define KEY_PANEL_BLUE_X                148     // Blue key panel X location
        #define KEY_PANEL_BLUE_Y                99      // Blue key panel X location
        #define KEY_PANEL_GREEN_X               374     // Green key panel X location
        #define KEY_PANEL_GREEN_Y               268     // Green key panel X location
        #define KEY_PANEL_YELLOW_X              148     // Yellow key panel X location
        #define KEY_PANEL_YELLOW_Y              268     // Yellow key panel X location

        #define KEY_ICON_WIDTH                  32      // Key icon width
        #define KEY_ICON_HEIGHT                 32      // Key icon height

    // ---- Droids ----------------------------------------------------------------------------

        #define DROID_TYPE_COUNT                4       // Number of droid types

        #define DROID_TYPE_PLAYER               0       // Player droid
        #define DROID_TYPE_ENEMY                1       // Player enemy

        #define DROID_TYPE_WHITE                0       // The white droid (player)
        #define DROID_TYPE_BLUE                 1       // The blue droid
        #define DROID_TYPE_GREY                 2       // The grey droid
        #define DROID_TYPE_RED                  3       // The red droid

        #define PLAYER_DROID_SPEED              3       // The player droid's speed

        #define ENEMY_DROID_COUNT               8       // Droids population size per room

        #define DROID_DAMAGE_RATTLE_DUR         80      // Damage rattle duration

        #define DROID_WIDTH                     90      // Average droid frame width
        #define DROID_HEIGHT                    90      // Average droid frame height

        #define DROID_COLLIDE_DAMAGE            1       // Damage incurred by colliding with
                                                        // a droid
        #define DROID_COLLIDE_DAMAGE_CHANCE     40      // Chances that a droid collision will
                                                        // cause damage

        #define PLAYER_ENERGY_RECHARGE          0.01F   // The player's energy recharge rate

    // ---- Fortress --------------------------------------------------------------------------

        #define FORTRESS_WIDTH                  5       // Fortress width (in rooms)
        #define FORTRESS_HEIGHT                 5       // Fortress height

        #define ROOM_TYPE_NULL                  0       // Null room
        #define ROOM_TYPE_NORMAL                1       // Normal room
        #define ROOM_TYPE_GUARD                 2       // Normal room with stronger droids
        #define ROOM_TYPE_PEDESTAL              3       // Pedestal room
        #define ROOM_TYPE_KEY                   4       // Key room

        #define DOOR_STATE_CLOSED               0       // A closed door
        #define DOOR_STATE_OPENING              1       // An opening door
        #define DOOR_STATE_OPEN                 2       // An open door
        #define DOOR_ANIM_SPEED                 50      // The speed at which the door animates

        #define WALL_WIDTH                      52      // The width of the surrounding wall

    // ---- Interface -------------------------------------------------------------------------

        #define ENERGY_BAR_WIDTH                10      // Energy bar width
        #define ENERGY_BAR_HEIGHT               32      // Energy bar height

        #define MAP_CURSOR_BLINK_RATE           480     // Map cursor blink rate

    // ---- Lasers ----------------------------------------------------------------------------

        #define MAX_LASER_COUNT                 128     // Maximum number of onscreen lasers

        #define LASER_TYPE_PLAYER               0       // Player laser
        #define LASER_TYPE_ENEMY                1       // Enemey laser

        #define LASER_ANIM_FRAME_COUNT          4       // Number of laser animation frames
        #define LASER_ANIM_SPEED                40      // Laser animation speed

        #define LASER_SPEED                     8       // Laser speed
        #define LASER_DAMAGE                    2       // Laser damage

        #define LASER_RECHARGE_DUR              80      // Laser recharge duration

        #define LASER_KICK                      4       // Kick produced by firing the laser

    // ---- Explosions ------------------------------------------------------------------------

        #define MAX_EXPLOSION_COUNT             64      // Maximum number of simultaneous
                                                        // explosions
        #define EXPLOSION_ANIM_FRAME_COUNT      7       // Number of explosion frames
        #define EXPLOSION_ANIM_SPEED            50      // Explosion animation speed

        #define EXPLOSION_ANIM_WIDTH            140     // Explosion animation width
        #define EXPLOSION_ANIM_HEIGHT           119     // Explosion animation height

    // ---- Scripting -------------------------------------------------------------------------

        #define SCRIPT_TIMESLICE_DUR            20      // Timeslice duration of the scripts

// ---- Data Structures -----------------------------------------------------------------------

    typedef struct _Droid                               // A droid
    {
        int iIsActive;                                  // Is the droid active?
        int iX,                                         // X, Y location on screen
            iY;
        int iDir;                                       // Direction facing
        int iType;                                      // Type
        float fEnergy;                                  // Energy
        unsigned int iLastFireTime;                     // When the laser was last fired
        int iKickDir;                                   // Direction of the kick caused by the
                                                        // laser
        unsigned int iLastDamageTime;                   // When the last damage was incurred
    }
        Droid;

    typedef struct _Player                              // The player
    {
        int iRoomX,                                     // Current room within the fortress
            iRoomY;

        int iKeys [ 4 ];                                // Collected keys
        int iActiveKeyPanels [ 4 ];                     // Activated key panels

        Droid Droid;                                    // The player droid
    }
        Player;

    typedef struct _Door                                // A door
    {
        int iState;                                     // Current state; open, opening, or
                                                        // closed
        int iCurrFrame;                                 // Current animation frame
        unsigned int iLastFrameUpdateTime;              // Time of the last frame update
        W_Rect Rect;                                    // Bounding rectangle
    }
        Door;

    typedef struct _Room                                // A room
    {
        int iLights;                                    // Status of the lights (on or off)

        Door Doors [ STRAIGHT_DIR_COUNT ];              // Doors

        W_Rect NorthDoorTrigger;                        // North door trigger region
        W_Rect SouthDoorTrigger;                        // South door trigger region
        W_Rect EastDoorTrigger;                         // East door trigger region
        W_Rect WestDoorTrigger;                         // West door trigger region
    }
        Room;

    typedef struct _Laser                               // A Laser
    {
        int iIsActive;                                  // Is the laser active?
        int iX,                                         // X, Y location
            iY;
        int iDir;                                       // Direction
        int iType;                                      // Laser type
        int iSpeed;                                     // Speed
        int iCurrFrame;                                 // Current animation frame
        unsigned int iLastFrameUpdateTime;              // Time of the last frame update
    }
        Laser;

    typedef struct _Explosion                           // An Explosion
    {
        int iIsActive;                                  // Is the explosion exploding?
        int iX,                                         // X, Y location
            iY;
        int iCurrFrame;                                 // Current animation frame
        unsigned int iLastFrameUpdateTime;              // Time of the last frame update
    }
        Explosion;

    typedef struct _Key                                 // Key
    {
        int iIsActive;                                  // Is the key active?
        int iX,                                         // X, Y location
            iY;
        int iColor;                                     // Color
        int iCurrFrame;                                 // Current animation frame
        unsigned int iLastFrameUpdateTime;              // Time of the last frame update
        float iCurrFloatFrame;                          // Floating animation frame (in
                                                        // radians)
    }
        Key;

// ---- Global Variables ----------------------------------------------------------------------

    // ---- Game Engine -----------------------------------------------------------------------

        int g_iCurrGameState;                           // The current game state

    // ---- Graphics --------------------------------------------------------------------------

        // ---- Title screen ------------------------------------------------------------------

            W_Image g_TitleScreen;                      // The title screen
            W_Image g_TitleExitHilite;                  // The exit option highlighted

        // ---- How to Play screen ------------------------------------------------------------

            W_Image g_HowToPlayScreen;                  // The How to Play screen

        // ---- Loading screen ----------------------------------------------------------------

            W_Image g_LoadingScreen;                    // The loading screen

        // ---- Game --------------------------------------------------------------------------

            // ---- Rooms

            W_Image g_NormalRoomOff;                    // The normal room, lights off
            W_Image g_NormalRoomOn;                     // The normal room, lights on
            W_Image g_PedestalRoomOff;                  // The pedestal room, lights off
            W_Image g_PedestalRoomOn;                   // The pedestal room, lights on
            W_Image g_KeyRoom;                          // The key room

            // ---- Doors

            W_Image g_DoorAnims [ STRAIGHT_DIR_COUNT ][ 2 ][ DOOR_ANIM_FRAME_COUNT ];   // Door animations

            // ---- Interface

            W_Image g_EnergyFrame;                      // Energy meter frame
            W_Image g_KeysFrame;                        // Collected keys frame
            W_Image g_EnergyBars [ 3 ][ 2 ];            // Energy bars

            // ---- Map

            W_Image g_MapScreen;                        // The map screen
            W_Image g_MapCursor;                        // The map cursor

            // ---- Droids

            W_Image g_Droids [ DROID_TYPE_COUNT ][ DIR_COUNT ];    // The droids

            // ---- Lasers

            W_Image g_PlayerLaserAnims [ STRAIGHT_DIR_COUNT ][ LASER_ANIM_FRAME_COUNT ];  // Player Laser animations
            W_Image g_EnemyLaserAnims [ STRAIGHT_DIR_COUNT ][ LASER_ANIM_FRAME_COUNT ];   // Enemy Laser animations

            // ---- Keys

            W_Image g_KeyAnims [ KEY_COUNT ][ KEY_ANIM_FRAME_COUNT ];    // The keys
            W_Image g_KeyIcons [ KEY_COUNT ];           // Key icons
            W_Image g_KeyIconsEmpty [ KEY_COUNT ];      // Key icon outlines
            W_Image g_KeyPanels [ KEY_COUNT ];          // The key panels

            // ---- Explosion Animation

            W_Image g_ExplosionAnim [ EXPLOSION_ANIM_FRAME_COUNT ];  // Explosion animation

        // ---- Sounds ------------------------------------------------------------------------

            // ---- Intro ---------------------------------------------------------------------

                W_Sound g_IntroMusic;                   // Intro music

            // ---- Interface -----------------------------------------------------------------

                W_Sound g_BrowseSound;                  // Browsing
                W_Sound g_SelectSound;                  // Selection

            // ---- Rooms ---------------------------------------------------------------------

                W_Sound g_DoorOpenSound;                // Door opening
                W_Sound g_LightsFlickerSound;           // Lights flickering sound

            // ---- Keys ----------------------------------------------------------------------

                W_Sound g_GetKeySound;                  // Picking up a key
                W_Sound g_PanelActivateSound;           // Activating a key panel
                W_Sound g_AllPanelsActivatedSound;      // All panels have been activated

            // ---- Weapons -------------------------------------------------------------------

                W_Sound g_PlayerLaserSound;             // Player laser firing
                W_Sound g_EnemyLaserSound;              // Enemy laser firing
                W_Sound g_ExplosionSound;               // Explosions

            // ---- Game Over -----------------------------------------------------------------

                W_Sound g_GameOverSound;                // Game over

        // ---- Congrats screen ---------------------------------------------------------------

            W_Image g_CongratsScreen;                   // The congrats screen

        // ---- Game Over screen --------------------------------------------------------------

            W_Image g_GameOverScreen;                   // The Game Over screen

    // ---- Gameplay --------------------------------------------------------------------------

        Player g_Player;                                // The player

        Room g_CurrRoom;                                // The current room

        Key g_Key;                                      // The key

        Droid g_EnemyDroids [ ENEMY_DROID_COUNT ];      // The droid population
        Explosion g_Explosions [ MAX_EXPLOSION_COUNT ]; // Explosions
        Laser g_Lasers [ MAX_LASER_COUNT ];             // Lasers

        // The fortress map (Y x X rooms)

        int g_iRooms [ FORTRESS_WIDTH ][ FORTRESS_HEIGHT ] =
        {
            ROOM_TYPE_PEDESTAL, ROOM_TYPE_GUARD, ROOM_TYPE_NORMAL, ROOM_TYPE_GUARD, ROOM_TYPE_PEDESTAL,
            ROOM_TYPE_GUARD,    ROOM_TYPE_NULL,  ROOM_TYPE_NULL,   ROOM_TYPE_NULL,  ROOM_TYPE_GUARD,
            ROOM_TYPE_NORMAL,   ROOM_TYPE_GUARD, ROOM_TYPE_KEY,    ROOM_TYPE_NULL,  ROOM_TYPE_NORMAL,
            ROOM_TYPE_GUARD,    ROOM_TYPE_NULL,  ROOM_TYPE_NULL,   ROOM_TYPE_NULL,  ROOM_TYPE_GUARD,
            ROOM_TYPE_PEDESTAL, ROOM_TYPE_GUARD, ROOM_TYPE_NORMAL, ROOM_TYPE_GUARD, ROOM_TYPE_PEDESTAL
        };

        int g_iIsGameOver;                              // Is the game over?
        unsigned int g_iGameOverTime;                   // When was the game completed?

    // ---- Interface -------------------------------------------------------------------------

        W_TimerHandle g_MapCursorBlinkTimer;            // Map cursor blink timer

    // ---- Scripting -------------------------------------------------------------------------

        int g_iAmbientThreadIndex;                      // Ambient script thread index
        int g_iBlueDroidThreadIndex;                    // Blue droid script index
        int g_iGreyDroidThreadIndex;                    // Grey droid script index
        int g_iRedDroidThreadIndex;                     // Red droid script index

// ---- Function Prototypes -------------------------------------------------------------------

    void Init ();
    void InitGame ();
    void ShutDown ();

    void SetGameState ( int iState );
    void HandleState ();

    void DrawGameScreen ();
    void DrawInterface ();

    void InitRoom ( int iType );
    int IsRoomValid ( int iX, int iY );

    void OpenDoor ( Door & AnimDoor );
    void UpdateDoor ( Door & AnimDoor );
    void DrawDoors ();
    
    void ActivateKey ( int iColor );
    void DeactivateKey ();
    void HandleKey ();

    void InitDroid ( Droid & NewDroid, int iX, int iY, int iDir, int iType, float fEnergy );
    void DrawDroid ( Droid & DrawDroid );
    void MoveDroid ( Droid & MoveDroid, int iDir, int iDist );
    void FireDroidGun ( Droid & DroidGun, int iType );
    void DamageDroid ( Droid & DamageDroid, float fDamage );
    void GetDroidCenter ( Droid & CenterDroid, W_Point & Point );
    void SetDroidCenter ( Droid & CenterDroid, int iX, int iY );

    void ResetLasers ();
    void AddLaser ( int iX, int iY, int iDir, int iType, int iSpeed );
    void DrawLasers ();
    void UpdateLasers ();
    void HandleLaserDroidCollision ( int iLaser, Droid & CollideDroid, int iDroidType );

    void ResetExplosions ();
    void AddExplosion ( int iX, int iY );
    void DrawExplosions ();
    void UpdateExplosions ();

    void HAPI_GetRandInRange ( int iThreadIndex );
    void HAPI_ToggleRoomLights ( int iThreadIndex );
    void HAPI_MoveEnemyDroid ( int iThreadIndex );
    void HAPI_GetEnemyDroidX ( int iThreadIndex );
    void HAPI_GetEnemyDroidY ( int iThreadIndex );
    void HAPI_IsEnemyDroidAlive ( int iThreadIndex );
    void HAPI_FireEnemyDroidGun ( int iThreadIndex );
    void HAPI_GetPlayerDroidX ( int iThreadIndex );
    void HAPI_GetPlayerDroidY ( int iThreadIndex );

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   ResetExplosions ()
    *
    *   Clears all explosions.
    */

    void ResetsExplosions ()
    {
        // Find the next free explosion

        for ( int iCurrExplosion = 0; iCurrExplosion < MAX_EXPLOSION_COUNT; ++ iCurrExplosion )
            g_Explosions [ iCurrExplosion ].iIsActive = FALSE;
    }

    /******************************************************************************************
    *
    *   AddExplosion ()
    *
    *   Adds an explosion to the screen.
    */

    void AddExplosion ( int iX, int iY )
    {
        // Find the next free explosion

        for ( int iCurrExplosion = 0; iCurrExplosion < MAX_EXPLOSION_COUNT; ++ iCurrExplosion )
        {
            // Is this explosion free?

            if ( ! g_Explosions [ iCurrExplosion ].iIsActive )
            {
                // Set the explosion's location

                g_Explosions [ iCurrExplosion ].iX = iX;
                g_Explosions [ iCurrExplosion ].iY = iY;

                // Reset the explosion animation

                g_Explosions [ iCurrExplosion ].iCurrFrame = 0;
                g_Explosions [ iCurrExplosion ].iLastFrameUpdateTime = W_GetTickCount ();

                // Activate the explosion

                g_Explosions [ iCurrExplosion ].iIsActive = TRUE;

                // Play the explosion sound

                W_PlaySound ( g_ExplosionSound );

                break;
            }
        }
    }

    /******************************************************************************************
    *
    *   DrawExplosions ()
    *
    *   Draws the onscreen explosions.
    */

    void DrawExplosions ()
    {
        for ( int iCurrExplosion = 0; iCurrExplosion < MAX_EXPLOSION_COUNT; ++ iCurrExplosion )
        {
            // Is this explosion active?

            if ( g_Explosions [ iCurrExplosion ].iIsActive )
            {
                W_Image * CurrFrame = & g_ExplosionAnim [ g_Explosions [ iCurrExplosion ].iCurrFrame ];

                int iX = g_Explosions [ iCurrExplosion ].iX - EXPLOSION_ANIM_WIDTH / 2;
                int iY = g_Explosions [ iCurrExplosion ].iY - EXPLOSION_ANIM_HEIGHT / 2;

                W_BlitImage ( * CurrFrame, iX, iY );
            }
        }
    }

    /******************************************************************************************
    *
    *   UpdateExplosions ()
    *
    *   Updates the explosion animations.
    */

    void UpdateExplosions ()
    {
        for ( int iCurrExplosion = 0; iCurrExplosion < MAX_EXPLOSION_COUNT; ++ iCurrExplosion )
        {
            // Is this explosion active?

            if ( g_Explosions [ iCurrExplosion ].iIsActive )
            {
                // If the animation timer has elapsed, update the frame

                if ( W_GetTickCount () > g_Explosions [ iCurrExplosion ].iLastFrameUpdateTime + EXPLOSION_ANIM_SPEED )
                {
                    // Move to the next frame and reset the animation timer

                    ++ g_Explosions [ iCurrExplosion ].iCurrFrame;
                    g_Explosions [ iCurrExplosion ].iLastFrameUpdateTime = W_GetTickCount ();

                    // If the animation is complete, kill the explosion

                    if ( g_Explosions [ iCurrExplosion ].iCurrFrame >= EXPLOSION_ANIM_FRAME_COUNT )
                        g_Explosions [ iCurrExplosion ].iIsActive = FALSE;
                }
            }
        }
    }

    /******************************************************************************************
    *
    *   AddLaser ()
    *
    *   Adds a laser to the onscreen projectiles.
    */

    void AddLaser ( int iX, int iY, int iDir, int iType, int iSpeed )
    {
        for ( int iCurrLaser = 0; iCurrLaser < MAX_LASER_COUNT; ++ iCurrLaser )
        {
            if ( ! g_Lasers [ iCurrLaser ].iIsActive )
            {
                switch ( iDir )
                {
                    case NORTH:
                        iDir = STRAIGHT_NORTH;
                        break;

                    case SOUTH:
                        iDir = STRAIGHT_SOUTH;
                        break;

                    case EAST:
                        iDir = STRAIGHT_EAST;
                        break;

                    case WEST:
                        iDir = STRAIGHT_WEST;
                        break;
                }

                // Set the X, Y location

                g_Lasers [ iCurrLaser ].iX = iX;
                g_Lasers [ iCurrLaser ].iY = iY;

                // Set the direction, speed and type

                g_Lasers [ iCurrLaser ].iDir = iDir;
                g_Lasers [ iCurrLaser ].iType = iType;
                g_Lasers [ iCurrLaser ].iSpeed = iSpeed;

                // Initialize the animation

                g_Lasers [ iCurrLaser ].iCurrFrame = 0;
                g_Lasers [ iCurrLaser ].iLastFrameUpdateTime = W_GetTickCount ();

                // Activate the laser

                g_Lasers [ iCurrLaser ].iIsActive = TRUE;

                // Play sound

                if ( iType == LASER_TYPE_PLAYER )
                    W_PlaySound ( g_PlayerLaserSound );
                else
                    W_PlaySound ( g_EnemyLaserSound );

                break;
            }
        }
    }

    /******************************************************************************************
    *
    *   ResetLasers ()
    *
    *   Clears the laser array.
    */

    void ResetLasers ()
    {
        for ( int iCurrLaser = 0; iCurrLaser < MAX_LASER_COUNT; ++ iCurrLaser )
            g_Lasers [ iCurrLaser ].iIsActive = FALSE;
    }

    /******************************************************************************************
    *
    *   DrawLasers ()
    *
    *   Draws the currently active lasers.
    */

    void DrawLasers ()
    {
        for ( int iCurrLaser = 0; iCurrLaser < MAX_LASER_COUNT; ++ iCurrLaser )
        {
            if ( g_Lasers [ iCurrLaser ].iIsActive )
            {
                // Determine which laser sprite to use

                W_Image * LaserSprite = NULL;
                switch ( g_Lasers [ iCurrLaser ].iDir )
                {
                    case STRAIGHT_NORTH:
                        if ( g_Lasers [ iCurrLaser ].iType == LASER_TYPE_PLAYER )
                            LaserSprite = & g_PlayerLaserAnims [ STRAIGHT_NORTH ][ g_Lasers [ iCurrLaser ].iCurrFrame ];
                        else
                            LaserSprite = & g_EnemyLaserAnims [ STRAIGHT_NORTH ][ g_Lasers [ iCurrLaser ].iCurrFrame ];
                        break;

                    case STRAIGHT_SOUTH:
                        if ( g_Lasers [ iCurrLaser ].iType == LASER_TYPE_PLAYER )
                            LaserSprite = & g_PlayerLaserAnims [ STRAIGHT_SOUTH ][ g_Lasers [ iCurrLaser ].iCurrFrame ];
                        else
                            LaserSprite = & g_EnemyLaserAnims [ STRAIGHT_SOUTH ][ g_Lasers [ iCurrLaser ].iCurrFrame ];
                        break;

                    case STRAIGHT_EAST:
                        if ( g_Lasers [ iCurrLaser ].iType == LASER_TYPE_PLAYER )
                            LaserSprite = & g_PlayerLaserAnims [ STRAIGHT_EAST ][ g_Lasers [ iCurrLaser ].iCurrFrame ];
                        else
                            LaserSprite = & g_EnemyLaserAnims [ STRAIGHT_EAST ][ g_Lasers [ iCurrLaser ].iCurrFrame ];
                        break;

                    case STRAIGHT_WEST:
                        if ( g_Lasers [ iCurrLaser ].iType == LASER_TYPE_PLAYER )
                            LaserSprite = & g_PlayerLaserAnims [ STRAIGHT_WEST ][ g_Lasers [ iCurrLaser ].iCurrFrame ];
                        else
                            LaserSprite = & g_EnemyLaserAnims [ STRAIGHT_WEST ][ g_Lasers [ iCurrLaser ].iCurrFrame ];
                        break;
                }

                // Update the laser animation

                if ( W_GetTickCount () - g_Lasers [ iCurrLaser ].iLastFrameUpdateTime > LASER_ANIM_SPEED )
                {
                    // Move to the next frame if the animation isn't finished

                    if ( g_Lasers [ iCurrLaser ].iCurrFrame < LASER_ANIM_FRAME_COUNT - 1 )
                    {
                        ++ g_Lasers [ iCurrLaser ].iCurrFrame;
                        g_Lasers [ iCurrLaser ].iLastFrameUpdateTime = W_GetTickCount ();
                    }
                }

                // Draw the laser

                W_BlitImage ( * LaserSprite, g_Lasers [ iCurrLaser ].iX - LaserSprite->ClipRect.iX0, g_Lasers [ iCurrLaser ].iY - LaserSprite->ClipRect.iY0 );
            }
        }
    }

    /******************************************************************************************
    *
    *   UpdateLasers ()
    *
    *   Moves the currently active lasers and checks for collisions.
    */

    void UpdateLasers ()
    {
        for ( int iCurrLaser = 0; iCurrLaser < MAX_LASER_COUNT; ++ iCurrLaser )
        {
            if ( g_Lasers [ iCurrLaser ].iIsActive )
            {
                // Move the laser along its path

                switch ( g_Lasers [ iCurrLaser ].iDir )
                {
                    case STRAIGHT_NORTH:
                        g_Lasers [ iCurrLaser ].iY -= g_Lasers [ iCurrLaser ].iSpeed;
                        break;

                    case STRAIGHT_SOUTH:
                        g_Lasers [ iCurrLaser ].iY += g_Lasers [ iCurrLaser ].iSpeed;
                        break;

                    case STRAIGHT_EAST:
                        g_Lasers [ iCurrLaser ].iX += g_Lasers [ iCurrLaser ].iSpeed;
                        break;

                    case STRAIGHT_WEST:
                        g_Lasers [ iCurrLaser ].iX -= g_Lasers [ iCurrLaser ].iSpeed;
                        break;
                }

                // Deactivate the laser if it moves beyond the bounds of the screen

                if ( g_Lasers [ iCurrLaser ].iX < 0 || g_Lasers [ iCurrLaser ].iX > W_GetScreenXMax () ||
                     g_Lasers [ iCurrLaser ].iY < 0 || g_Lasers [ iCurrLaser ].iY > W_GetScreenYMax () )
                    g_Lasers [ iCurrLaser ].iIsActive = FALSE;

                // Check for collisions between lasers and droids

                for ( int iCurrDroid = 0; iCurrDroid < ENEMY_DROID_COUNT; ++ iCurrDroid )
                    HandleLaserDroidCollision ( iCurrLaser, g_EnemyDroids [ iCurrDroid ], DROID_TYPE_ENEMY );

                HandleLaserDroidCollision ( iCurrLaser, g_Player.Droid, DROID_TYPE_PLAYER );
            }
        }
    }

    /******************************************************************************************
    *
    *   HandleLaserDroidCollision ()
    *
    *   Handles a laser/droid collision.
    */

    void HandleLaserDroidCollision ( int iLaser, Droid & CollideDroid, int iDroidType )
    {
        W_Rect LaserSpriteRect;
        W_Rect LaserRect;

        if ( g_Lasers [ iLaser ].iType == LASER_TYPE_PLAYER )
            LaserSpriteRect = g_PlayerLaserAnims [ g_Lasers [ iLaser ].iDir ][ g_Lasers [ iLaser ].iCurrFrame ].ClipRect;
        else
            LaserSpriteRect = g_EnemyLaserAnims [ g_Lasers [ iLaser ].iDir ][ g_Lasers [ iLaser ].iCurrFrame ].ClipRect;

        LaserRect.iX0 = g_Lasers [ iLaser ].iX;
        LaserRect.iY0 = g_Lasers [ iLaser ].iY;
        LaserRect.iX1 = LaserRect.iX0 + LaserSpriteRect.iX1;
        LaserRect.iY1 = LaserRect.iY0 + LaserSpriteRect.iY1;

        if ( CollideDroid.iIsActive )
        {
            W_Rect DroidSpriteRect;
            W_Rect DroidRect;

            DroidSpriteRect = g_Droids [ CollideDroid.iType ][ CollideDroid.iDir ].ClipRect;

            DroidRect.iX0 = CollideDroid.iX;
            DroidRect.iY0 = CollideDroid.iY;
            DroidRect.iX1 = DroidRect.iX0 + DroidSpriteRect.iX1;
            DroidRect.iY1 = DroidRect.iY0 + DroidSpriteRect.iY1;

            if ( W_DoRectsIntersect ( & LaserRect, & DroidRect ) && g_Lasers [ iLaser ].iType != iDroidType )
            {
                DamageDroid ( CollideDroid, LASER_DAMAGE );
                g_Lasers [ iLaser ].iIsActive = FALSE;
            }
        }
    }

    /******************************************************************************************
    *
    *   InitDroid ()
    *
    *   Initializes a droid.
    */

    void InitDroid ( Droid & NewDroid, int iX, int iY, int iDir, int iType, float fEnergy )
    {
        // Set the new droid's fields

        NewDroid.iX = iX;
        NewDroid.iY = iY;
        NewDroid.iDir = iDir;
        NewDroid.iType = iType;
        NewDroid.fEnergy = fEnergy;
        NewDroid.iLastDamageTime = 0;
        NewDroid.iLastFireTime = 0;

        // Activate the droid

        NewDroid.iIsActive = TRUE;
    }

    /******************************************************************************************
    *
    *   DrawDroid ()
    *
    *   Draws a droid to the screen.
    */

    void DrawDroid ( Droid & DrawDroid )
    {
        // Make sure the droid is active

        if ( ! DrawDroid.iIsActive )
            return;

        // Set the rattle to zero

        int iRattleX = 0,
            iRattleY = 0;

        // If the droid is currently damaged, add random rattle and check to see if the rattle
        // is over yet

        if ( W_GetTickCount () - DrawDroid.iLastDamageTime < DROID_DAMAGE_RATTLE_DUR )
        {
            iRattleX = rand () % 4;
            iRattleY = rand () % 4;
        }

        // Handle the kick from the last shot by adding to the rattle

        if ( W_GetTickCount () - DrawDroid.iLastFireTime < LASER_RECHARGE_DUR )
        {
            switch ( DrawDroid.iKickDir )
            {
                case NORTH:
                    iRattleY += LASER_KICK;
                    break;

                case SOUTH:
                    iRattleY -= LASER_KICK;
                    break;

                case EAST:
                    iRattleX -= LASER_KICK;
                    break;

                case WEST:
                    iRattleX += LASER_KICK;
                    break;
            }
        }

        // Draw the droid

        W_Image * DroidSprite = & g_Droids [ DrawDroid.iType ][ DrawDroid.iDir ];
        W_BlitImage ( * DroidSprite,
                      DrawDroid.iX + iRattleX - DroidSprite->ClipRect.iX0, DrawDroid.iY + iRattleY - DroidSprite->ClipRect.iY0 );
    }

    /******************************************************************************************
    *
    *   MoveDroid ()
    *
    *   Moves a droid the specified distance in the specified direction.
    */

    void MoveDroid ( Droid & MoveDroid, int iDir, int iDist )
    {
        // Set the direction

        MoveDroid.iDir = iDir;

        // Save the old position in case a boundary is hit

        int iPrevX = MoveDroid.iX,
            iPrevY = MoveDroid.iY;

        // Move the droid

        switch ( iDir )
        {
            case NORTH:
                MoveDroid.iY -= iDist;
                break;

            case NORTH_EAST:
                MoveDroid.iX += iDist;
                MoveDroid.iY -= iDist;
                break;

            case EAST:
                MoveDroid.iX += iDist;
                break;

            case SOUTH_EAST:
                MoveDroid.iX += iDist;
                MoveDroid.iY += iDist;
                break;

            case SOUTH:
                MoveDroid.iY += iDist;
                break;

            case SOUTH_WEST:
                MoveDroid.iX -= iDist;
                MoveDroid.iY += iDist;
                break;

            case WEST:
                MoveDroid.iX -= iDist;
                break;

            case NORTH_WEST:
                MoveDroid.iX -= iDist;
                MoveDroid.iY -= iDist;
                break;
        }

        // Check for player collisions

        if ( MoveDroid.iType == DROID_TYPE_WHITE )
        {
            W_Rect DroidRect; //W_OffsetRect ( g_Droids [ MoveDroid.iType ][ MoveDroid.iDir ].ClipRect, MoveDroid.iX, MoveDroid.iY );

            DroidRect.iX0 = MoveDroid.iX;
            DroidRect.iY0 = MoveDroid.iY;
            DroidRect.iX1 = DroidRect.iX0 + g_Droids [ MoveDroid.iType ][ MoveDroid.iDir ].ClipRect.iX1;
            DroidRect.iY1 = DroidRect.iY0 + g_Droids [ MoveDroid.iType ][ MoveDroid.iDir ].ClipRect.iY1;

            // Check for wall collisions

            W_Point DroidCenter;
            GetDroidCenter ( g_Player.Droid, DroidCenter );
        
            // Check for door trigger collisions

            if ( W_DoRectsIntersect ( & g_CurrRoom.NorthDoorTrigger, & DroidRect ) )
                OpenDoor ( g_CurrRoom.Doors [ STRAIGHT_NORTH ] );

            if ( W_DoRectsIntersect ( & g_CurrRoom.SouthDoorTrigger, & DroidRect ) )
                OpenDoor ( g_CurrRoom.Doors [ STRAIGHT_SOUTH ] );

            if ( W_DoRectsIntersect ( & g_CurrRoom.EastDoorTrigger, & DroidRect ) )
                OpenDoor ( g_CurrRoom.Doors [ STRAIGHT_EAST ] );

            if ( W_DoRectsIntersect ( & g_CurrRoom.WestDoorTrigger, & DroidRect ) )
                OpenDoor ( g_CurrRoom.Doors [ STRAIGHT_WEST ] );


            // Check for door collisions

            int iRoomChange = FALSE;

            // North wall

            if ( DroidCenter.iY < WALL_WIDTH - 1 )
            {
                if ( DroidCenter.iX > g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iX0 &&
                     DroidCenter.iX < g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iX1 )
                {
                    if ( DroidCenter.iY < g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iY0 )
                    {
                        -- g_Player.iRoomY;
                        SetDroidCenter ( g_Player.Droid, -1, W_GetScreenYMax () - WALL_WIDTH );
                        iRoomChange = TRUE;
                    }
                }
                else
                {
                    MoveDroid.iX = iPrevX;
                    MoveDroid.iY = iPrevY;
                }
            }

            // South wall

            if ( DroidCenter.iY > W_GetScreenYMax () - WALL_WIDTH )
            {
                if ( DroidCenter.iX > g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iX0 &&
                     DroidCenter.iX < g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iX1 )
                {
                    if ( DroidCenter.iY > g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iY1 )
                    {
                        ++ g_Player.iRoomY;
                        SetDroidCenter ( g_Player.Droid, -1, WALL_WIDTH - 1 );
                        iRoomChange = TRUE;
                    }
                }
                else
                {
                    MoveDroid.iX = iPrevX;
                    MoveDroid.iY = iPrevY;
                }
            }

            // East wall

            if ( DroidCenter.iX > W_GetScreenXMax () - WALL_WIDTH )
            {
                if ( DroidCenter.iY > g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iY0 &&
                     DroidCenter.iY < g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iY1 )
                {
                    if ( DroidCenter.iX < g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iX0 )
                    {
                        ++ g_Player.iRoomX;
                        SetDroidCenter ( g_Player.Droid, WALL_WIDTH - 1, -1 );
                        iRoomChange = TRUE;
                    }
                }
                else
                {
                    MoveDroid.iX = iPrevX;
                    MoveDroid.iY = iPrevY;
                }
            }

            // West wall

            if ( DroidCenter.iX < WALL_WIDTH )
            {
                if ( DroidCenter.iY > g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iY0 &&
                     DroidCenter.iY < g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iY1 )
                {
                    if ( DroidCenter.iX < g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iX1 )
                    {
                        -- g_Player.iRoomX;
                        SetDroidCenter ( g_Player.Droid, W_GetScreenXMax () - WALL_WIDTH, -1 );
                        iRoomChange = TRUE;
                    }
                }
                else
                {
                    MoveDroid.iX = iPrevX;
                    MoveDroid.iY = iPrevY;
                }
            }

            if ( iRoomChange )
                InitRoom ( g_iRooms [ g_Player.iRoomX ][ g_Player.iRoomY ] );

            // Check for collisions with other droids

            for ( int iCurrDroid = 0; iCurrDroid < ENEMY_DROID_COUNT; ++ iCurrDroid )
            {
                Droid * CurrDroid = & g_EnemyDroids [ iCurrDroid ];
                if ( g_EnemyDroids [ iCurrDroid ].iIsActive )
                {
                    W_Rect EnemyDroidRect;
                    EnemyDroidRect.iX0 = CurrDroid->iX;
                    EnemyDroidRect.iY0 = CurrDroid->iY;
                    EnemyDroidRect.iX1 = EnemyDroidRect.iX0 + g_Droids [ CurrDroid->iType ][ CurrDroid->iDir ].ClipRect.iX1;
                    EnemyDroidRect.iY1 = EnemyDroidRect.iY0 + g_Droids [ CurrDroid->iType ][ CurrDroid->iDir ].ClipRect.iY1;

                    if ( W_DoRectsIntersect ( & EnemyDroidRect, & DroidRect ) )
                        if ( rand () % DROID_COLLIDE_DAMAGE_CHANCE == 1 )
                            DamageDroid ( MoveDroid, DROID_COLLIDE_DAMAGE );
                }
            }

            // Determine if the player has picked up a key

            if ( g_Key.iIsActive )
            {
                W_Rect KeyRect;

                KeyRect.iX0 = g_Key.iX;
                KeyRect.iY0 = g_Key.iY;
                KeyRect.iX1 = KeyRect.iX0 + g_KeyAnims [ g_Key.iColor ][ g_Key.iCurrFrame ].ClipRect.iX1;
                KeyRect.iY1 = KeyRect.iY0 + g_KeyAnims [ g_Key.iColor ][ g_Key.iCurrFrame ].ClipRect.iY1;

                if ( W_DoRectsIntersect ( & DroidRect, & KeyRect ) )
                {
                    DeactivateKey ();
                    g_Player.iKeys [ g_Key.iColor ] = TRUE;
                    W_PlaySound ( g_GetKeySound );
                }
            }

            // If we're in the key room, determine if the player has activated a key panel

            if ( g_iRooms [ g_Player.iRoomX ][ g_Player.iRoomY ] == ROOM_TYPE_KEY )
            {               
                // The red panel

                if ( DroidCenter.iX >= KEY_PANEL_RED_X && DroidCenter.iY >= KEY_PANEL_RED_Y &&
                     DroidCenter.iX < KEY_PANEL_RED_X + KEY_PANEL_WIDTH &&
                     DroidCenter.iY < KEY_PANEL_RED_Y + KEY_PANEL_HEIGHT )
                {
                    if ( g_Player.iKeys [ RED ] && ! g_Player.iActiveKeyPanels [ RED ] )
                    {
                        g_Player.iActiveKeyPanels [ RED ] = TRUE;
                        W_PlaySound ( g_PanelActivateSound );
                    }
                }

                // The blue panel

                if ( DroidCenter.iX >= KEY_PANEL_BLUE_X && DroidCenter.iY >= KEY_PANEL_BLUE_Y &&
                     DroidCenter.iX < KEY_PANEL_BLUE_X + KEY_PANEL_WIDTH &&
                     DroidCenter.iY < KEY_PANEL_BLUE_Y + KEY_PANEL_HEIGHT )
                {
                    if ( g_Player.iKeys [ BLUE ] && ! g_Player.iActiveKeyPanels [ BLUE ] )
                    {
                        g_Player.iActiveKeyPanels [ BLUE ] = TRUE;
                        W_PlaySound ( g_PanelActivateSound );
                    }
                }

                // The green panel

                if ( DroidCenter.iX >= KEY_PANEL_GREEN_X && DroidCenter.iY >= KEY_PANEL_GREEN_Y &&
                     DroidCenter.iX < KEY_PANEL_GREEN_X + KEY_PANEL_WIDTH &&
                     DroidCenter.iY < KEY_PANEL_GREEN_Y + KEY_PANEL_HEIGHT )
                {
                    if ( g_Player.iKeys [ GREEN ] && ! g_Player.iActiveKeyPanels [ GREEN ] )
                    {
                        g_Player.iActiveKeyPanels [ GREEN ] = TRUE;
                        W_PlaySound ( g_PanelActivateSound );
                    }
                }

                // The yellow panel

                if ( DroidCenter.iX >= KEY_PANEL_YELLOW_X && DroidCenter.iY >= KEY_PANEL_YELLOW_Y &&
                     DroidCenter.iX < KEY_PANEL_YELLOW_X + KEY_PANEL_WIDTH &&
                     DroidCenter.iY < KEY_PANEL_YELLOW_Y + KEY_PANEL_HEIGHT )
                {
                    if ( g_Player.iKeys [ YELLOW ] && ! g_Player.iActiveKeyPanels [ YELLOW ] )
                    {
                        g_Player.iActiveKeyPanels [ YELLOW ] = TRUE;
                        W_PlaySound ( g_PanelActivateSound );
                    }
                }
            }
        }
        else
        {
            W_Point DroidCenter;
            GetDroidCenter ( MoveDroid, DroidCenter );

            // Check for non-player droid collisions

            if ( DroidCenter.iX < WALL_WIDTH - 1 || DroidCenter.iY < WALL_WIDTH - 1 ||
                 DroidCenter.iX > W_GetScreenXMax () - WALL_WIDTH || DroidCenter.iY > W_GetScreenYMax () - WALL_WIDTH )
            {
                // Reset the player's position

                MoveDroid.iX = iPrevX;
                MoveDroid.iY = iPrevY;
            }
        }
    }

    /******************************************************************************************
    *
    *   FireDroidGun ()
    *
    *   Fires a droid's weapon.
    */

    void FireDroidGun ( Droid & DroidGun, int iType )
    {
        // Make sure the droid is active

        if ( ! DroidGun.iIsActive )
            return;

        // Fire the shot if the droid isn't facing diagonally and isn't recharging

        if ( W_GetTickCount () - DroidGun.iLastFireTime > LASER_RECHARGE_DUR &&
             ( DroidGun.iDir == NORTH ||
               DroidGun.iDir == SOUTH ||
               DroidGun.iDir == EAST ||
               DroidGun.iDir == WEST ) )
        {
            // Fire the laser

            int iX = DroidGun.iX,
                iY = DroidGun.iY;

            switch ( DroidGun.iDir )
            {
                case NORTH:
                    iX += 26;
                    iY -= 17;
                    break;

                case SOUTH:
                    iX += 26;
                    iY += 59;
                    break;

                case EAST:
                    iX += 68;
                    iY += 28;
                    break;

                case WEST:
                    iX -= 8;
                    iY += 28;
                    break;
            }

            AddLaser ( iX, iY, DroidGun.iDir, iType, LASER_SPEED );

            // Start recharging and add a kick

            DroidGun.iLastFireTime = W_GetTickCount ();
            DroidGun.iKickDir = DroidGun.iDir;
        }
    }

    /******************************************************************************************
    *
    *   DamageDroid ()
    *
    *   Damages a droid.
    */

    void DamageDroid ( Droid & DamageDroid, float fDamage )
    {
        // Make sure the droid is active

        if ( ! DamageDroid.iIsActive )
            return;

        // Deal the damage

        DamageDroid.fEnergy -= fDamage;
        DamageDroid.iLastDamageTime = W_GetTickCount ();

        // If the droid is destroyed, deactivate it and add an explosion

        if ( DamageDroid.fEnergy <= 0 )
        {
            DamageDroid.iIsActive = FALSE;
            
            W_Point DroidCenter;
            GetDroidCenter ( DamageDroid, DroidCenter );
            AddExplosion ( DroidCenter.iX, DroidCenter.iY );
        }
    }

    /******************************************************************************************
    *
    *   GetDroidCenter ()
    *
    *   Calculates and returns the center of a droid.
    */

    void GetDroidCenter ( Droid & CenterDroid, W_Point & Point )
    {
        W_Image * DroidSprite = & g_Droids [ CenterDroid.iType ][ CenterDroid.iDir ];

        Point.iX = CenterDroid.iX + DroidSprite->ClipRect.iX1 / 2;
        Point.iY = CenterDroid.iY + DroidSprite->ClipRect.iY1 / 2;
    }

    /******************************************************************************************
    *
    *   SetDroidCenter ()
    *
    *   Moves a droid to the specified location by its center.
    */

    void SetDroidCenter ( Droid & CenterDroid, int iX, int iY )
    {
        W_Image * DroidSprite = & g_Droids [ CenterDroid.iType ][ CenterDroid.iDir ];

        if ( iX != -1 )
            CenterDroid.iX = iX - DroidSprite->ClipRect.iX1 / 2;

        if ( iY != -1 )
            CenterDroid.iY = iY - DroidSprite->ClipRect.iY1 / 2;
    }

    /******************************************************************************************
    *
    *   InitRoom ()
    *
    *   Initializes the room.
    */

    void InitRoom ( int iType )
    {
        // Turn off the lights

        g_CurrRoom.iLights = LIGHTS_OFF;

        // Initialize the doors

        for ( int iCurrDoor = 0; iCurrDoor < STRAIGHT_DIR_COUNT; ++ iCurrDoor )
            g_CurrRoom.Doors [ iCurrDoor ].iState = DOOR_STATE_CLOSED;

        // Initialize the key

        if ( iType == ROOM_TYPE_PEDESTAL )
        {
            // Room 0, 0 is the red key pedestal

            if ( g_Player.iRoomX == 0 && g_Player.iRoomY == 0 )
                ActivateKey ( BLUE );

            // Room MAX, 0 is the green key pedestal

            if ( g_Player.iRoomX == FORTRESS_WIDTH - 1 && g_Player.iRoomY == 0 )
                ActivateKey ( RED );

            // Room 0, MAX is the yellow key pedestal

            if ( g_Player.iRoomX == 0 && g_Player.iRoomY == FORTRESS_WIDTH - 1 )
                ActivateKey ( YELLOW );

            // Room MAX, MAX is the yellow key pedestal

            if ( g_Player.iRoomX == FORTRESS_WIDTH - 1 && g_Player.iRoomY == FORTRESS_HEIGHT - 1 )
                ActivateKey ( GREEN );
        }
        else
        {
            DeactivateKey ();
        }

        // Clear any remaining lasers and explosions from the last room

        ResetLasers ();
        ResetsExplosions ();

        // ---- Initialize the droid population

        // Determine the droid type based on the room type

        int iDroidType;
        switch ( iType )
        {
            case ROOM_TYPE_NORMAL:

                XS_StartScript ( g_iBlueDroidThreadIndex );
                XS_StopScript ( g_iGreyDroidThreadIndex );
                XS_StopScript ( g_iRedDroidThreadIndex );

                iDroidType = DROID_TYPE_BLUE;

                break;

            case ROOM_TYPE_GUARD:

                XS_StopScript ( g_iBlueDroidThreadIndex );
                XS_StartScript ( g_iGreyDroidThreadIndex );
                XS_StopScript ( g_iRedDroidThreadIndex );

                iDroidType = DROID_TYPE_GREY;

                break;

            case ROOM_TYPE_PEDESTAL:

                XS_StopScript ( g_iBlueDroidThreadIndex );
                XS_StopScript ( g_iGreyDroidThreadIndex );
                XS_StartScript ( g_iRedDroidThreadIndex );

                iDroidType = DROID_TYPE_RED;

                break;
        }

        // ---- Initialize the droids, unless we're in the key room       

        for ( int iCurrDroid = 0; iCurrDroid < ENEMY_DROID_COUNT; ++ iCurrDroid )
            g_EnemyDroids [ iCurrDroid ].iIsActive = FALSE;

        if ( iType != ROOM_TYPE_KEY )
        {
            for ( int iCurrDroid = 0; iCurrDroid < ENEMY_DROID_COUNT; ++ iCurrDroid )
            {
                InitDroid ( g_EnemyDroids [ iCurrDroid ],
                            W_GetRandInRange ( WALL_WIDTH, W_GetScreenXMax () - WALL_WIDTH - DROID_WIDTH ),
                            W_GetRandInRange ( WALL_WIDTH, W_GetScreenYMax () - WALL_WIDTH - DROID_WIDTH ),
                            rand () % 4,
                            iDroidType,
                            MAX_ENERGY / 2 );
            }
        }

        // Clear the doors and triggers

        g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iX0 = 0;
        g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iY0 = 0;
        g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iX1 = 0;
        g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iY1 = 0;
        g_CurrRoom.NorthDoorTrigger.iX0 = 0;
        g_CurrRoom.NorthDoorTrigger.iY0 = 0;
        g_CurrRoom.NorthDoorTrigger.iX1 = 0;
        g_CurrRoom.NorthDoorTrigger.iY1 = 0;

        g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iX0 = 0;
        g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iY0 = 0;
        g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iX1 = 0;
        g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iY1 = 0;
        g_CurrRoom.SouthDoorTrigger.iX0 = 0;
        g_CurrRoom.SouthDoorTrigger.iY0 = 0;
        g_CurrRoom.SouthDoorTrigger.iX1 = 0;
        g_CurrRoom.SouthDoorTrigger.iY1 = 0;

        g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iX0 = 0;
        g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iY0 = 0;
        g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iX1 = 0;
        g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iY1 = 0;
        g_CurrRoom.EastDoorTrigger.iX0 = 0;
        g_CurrRoom.EastDoorTrigger.iY0 = 0;
        g_CurrRoom.EastDoorTrigger.iX1 = 0;
        g_CurrRoom.EastDoorTrigger.iY1 = 0;

        g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iX0 = 0;
        g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iY0 = 0;
        g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iX1 = 0;
        g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iY1 = 0;
        g_CurrRoom.WestDoorTrigger.iX0 = 0;
        g_CurrRoom.WestDoorTrigger.iY0 = 0;
        g_CurrRoom.WestDoorTrigger.iX1 = 0;
        g_CurrRoom.WestDoorTrigger.iY1 = 0;

        // Initialize the door and trigger rectangles

        // Set the north door's bounding rectangle

        if ( IsRoomValid ( g_Player.iRoomX, g_Player.iRoomY - 1 ) )
        {
            g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iX0 = 274;
            g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iY0 = 0;
            g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iX1 = 365;
            g_CurrRoom.Doors [ STRAIGHT_NORTH ].Rect.iY1 = 19;

            g_CurrRoom.NorthDoorTrigger.iX0 = 239;
            g_CurrRoom.NorthDoorTrigger.iY0 = 00;
            g_CurrRoom.NorthDoorTrigger.iX1 = 379;
            g_CurrRoom.NorthDoorTrigger.iY1 = 100;
        }

        // Set the south door's bounding rectangle

        if ( IsRoomValid ( g_Player.iRoomX, g_Player.iRoomY + 1 ) )
        {
            g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iX0 = 274;
            g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iY0 = 459;
            g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iX1 = 365;
            g_CurrRoom.Doors [ STRAIGHT_SOUTH ].Rect.iY1 = 479;

            g_CurrRoom.SouthDoorTrigger.iX0 = 239;
            g_CurrRoom.SouthDoorTrigger.iY0 = 379;
            g_CurrRoom.SouthDoorTrigger.iX1 = 379;
            g_CurrRoom.SouthDoorTrigger.iY1 = 479;
        }

        // Set the east door's bounding rectangle

        if ( IsRoomValid ( g_Player.iRoomX + 1, g_Player.iRoomY ) )
        {
            g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iX0 = 619;
            g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iY0 = 193;
            g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iX1 = 639;
            g_CurrRoom.Doors [ STRAIGHT_EAST ].Rect.iY1 = 284;

            g_CurrRoom.EastDoorTrigger.iX0 = 539;
            g_CurrRoom.EastDoorTrigger.iY0 = 179;
            g_CurrRoom.EastDoorTrigger.iX1 = 639;
            g_CurrRoom.EastDoorTrigger.iY1 = 299;
        }

        // Set the west door's bounding rectangle

        if ( IsRoomValid ( g_Player.iRoomX - 1, g_Player.iRoomY ) )
        {
            g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iX0 = 0;
            g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iY0 = 193;
            g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iX1 = 19;
            g_CurrRoom.Doors [ STRAIGHT_WEST ].Rect.iY1 = 284;

            g_CurrRoom.WestDoorTrigger.iX0 = 0;
            g_CurrRoom.WestDoorTrigger.iY0 = 179;
            g_CurrRoom.WestDoorTrigger.iX1 = 99;
            g_CurrRoom.WestDoorTrigger.iY1 = 299;
        }
    }

    /******************************************************************************************
    *
    *   OpenDoor ()
    *
    *   Opens a door
    */

    void OpenDoor ( Door & AnimDoor )
    {
        // Make sure the door isn't already open

        if ( AnimDoor.iState == DOOR_STATE_CLOSED )
        {
            // Set the current frame to zero, set the animation frame timer, and the door state
            // to open

            AnimDoor.iCurrFrame = 0;
            AnimDoor.iLastFrameUpdateTime = W_GetTickCount ();
            AnimDoor.iState = DOOR_STATE_OPENING;

            // Play the opening sound

            W_PlaySound ( g_DoorOpenSound );
        }
    }

    /******************************************************************************************
    *
    *   UpdateDoors ()
    *
    *   Updates the animations of the room's doors.
    */

    void UpdateDoors ()
    {
        for ( int iCurrDoor = 0; iCurrDoor < STRAIGHT_DIR_COUNT; ++ iCurrDoor )
        {
            // Make sure the door is still opening

            if ( g_CurrRoom.Doors [ iCurrDoor ].iState == DOOR_STATE_OPENING )
            {
                // If the animation frame timer has elapsed, update the frame

                if ( ( W_GetTickCount () - g_CurrRoom.Doors [ iCurrDoor ].iLastFrameUpdateTime ) > DOOR_ANIM_SPEED )
                {
                    // Move to the next frame

                    ++ g_CurrRoom.Doors [ iCurrDoor ].iCurrFrame;

                    // Is the door now open?

                    if ( g_CurrRoom.Doors [ iCurrDoor ].iCurrFrame == DOOR_ANIM_FRAME_COUNT - 1 )
                    {
                        // Yes, so update the state

                        g_CurrRoom.Doors [ iCurrDoor ].iState = DOOR_STATE_OPEN;
                    }
                    else
                    {
                        // No, so update the animation frame timer

                        g_CurrRoom.Doors [ iCurrDoor ].iLastFrameUpdateTime = W_GetTickCount ();
                    }
                }
            }
        }
    }

    /******************************************************************************************
    *
    *   DrawDoors ()
    *
    *   Draws the doors.
    */

    void DrawDoors ()
    {
        int iLightFlag = g_CurrRoom.iLights;
        if ( g_iRooms [ g_Player.iRoomX ][ g_Player.iRoomY ] == ROOM_TYPE_KEY )
            iLightFlag = LIGHTS_OFF;

        if ( g_CurrRoom.Doors [ STRAIGHT_NORTH ].iState != DOOR_STATE_CLOSED )
            W_BlitImage ( g_DoorAnims [ STRAIGHT_NORTH ][ iLightFlag ][ g_CurrRoom.Doors [ STRAIGHT_NORTH ].iCurrFrame ], 270, 0 );

        if ( g_CurrRoom.Doors [ STRAIGHT_SOUTH ].iState != DOOR_STATE_CLOSED )
            W_BlitImage ( g_DoorAnims [ STRAIGHT_SOUTH ][ iLightFlag ][ g_CurrRoom.Doors [ STRAIGHT_SOUTH ].iCurrFrame ], 270, 431 );

        if ( g_CurrRoom.Doors [ STRAIGHT_EAST ].iState != DOOR_STATE_CLOSED )
            W_BlitImage ( g_DoorAnims [ STRAIGHT_EAST ][ iLightFlag ][ g_CurrRoom.Doors [ STRAIGHT_EAST ].iCurrFrame ], 591, 191 );

        if ( g_CurrRoom.Doors [ STRAIGHT_WEST ].iState != DOOR_STATE_CLOSED )
            W_BlitImage ( g_DoorAnims [ STRAIGHT_WEST ][ iLightFlag ][ g_CurrRoom.Doors [ STRAIGHT_WEST ].iCurrFrame ], 0, 191 );
    }

    /******************************************************************************************
    *
    *   IsRoomValid ()
    *
    *   Determines if the room at the specified index is valid.
    */

    int IsRoomValid ( int iX, int iY )
    {
        // If the room lies beyond the map of the fortress, it's invalid

        if ( iX < 0 || iY < 0 )
            return FALSE;

        if ( iX >= FORTRESS_WIDTH || iY >= FORTRESS_HEIGHT )
            return FALSE;

        // If the room is null, it's invalid

        if ( g_iRooms [ iX ][ iY ] == ROOM_TYPE_NULL )
            return FALSE;

        // Any other room is valid

        return TRUE;
    }

    /******************************************************************************************
    *
    *   ActivateKey ()
    *
    *   Activates the key with a specified color.
    */

    void ActivateKey ( int iColor )
    {
        // Make sure the speciifed key hasn't already been collected

        if ( g_Player.iKeys [ iColor ] )
            return;

        g_Key.iIsActive = TRUE;
        g_Key.iX = KEY_X;
        g_Key.iY = KEY_Y;
        g_Key.iColor = iColor;
        g_Key.iCurrFrame = 0;
        g_Key.iCurrFloatFrame = 0;
        g_Key.iLastFrameUpdateTime = W_GetTickCount ();
    }

    /******************************************************************************************
    *
    *   DeactivateKey ()
    *
    *   Deactivates the key.
    */

    void DeactivateKey ()
    {
        g_Key.iIsActive = FALSE;
    }

    /******************************************************************************************
    *
    *   HandleKey ()
    *
    *   Draws and updates the kye.
    */

    void HandleKey ()
    {
        // Make sure the key is active

        if ( ! g_Key.iIsActive )
            return;

        // Calculate the vertical floating offset

        int iFloatOffsetY = ( int ) ( sin ( g_Key.iCurrFloatFrame ) * KEY_FLOAT_SCALE );

        // Draw the key

        W_BlitImage ( g_KeyAnims [ g_Key.iColor ][ g_Key.iCurrFrame ], g_Key.iX, g_Key.iY - iFloatOffsetY );

        // Update the animation

        if ( W_GetTickCount () - g_Key.iLastFrameUpdateTime > KEY_ANIM_SPEED )
        {
            ++ g_Key.iCurrFrame;
            if ( g_Key.iCurrFrame >= KEY_ANIM_FRAME_COUNT )
                g_Key.iCurrFrame = 0;

            g_Key.iCurrFloatFrame += KEY_FLOAT_FRAME_STEP;
            if ( g_Key.iCurrFloatFrame > MAX_RADIAN )
                g_Key.iCurrFloatFrame = 0;

            g_Key.iLastFrameUpdateTime = W_GetTickCount ();
        }
    }

    /******************************************************************************************
    *
    *   DrawInterface ()
    *
    *   Draws the onscreen interface.
    */

    void DrawInterface ()
    {
        // Draw the energy frame

        W_BlitImage ( g_EnergyFrame, 21, 43 );

        // Draw the energy bars

        W_Image * pEnergyBarClear;
        W_Image * pEnergyBarFull;

        if ( g_Player.Droid.fEnergy < 3 )
        {
            pEnergyBarFull = & g_EnergyBars [ 2 ][ 0 ];
            pEnergyBarClear = & g_EnergyBars [ 2 ][ 1 ];
        }
        else if ( g_Player.Droid.fEnergy < 6 )
        {
            pEnergyBarFull = & g_EnergyBars [ 1 ][ 0 ];
            pEnergyBarClear = & g_EnergyBars [ 1 ][ 1 ];
        }
        else
        {
            pEnergyBarFull = & g_EnergyBars [ 0 ][ 0 ];
            pEnergyBarClear = & g_EnergyBars [ 0 ][ 1 ];
        }

        int iCurrBarX = 21;
        for ( int iCurrBar = 0; iCurrBar < MAX_ENERGY; ++ iCurrBar )
        {
            if ( iCurrBar + 1 > g_Player.Droid.fEnergy )
                W_BlitImage ( * pEnergyBarClear, iCurrBarX, 19 );
            else
                W_BlitImage ( * pEnergyBarFull, iCurrBarX, 19 );

            iCurrBarX += ENERGY_BAR_WIDTH + 2;
        }

        // Draw the key frame

        W_BlitImage ( g_KeysFrame, 470, 43 );

        // Draw the key icons

        int iCurrKeyIconX = 489;
        for ( int iCurrKey = 0; iCurrKey < KEY_COUNT; ++ iCurrKey )
        {
            if ( g_Player.iKeys [ iCurrKey ] )
                W_BlitImage ( g_KeyIcons [ iCurrKey ], iCurrKeyIconX, 19 );
            else
                W_BlitImage ( g_KeyIconsEmpty [ iCurrKey ], iCurrKeyIconX, 19 );
            iCurrKeyIconX += KEY_ICON_WIDTH + 2;
        }
    }

    /******************************************************************************************
    *
    *   DrawGameScreen ()
    *
    *   Draws the current game screen.
    */

    void DrawGameScreen ()
    {
        // ---- Draw the current room background

        switch ( g_iRooms [ g_Player.iRoomX ][ g_Player.iRoomY ] )
        {
            // Normal room

            case ROOM_TYPE_NORMAL:
            case ROOM_TYPE_GUARD:

                // Take the lighting into account as long as the doors are all closed (so the
                // bitmap colors line up)

                if ( g_CurrRoom.iLights == LIGHTS_ON )
                    W_BlitImage ( g_NormalRoomOn, 0, 0 );
                else
                    W_BlitImage ( g_NormalRoomOff, 0, 0 );

                break;
            
            // Pedestal room

            case ROOM_TYPE_PEDESTAL:

                if ( g_CurrRoom.iLights == LIGHTS_ON )
                    W_BlitImage ( g_PedestalRoomOn, 0, 0 );
                else
                    W_BlitImage ( g_PedestalRoomOff, 0, 0 );

               break;

            // Key room

            case ROOM_TYPE_KEY:

               W_BlitImage ( g_KeyRoom, 0, 0 );

               // Draw the key panels

               if ( g_Player.iActiveKeyPanels [ RED ] )
                   W_BlitImage ( g_KeyPanels [ RED ], KEY_PANEL_RED_X, KEY_PANEL_RED_Y );

               if ( g_Player.iActiveKeyPanels [ BLUE ] )
                   W_BlitImage ( g_KeyPanels [ BLUE ], KEY_PANEL_BLUE_X, KEY_PANEL_BLUE_Y );

               if ( g_Player.iActiveKeyPanels [ GREEN ] )
                   W_BlitImage ( g_KeyPanels [ GREEN ], KEY_PANEL_GREEN_X, KEY_PANEL_GREEN_Y );

               if ( g_Player.iActiveKeyPanels [ YELLOW ] )
                   W_BlitImage ( g_KeyPanels [ YELLOW ], KEY_PANEL_YELLOW_X, KEY_PANEL_YELLOW_Y );

               break;
        }

        // ---- Draw and handle any necessary door animations

        DrawDoors ();
        UpdateDoors ();

        // ---- Draw the droids

        // Draw the enemies

        for ( int iCurrDroid = 0; iCurrDroid < ENEMY_DROID_COUNT; ++ iCurrDroid )
            DrawDroid ( g_EnemyDroids [ iCurrDroid ] );

        // Draw the player

        DrawDroid ( g_Player.Droid );

        // ---- Draw and update the lasers

        DrawLasers ();
        UpdateLasers ();

        // ---- Draw and update the key

        HandleKey ();

        // ---- Draw and update the explosions

        DrawExplosions ();
        UpdateExplosions ();

        // ---- Draw the interface

        DrawInterface ();
    }

    /******************************************************************************************
    *
    *   SetGameState ()
    *
    *   Sets the game state.
    */

    void SetGameState ( int iState )
    {
        // Clear out any playing sounds

        W_StopAllSounds ();

        // Shut down the old state

        switch ( g_iCurrGameState )
        {
            // Title screen

            case GAME_STATE_TITLE:

                W_FreeImage ( & g_TitleScreen );
                W_FreeImage ( & g_TitleExitHilite );

                break;

            // How to Play screen

            case GAME_STATE_HOW_TO_PLAY:
                W_FreeImage ( & g_HowToPlayScreen );
                break;

            // Loading screen

            case GAME_STATE_LOADING:
                W_FreeImage ( & g_LoadingScreen );
                break;

            // Gameplay

            case GAME_STATE_PLAY:
            {
                // Stop the ambient script

                XS_StopScript ( g_iAmbientThreadIndex );

                // ---- Free the graphics

                int iCurrDir;

                // Free the room graphics

                W_FreeImage ( & g_NormalRoomOff );
                W_FreeImage ( & g_NormalRoomOn );
                W_FreeImage ( & g_PedestalRoomOff );
                W_FreeImage ( & g_PedestalRoomOn );
                W_FreeImage ( & g_KeyRoom );

                // Free the door animations

                for ( iCurrDir = 0; iCurrDir < STRAIGHT_DIR_COUNT; ++ iCurrDir )
                {
                    for ( int iCurrDoorFrame = 0; iCurrDoorFrame < DOOR_ANIM_FRAME_COUNT; ++ iCurrDoorFrame )
                    {
                        W_FreeImage ( & g_DoorAnims [ iCurrDir ][ LIGHTS_OFF ][ iCurrDoorFrame ] );
                        W_FreeImage ( & g_DoorAnims [ iCurrDir ][ LIGHTS_ON ][ iCurrDoorFrame ] );
                    }
                }

                // Free the interface graphics

                W_FreeImage ( & g_EnergyFrame );
                W_FreeImage ( & g_KeysFrame );

                for ( int iCurrEnergyBar = 0; iCurrEnergyBar < 3; ++ iCurrEnergyBar )
                {
                    W_FreeImage ( & g_EnergyBars [ iCurrEnergyBar ][ 0 ] );
                    W_FreeImage ( & g_EnergyBars [ iCurrEnergyBar ][ 1 ] );
                }

                // Free the map graphics

                W_FreeImage ( & g_MapScreen );
                W_FreeImage ( & g_MapCursor );

                // Free the key graphics

                for ( int iCurrKey = 0; iCurrKey < KEY_COUNT; ++ iCurrKey )
                    for ( int iCurrKeyFrame = 0; iCurrKeyFrame < KEY_ANIM_FRAME_COUNT; ++ iCurrKeyFrame )
                        W_FreeImage ( & g_KeyAnims [ iCurrKey ][ iCurrKeyFrame ] );

                for ( int iCurrKeyPanel = 0; iCurrKeyPanel < KEY_COUNT; ++ iCurrKeyPanel )
                    W_FreeImage ( & g_KeyPanels [ iCurrKeyPanel ] );

                for ( int iCurrKeyIcon = 0; iCurrKeyIcon < KEY_COUNT; ++ iCurrKeyIcon )
                {
                    W_FreeImage ( & g_KeyIcons [ iCurrKeyIcon ] );
                    W_FreeImage ( & g_KeyIconsEmpty [ iCurrKeyIcon ] );
                }

                // Free the droid graphics

                for ( int iCurrDroidType = 0; iCurrDroidType < DROID_TYPE_COUNT; ++ iCurrDroidType )
                    for ( iCurrDir = 0; iCurrDir < DIR_COUNT; ++ iCurrDir )
                        W_FreeImage ( & g_Droids [ iCurrDroidType ][ iCurrDir ] );

                // Free the laser animations

                for ( iCurrDir = 0; iCurrDir < STRAIGHT_DIR_COUNT; ++ iCurrDir )
                {
                    for ( int iCurrLaserFrame = 0; iCurrLaserFrame < LASER_ANIM_FRAME_COUNT; ++ iCurrLaserFrame )
                    {
                        W_FreeImage ( & g_PlayerLaserAnims [ iCurrDir ][ iCurrLaserFrame ] );
                        W_FreeImage ( & g_EnemyLaserAnims [ iCurrDir ][ iCurrLaserFrame ] );
                    }
                }

                // Free the explosion animation

                for ( int iCurrExplosionFrame = 0; iCurrExplosionFrame < EXPLOSION_ANIM_FRAME_COUNT; ++ iCurrExplosionFrame )
                    W_FreeImage ( & g_ExplosionAnim [ iCurrExplosionFrame ] );

                // Restore the key delay

                W_EnableKeyDelay ();
                break;
            }

            // Congrats screen

            case GAME_STATE_CONGRATS:
                W_FreeImage ( & g_CongratsScreen );
                break;

            // Game over screen

            case GAME_STATE_GAME_OVER:
                W_FreeImage ( & g_GameOverScreen );
                break;
        }

        // Prepare the new state

        switch ( iState )
        {
            // Title screen

            case GAME_STATE_TITLE:

                W_LoadImage ( "Gfx/Title/BG.bmp", & g_TitleScreen );
                W_LoadImage ( "Gfx/Title/Exit.bmp", & g_TitleExitHilite );

                W_PlaySound ( g_IntroMusic );

                break;

            // How to Play screen

            case GAME_STATE_HOW_TO_PLAY:
                W_LoadImage ( "Gfx/How_To_Play/BG.bmp", & g_HowToPlayScreen );
                break;

            // Loading screen

            case GAME_STATE_LOADING:
                W_LoadImage ( "Gfx/Loading/BG.bmp", & g_LoadingScreen );
                break;

            // Gameplay

            case GAME_STATE_PLAY:
            {
                // Clear the key delay for a faster response

                W_DisableKeyDelay ();

                // Initialize the game

                InitGame ();

                // Start up the ambient script

                XS_StartScript ( g_iAmbientThreadIndex );

                break;
            }

            // Congrats screen

            case GAME_STATE_CONGRATS:
                W_LoadImage ( "Gfx/Congrats/BG.bmp", & g_CongratsScreen );
                break;

            // Game Over screen

            case GAME_STATE_GAME_OVER:
                W_LoadImage ( "Gfx/Game_Over/BG.bmp", & g_GameOverScreen );
                W_PlaySound ( g_GameOverSound );
                break;

            // End the game

            case GAME_STATE_END:
                W_Exit ();
                break;
        }

        // Set the state

        g_iCurrGameState = iState;
    }

    /******************************************************************************************
    *
    *   HandleState ()
    *
    *   Performs the actions of the current state.
    */

    void HandleState ()
    {
        W_Int64 iStartTime;
        W_Int64 iCurrTime;

        iStartTime = W_GetHighPerformanceTickCount ();

        switch ( g_iCurrGameState )
        {
            // Title screen

            case GAME_STATE_TITLE:
            {
                // The current menu item

                static int iCurrItem = TITLE_MENU_NEW_GAME;

                // Draw the title screen
                
                W_BlitImage ( g_TitleScreen, 0, 0 );

                // If exit is selected, draw it's highlight
                
                if ( iCurrItem == TITLE_MENU_EXIT )
                    W_BlitImage ( g_TitleExitHilite, 489, 457 );

                // Check for the enter key

				if ( W_GetKeyState ( W_KEY_ENTER ) )
                {
                    // If a new game was selected, begin; otherwise, exit

                    W_PlaySound ( g_SelectSound );
                    W_Delay ( 1000 );

                    if ( iCurrItem == TITLE_MENU_NEW_GAME )
					    SetGameState ( GAME_STATE_HOW_TO_PLAY );
                    else
                        SetGameState ( GAME_STATE_END );
                }

                // If the right key was pressed hilight the exit option

                if ( W_GetKeyState ( W_KEY_RIGHT ) )
                {
                    W_PlaySound ( g_BrowseSound );
                    iCurrItem = TITLE_MENU_EXIT;
                }

                // If the left key was pressed hilight the new game option
 
                if ( W_GetKeyState ( W_KEY_LEFT ) )
                {
                    W_PlaySound ( g_BrowseSound );
                    iCurrItem = TITLE_MENU_NEW_GAME;
                }

                break;
            }

            // How to Play screen

            case GAME_STATE_HOW_TO_PLAY:
            {
                // Draw the How to Play screen

                W_BlitImage ( g_HowToPlayScreen, 0, 0 );

                // If any key is pressed, move to the loading screen

				if ( W_GetAnyKeyState () )
					SetGameState ( GAME_STATE_LOADING );

                break;
            }

            // Loading screen

            case GAME_STATE_LOADING:
            {
                // Draw the loading screen

                W_BlitImage ( g_LoadingScreen, 0, 0 );
                W_BlitFrame ();

                // Load the room graphics

                W_LoadImage ( "Gfx/Rooms/BG.bmp", & g_NormalRoomOff );
                W_LoadImage ( "Gfx/Rooms/BG_Lit.bmp", & g_NormalRoomOn );
                W_LoadImage ( "Gfx/Rooms/Pedestal_BG.bmp", & g_PedestalRoomOff );
                W_LoadImage ( "Gfx/Rooms/Pedestal_BG_Lit.bmp", & g_PedestalRoomOn );
                W_LoadImage ( "Gfx/Rooms/Key_BG.bmp", & g_KeyRoom );

                // Load the door animations

                W_LoadImage ( "Gfx/Rooms/Doors/North/Open_0.bmp", & g_DoorAnims [ STRAIGHT_NORTH ][ LIGHTS_OFF ][ 0 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/North/Open_1.bmp", & g_DoorAnims [ STRAIGHT_NORTH ][ LIGHTS_OFF ][ 1 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/North/Open_2.bmp", & g_DoorAnims [ STRAIGHT_NORTH ][ LIGHTS_OFF ][ 2 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/North/Open_0_Lit.bmp", & g_DoorAnims [ STRAIGHT_NORTH ][ LIGHTS_ON ][ 0 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/North/Open_1_Lit.bmp", & g_DoorAnims [ STRAIGHT_NORTH ][ LIGHTS_ON ][ 1 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/North/Open_2_Lit.bmp", & g_DoorAnims [ STRAIGHT_NORTH ][ LIGHTS_ON ][ 2 ] );

                W_LoadImage ( "Gfx/Rooms/Doors/South/Open_0.bmp", & g_DoorAnims [ STRAIGHT_SOUTH ][ LIGHTS_OFF ][ 0 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/South/Open_1.bmp", & g_DoorAnims [ STRAIGHT_SOUTH ][ LIGHTS_OFF ][ 1 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/South/Open_2.bmp", & g_DoorAnims [ STRAIGHT_SOUTH ][ LIGHTS_OFF ][ 2 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/South/Open_0_Lit.bmp", & g_DoorAnims [ STRAIGHT_SOUTH ][ LIGHTS_ON ][ 0 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/South/Open_1_Lit.bmp", & g_DoorAnims [ STRAIGHT_SOUTH ][ LIGHTS_ON ][ 1 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/South/Open_2_Lit.bmp", & g_DoorAnims [ STRAIGHT_SOUTH ][ LIGHTS_ON ][ 2 ] );

                W_LoadImage ( "Gfx/Rooms/Doors/East/Open_0.bmp", & g_DoorAnims [ STRAIGHT_EAST ][ LIGHTS_OFF ][ 0 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/East/Open_1.bmp", & g_DoorAnims [ STRAIGHT_EAST ][ LIGHTS_OFF ][ 1 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/East/Open_2.bmp", & g_DoorAnims [ STRAIGHT_EAST ][ LIGHTS_OFF ][ 2 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/East/Open_0_Lit.bmp", & g_DoorAnims [ STRAIGHT_EAST ][ LIGHTS_ON ][ 0 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/East/Open_1_Lit.bmp", & g_DoorAnims [ STRAIGHT_EAST ][ LIGHTS_ON ][ 1 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/East/Open_2_Lit.bmp", & g_DoorAnims [ STRAIGHT_EAST ][ LIGHTS_ON ][ 2 ] );

                W_LoadImage ( "Gfx/Rooms/Doors/West/Open_0.bmp", & g_DoorAnims [ STRAIGHT_WEST ][ LIGHTS_OFF ][ 0 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/West/Open_1.bmp", & g_DoorAnims [ STRAIGHT_WEST ][ LIGHTS_OFF ][ 1 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/West/Open_2.bmp", & g_DoorAnims [ STRAIGHT_WEST ][ LIGHTS_OFF ][ 2 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/West/Open_0_Lit.bmp", & g_DoorAnims [ STRAIGHT_WEST ][ LIGHTS_ON ][ 0 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/West/Open_1_Lit.bmp", & g_DoorAnims [ STRAIGHT_WEST ][ LIGHTS_ON ][ 1 ] );
                W_LoadImage ( "Gfx/Rooms/Doors/West/Open_2_Lit.bmp", & g_DoorAnims [ STRAIGHT_WEST ][ LIGHTS_ON ][ 2 ] );

                // Load the interface graphics

                W_LoadImage ( "Gfx/Interface/Frame_Energy.bmp", & g_EnergyFrame );
                W_LoadImage ( "Gfx/Interface/Frame_Keys.bmp", & g_KeysFrame );
                W_LoadImage ( "Gfx/Interface/Energy_Bars/Blue_Full.bmp", & g_EnergyBars [ 0 ][ 0 ] );
                W_LoadImage ( "Gfx/Interface/Energy_Bars/Blue_Clear.bmp", & g_EnergyBars [ 0 ][ 1 ] );
                W_LoadImage ( "Gfx/Interface/Energy_Bars/Yellow_Full.bmp", & g_EnergyBars [ 1 ][ 0 ] );
                W_LoadImage ( "Gfx/Interface/Energy_Bars/Yellow_Clear.bmp", & g_EnergyBars [ 1 ][ 1 ] );
                W_LoadImage ( "Gfx/Interface/Energy_Bars/Red_Full.bmp", & g_EnergyBars [ 2 ][ 0 ] );
                W_LoadImage ( "Gfx/Interface/Energy_Bars/Red_Clear.bmp", & g_EnergyBars [ 2 ][ 1 ] );

                // Load the map graphics

                W_LoadImage ( "Gfx/Zone_Map/BG.bmp", & g_MapScreen );
                W_LoadImage ( "Gfx/Zone_Map/Cursor.bmp", & g_MapCursor );

                // Load the key graphics

                W_LoadImage ( "Gfx/Keys/Red/0.bmp", & g_KeyAnims [ RED ][ 0 ] );
                W_LoadImage ( "Gfx/Keys/Red/1.bmp", & g_KeyAnims [ RED ][ 1 ] );
                W_LoadImage ( "Gfx/Keys/Red/2.bmp", & g_KeyAnims [ RED ][ 2 ] );
                W_LoadImage ( "Gfx/Keys/Red/3.bmp", & g_KeyAnims [ RED ][ 3 ] );
                W_LoadImage ( "Gfx/Keys/Red/4.bmp", & g_KeyAnims [ RED ][ 4 ] );
                W_LoadImage ( "Gfx/Keys/Red/5.bmp", & g_KeyAnims [ RED ][ 5 ] );
                W_LoadImage ( "Gfx/Keys/Red/6.bmp", & g_KeyAnims [ RED ][ 6 ] );
                W_LoadImage ( "Gfx/Keys/Red/7.bmp", & g_KeyAnims [ RED ][ 7 ] );

                W_LoadImage ( "Gfx/Keys/Blue/0.bmp", & g_KeyAnims [ BLUE ][ 0 ] );
                W_LoadImage ( "Gfx/Keys/Blue/1.bmp", & g_KeyAnims [ BLUE ][ 1 ] );
                W_LoadImage ( "Gfx/Keys/Blue/2.bmp", & g_KeyAnims [ BLUE ][ 2 ] );
                W_LoadImage ( "Gfx/Keys/Blue/3.bmp", & g_KeyAnims [ BLUE ][ 3 ] );
                W_LoadImage ( "Gfx/Keys/Blue/4.bmp", & g_KeyAnims [ BLUE ][ 4 ] );
                W_LoadImage ( "Gfx/Keys/Blue/5.bmp", & g_KeyAnims [ BLUE ][ 5 ] );
                W_LoadImage ( "Gfx/Keys/Blue/6.bmp", & g_KeyAnims [ BLUE ][ 6 ] );
                W_LoadImage ( "Gfx/Keys/Blue/7.bmp", & g_KeyAnims [ BLUE ][ 7 ] );

                W_LoadImage ( "Gfx/Keys/Green/0.bmp", & g_KeyAnims [ GREEN ][ 0 ] );
                W_LoadImage ( "Gfx/Keys/Green/1.bmp", & g_KeyAnims [ GREEN ][ 1 ] );
                W_LoadImage ( "Gfx/Keys/Green/2.bmp", & g_KeyAnims [ GREEN ][ 2 ] );
                W_LoadImage ( "Gfx/Keys/Green/3.bmp", & g_KeyAnims [ GREEN ][ 3 ] );
                W_LoadImage ( "Gfx/Keys/Green/4.bmp", & g_KeyAnims [ GREEN ][ 4 ] );
                W_LoadImage ( "Gfx/Keys/Green/5.bmp", & g_KeyAnims [ GREEN ][ 5 ] );
                W_LoadImage ( "Gfx/Keys/Green/6.bmp", & g_KeyAnims [ GREEN ][ 6 ] );
                W_LoadImage ( "Gfx/Keys/Green/7.bmp", & g_KeyAnims [ GREEN ][ 7 ] );

                W_LoadImage ( "Gfx/Keys/Yellow/0.bmp", & g_KeyAnims [ YELLOW ][ 0 ] );
                W_LoadImage ( "Gfx/Keys/Yellow/1.bmp", & g_KeyAnims [ YELLOW ][ 1 ] );
                W_LoadImage ( "Gfx/Keys/Yellow/2.bmp", & g_KeyAnims [ YELLOW ][ 2 ] );
                W_LoadImage ( "Gfx/Keys/Yellow/3.bmp", & g_KeyAnims [ YELLOW ][ 3 ] );
                W_LoadImage ( "Gfx/Keys/Yellow/4.bmp", & g_KeyAnims [ YELLOW ][ 4 ] );
                W_LoadImage ( "Gfx/Keys/Yellow/5.bmp", & g_KeyAnims [ YELLOW ][ 5 ] );
                W_LoadImage ( "Gfx/Keys/Yellow/6.bmp", & g_KeyAnims [ YELLOW ][ 6 ] );
                W_LoadImage ( "Gfx/Keys/Yellow/7.bmp", & g_KeyAnims [ YELLOW ][ 7 ] );

                W_LoadImage ( "Gfx/Keys/Icons/Red.bmp", & g_KeyIcons [ RED ] );
                W_LoadImage ( "Gfx/Keys/Icons/Blue.bmp", & g_KeyIcons [ BLUE ] );
                W_LoadImage ( "Gfx/Keys/Icons/Green.bmp", & g_KeyIcons [ GREEN ] );
                W_LoadImage ( "Gfx/Keys/Icons/Yellow.bmp", & g_KeyIcons [ YELLOW ] );
                W_LoadImage ( "Gfx/Keys/Icons/Red_Empty.bmp", & g_KeyIconsEmpty [ RED ] );
                W_LoadImage ( "Gfx/Keys/Icons/Blue_Empty.bmp", & g_KeyIconsEmpty [ BLUE ] );
                W_LoadImage ( "Gfx/Keys/Icons/Green_Empty.bmp", & g_KeyIconsEmpty [ GREEN ] );
                W_LoadImage ( "Gfx/Keys/Icons/Yellow_Empty.bmp", & g_KeyIconsEmpty [ YELLOW ] );

                W_LoadImage ( "Gfx/Rooms/Key_Panels/Red_Lit.bmp", & g_KeyPanels [ RED ] );
                W_LoadImage ( "Gfx/Rooms/Key_Panels/Blue_Lit.bmp", & g_KeyPanels [ BLUE ] );
                W_LoadImage ( "Gfx/Rooms/Key_Panels/Green_Lit.bmp", & g_KeyPanels [ GREEN ] );
                W_LoadImage ( "Gfx/Rooms/Key_Panels/Yellow_Lit.bmp", & g_KeyPanels [ YELLOW ] );

                // Load the droid graphics

                W_LoadImage ( "Gfx/Droids/White/North.bmp", & g_Droids [ DROID_TYPE_WHITE ][ NORTH ] );
                W_LoadImage ( "Gfx/Droids/White/North_East.bmp", & g_Droids [ DROID_TYPE_WHITE ][ NORTH_EAST ] );
                W_LoadImage ( "Gfx/Droids/White/East.bmp", & g_Droids [ DROID_TYPE_WHITE ][ EAST ] );
                W_LoadImage ( "Gfx/Droids/White/South_East.bmp", & g_Droids [ DROID_TYPE_WHITE ][ SOUTH_EAST ] );
                W_LoadImage ( "Gfx/Droids/White/South.bmp", & g_Droids [ DROID_TYPE_WHITE ][ SOUTH ] );
                W_LoadImage ( "Gfx/Droids/White/South_West.bmp", & g_Droids [ DROID_TYPE_WHITE ][ SOUTH_WEST ] );
                W_LoadImage ( "Gfx/Droids/White/West.bmp", & g_Droids [ DROID_TYPE_WHITE ][ WEST ] );
                W_LoadImage ( "Gfx/Droids/White/North_West.bmp", & g_Droids [ DROID_TYPE_WHITE ][ NORTH_WEST ] );
                

                W_LoadImage ( "Gfx/Droids/Blue/North.bmp", & g_Droids [ DROID_TYPE_BLUE ][ NORTH ] );
                W_LoadImage ( "Gfx/Droids/Blue/North_East.bmp", & g_Droids [ DROID_TYPE_BLUE ][ NORTH_EAST ] );
                W_LoadImage ( "Gfx/Droids/Blue/East.bmp", & g_Droids [ DROID_TYPE_BLUE ][ EAST ] );
                W_LoadImage ( "Gfx/Droids/Blue/South_East.bmp", & g_Droids [ DROID_TYPE_BLUE ][ SOUTH_EAST ] );
                W_LoadImage ( "Gfx/Droids/Blue/South.bmp", & g_Droids [ DROID_TYPE_BLUE ][ SOUTH ] );
                W_LoadImage ( "Gfx/Droids/Blue/South_West.bmp", & g_Droids [ DROID_TYPE_BLUE ][ SOUTH_WEST ] );
                W_LoadImage ( "Gfx/Droids/Blue/West.bmp", & g_Droids [ DROID_TYPE_BLUE ][ WEST ] );
                W_LoadImage ( "Gfx/Droids/Blue/North_West.bmp", & g_Droids [ DROID_TYPE_BLUE ][ NORTH_WEST ] );

                W_LoadImage ( "Gfx/Droids/Grey/North.bmp", & g_Droids [ DROID_TYPE_GREY ][ NORTH ] );
                W_LoadImage ( "Gfx/Droids/Grey/North_East.bmp", & g_Droids [ DROID_TYPE_GREY ][ NORTH_EAST ] );
                W_LoadImage ( "Gfx/Droids/Grey/East.bmp", & g_Droids [ DROID_TYPE_GREY ][ EAST ] );
                W_LoadImage ( "Gfx/Droids/Grey/South_East.bmp", & g_Droids [ DROID_TYPE_GREY ][ SOUTH_EAST ] );
                W_LoadImage ( "Gfx/Droids/Grey/South.bmp", & g_Droids [ DROID_TYPE_GREY ][ SOUTH ] );
                W_LoadImage ( "Gfx/Droids/Grey/South_West.bmp", & g_Droids [ DROID_TYPE_GREY ][ SOUTH_WEST ] );
                W_LoadImage ( "Gfx/Droids/Grey/West.bmp", & g_Droids [ DROID_TYPE_GREY ][ WEST ] );
                W_LoadImage ( "Gfx/Droids/Grey/North_West.bmp", & g_Droids [ DROID_TYPE_GREY ][ NORTH_WEST ] );

                W_LoadImage ( "Gfx/Droids/Red/North.bmp", & g_Droids [ DROID_TYPE_RED ][ NORTH ] );
                W_LoadImage ( "Gfx/Droids/Red/North_East.bmp", & g_Droids [ DROID_TYPE_RED ][ NORTH_EAST ] );
                W_LoadImage ( "Gfx/Droids/Red/East.bmp", & g_Droids [ DROID_TYPE_RED ][ EAST ] );
                W_LoadImage ( "Gfx/Droids/Red/South_East.bmp", & g_Droids [ DROID_TYPE_RED ][ SOUTH_EAST ] );
                W_LoadImage ( "Gfx/Droids/Red/South.bmp", & g_Droids [ DROID_TYPE_RED ][ SOUTH ] );
                W_LoadImage ( "Gfx/Droids/Red/South_West.bmp", & g_Droids [ DROID_TYPE_RED ][ SOUTH_WEST ] );
                W_LoadImage ( "Gfx/Droids/Red/West.bmp", & g_Droids [ DROID_TYPE_RED ][ WEST ] );
                W_LoadImage ( "Gfx/Droids/Red/North_West.bmp", & g_Droids [ DROID_TYPE_RED ][ NORTH_WEST ] );

                // Load the laser animations

                W_LoadImage ( "Gfx/Weapons/Player/North/0.bmp", & g_PlayerLaserAnims [ STRAIGHT_NORTH ][ 0 ] );
                W_LoadImage ( "Gfx/Weapons/Player/North/1.bmp", & g_PlayerLaserAnims [ STRAIGHT_NORTH ][ 1 ] );
                W_LoadImage ( "Gfx/Weapons/Player/North/2.bmp", & g_PlayerLaserAnims [ STRAIGHT_NORTH ][ 2 ] );
                W_LoadImage ( "Gfx/Weapons/Player/North/3.bmp", & g_PlayerLaserAnims [ STRAIGHT_NORTH ][ 3 ] );
                W_LoadImage ( "Gfx/Weapons/Player/South/0.bmp", & g_PlayerLaserAnims [ STRAIGHT_SOUTH ][ 0 ] );
                W_LoadImage ( "Gfx/Weapons/Player/South/1.bmp", & g_PlayerLaserAnims [ STRAIGHT_SOUTH ][ 1 ] );
                W_LoadImage ( "Gfx/Weapons/Player/South/2.bmp", & g_PlayerLaserAnims [ STRAIGHT_SOUTH ][ 2 ] );
                W_LoadImage ( "Gfx/Weapons/Player/South/3.bmp", & g_PlayerLaserAnims [ STRAIGHT_SOUTH ][ 3 ] );
                W_LoadImage ( "Gfx/Weapons/Player/East/0.bmp", & g_PlayerLaserAnims [ STRAIGHT_EAST ][ 0 ] );
                W_LoadImage ( "Gfx/Weapons/Player/East/1.bmp", & g_PlayerLaserAnims [ STRAIGHT_EAST ][ 1 ] );
                W_LoadImage ( "Gfx/Weapons/Player/East/2.bmp", & g_PlayerLaserAnims [ STRAIGHT_EAST ][ 2 ] );
                W_LoadImage ( "Gfx/Weapons/Player/East/3.bmp", & g_PlayerLaserAnims [ STRAIGHT_EAST ][ 3 ] );
                W_LoadImage ( "Gfx/Weapons/Player/West/0.bmp", & g_PlayerLaserAnims [ STRAIGHT_WEST ][ 0 ] );
                W_LoadImage ( "Gfx/Weapons/Player/West/1.bmp", & g_PlayerLaserAnims [ STRAIGHT_WEST ][ 1 ] );
                W_LoadImage ( "Gfx/Weapons/Player/West/2.bmp", & g_PlayerLaserAnims [ STRAIGHT_WEST ][ 2 ] );
                W_LoadImage ( "Gfx/Weapons/Player/West/3.bmp", & g_PlayerLaserAnims [ STRAIGHT_WEST ][ 3 ] );

                W_LoadImage ( "Gfx/Weapons/Enemy/North/0.bmp", & g_EnemyLaserAnims [ STRAIGHT_NORTH ][ 0 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/North/1.bmp", & g_EnemyLaserAnims [ STRAIGHT_NORTH ][ 1 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/North/2.bmp", & g_EnemyLaserAnims [ STRAIGHT_NORTH ][ 2 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/North/3.bmp", & g_EnemyLaserAnims [ STRAIGHT_NORTH ][ 3 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/South/0.bmp", & g_EnemyLaserAnims [ STRAIGHT_SOUTH ][ 0 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/South/1.bmp", & g_EnemyLaserAnims [ STRAIGHT_SOUTH ][ 1 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/South/2.bmp", & g_EnemyLaserAnims [ STRAIGHT_SOUTH ][ 2 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/South/3.bmp", & g_EnemyLaserAnims [ STRAIGHT_SOUTH ][ 3 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/East/0.bmp", & g_EnemyLaserAnims [ STRAIGHT_EAST ][ 0 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/East/1.bmp", & g_EnemyLaserAnims [ STRAIGHT_EAST ][ 1 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/East/2.bmp", & g_EnemyLaserAnims [ STRAIGHT_EAST ][ 2 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/East/3.bmp", & g_EnemyLaserAnims [ STRAIGHT_EAST ][ 3 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/West/0.bmp", & g_EnemyLaserAnims [ STRAIGHT_WEST ][ 0 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/West/1.bmp", & g_EnemyLaserAnims [ STRAIGHT_WEST ][ 1 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/West/2.bmp", & g_EnemyLaserAnims [ STRAIGHT_WEST ][ 2 ] );
                W_LoadImage ( "Gfx/Weapons/Enemy/West/3.bmp", & g_EnemyLaserAnims [ STRAIGHT_WEST ][ 3 ] );

                // Load the explosion animation

                W_LoadImage ( "Gfx/Explosion/0.bmp", & g_ExplosionAnim [ 0 ] );
                W_LoadImage ( "Gfx/Explosion/1.bmp", & g_ExplosionAnim [ 1 ] );
                W_LoadImage ( "Gfx/Explosion/2.bmp", & g_ExplosionAnim [ 2 ] );
                W_LoadImage ( "Gfx/Explosion/3.bmp", & g_ExplosionAnim [ 3 ] );
                W_LoadImage ( "Gfx/Explosion/4.bmp", & g_ExplosionAnim [ 4 ] );
                W_LoadImage ( "Gfx/Explosion/5.bmp", & g_ExplosionAnim [ 5 ] );
                W_LoadImage ( "Gfx/Explosion/6.bmp", & g_ExplosionAnim [ 6 ] );

                SetGameState ( GAME_STATE_PLAY );

                break;
            }

            // Gameplay

            case GAME_STATE_PLAY:
            {
                // Is the map active?

                static int iIsMapActive = FALSE;

                // Draw the next frame

                if ( iIsMapActive )
                {
                    // Is the cursor visible

                    static int iCursorVisible = TRUE;

                    // Draw the map

                    W_BlitImage ( g_MapScreen, 0, 0 );

                    // Draw the cursor

                    if ( iCursorVisible )
                        W_BlitImage ( g_MapCursor, 81 + ( 103 * g_Player.iRoomX ), 74 + ( g_Player.iRoomY * 77 ) );

                    // Check the blink timer and update the cursor's visibility

                    if ( W_GetTimerState ( g_MapCursorBlinkTimer ) )
                    {
                        if ( iCursorVisible )
                            iCursorVisible = FALSE;
                        else
                            iCursorVisible = TRUE;
                    }

                    // If enter is pressed again, return to the game

                    if ( W_GetKeyState ( W_KEY_ENTER ) )
                    {
                        W_DisableKeyDelay ();
                        iIsMapActive = FALSE;

                        unsigned int iTickCount = W_GetTickCount ();
                        while ( W_GetTickCount () < iTickCount + 600 );
                    }
                }
                else
                {
                    // Draw the game screen

                    DrawGameScreen ();

                    // Recharge the player's energy

                    if ( ( g_Player.Droid.fEnergy += PLAYER_ENERGY_RECHARGE ) > MAX_ENERGY )
                        g_Player.Droid.fEnergy = MAX_ENERGY;

                    // Run the scripts

                    XS_RunScripts ( SCRIPT_TIMESLICE_DUR );

                    // Determine if the player has activated all four key panels

                    if ( ! g_iIsGameOver )
                    {
                        int iArePanelsActivated = TRUE;
                        for ( int iCurrKeyPanel = 0; iCurrKeyPanel < KEY_COUNT; ++ iCurrKeyPanel )
                        {
                            if ( ! g_Player.iActiveKeyPanels [ iCurrKeyPanel ] )
                            {
                                iArePanelsActivated = FALSE;
                                break;
                            }
                        }

                        if ( iArePanelsActivated )
                        {
                            g_iIsGameOver = TRUE;
                            g_iGameOverTime = W_GetTickCount ();
                            W_PlaySound ( g_AllPanelsActivatedSound );
                        }
                    }
                    else
                    {
                        if ( W_GetTickCount () - g_iGameOverTime > CONGRATS_SCREEN_DELAY )
                            SetGameState ( GAME_STATE_CONGRATS );
                    }

                    // ---- Handle keyboard input, if the player is alive

                    if ( g_Player.Droid.iIsActive )
                    {
                        // Move the player based on its direction

                        // North east

                        if ( W_GetKeyState ( W_KEY_UP ) &&
                             W_GetKeyState ( W_KEY_RIGHT ) )
                        {
                            MoveDroid ( g_Player.Droid, NORTH_EAST, PLAYER_DROID_SPEED );
                        }
    
                        // North west

                        else if ( W_GetKeyState ( W_KEY_UP ) &&
                             W_GetKeyState ( W_KEY_LEFT ) )
                        {
                            MoveDroid ( g_Player.Droid, NORTH_WEST, PLAYER_DROID_SPEED );
                        }

                        // South east

                        else if ( W_GetKeyState ( W_KEY_DOWN ) &&
                             W_GetKeyState ( W_KEY_RIGHT ) )
                        {
                            MoveDroid ( g_Player.Droid, SOUTH_EAST, PLAYER_DROID_SPEED );
                        }

                        // South west

                        else if ( W_GetKeyState ( W_KEY_DOWN ) &&
                             W_GetKeyState ( W_KEY_LEFT ) )
                        {
                            MoveDroid ( g_Player.Droid, SOUTH_WEST, PLAYER_DROID_SPEED );
                        }

                        // North

                        else if ( W_GetKeyState ( W_KEY_UP ) )
                        {
                            MoveDroid ( g_Player.Droid, NORTH, PLAYER_DROID_SPEED );
                        }

                        // South

                        else if ( W_GetKeyState ( W_KEY_DOWN ) )
                        {
                            MoveDroid ( g_Player.Droid, SOUTH, PLAYER_DROID_SPEED );
                        }

                        // East

                        else if ( W_GetKeyState ( W_KEY_RIGHT ) )
                        {
                            MoveDroid ( g_Player.Droid, EAST, PLAYER_DROID_SPEED );
                        }

                        // West

                        else if ( W_GetKeyState ( W_KEY_LEFT ) )
                        {
                            MoveDroid ( g_Player.Droid, WEST, PLAYER_DROID_SPEED );
                        }

                        if ( W_GetKeyState ( W_KEY_SPACE ) )
                            FireDroidGun ( g_Player.Droid, LASER_TYPE_PLAYER );

                        // Display the map

                        if ( W_GetKeyState ( W_KEY_ENTER ) )
                        {
                            W_EnableKeyDelay ();
                            iIsMapActive = TRUE;
                        }

                        // Exit the game

		                if ( W_GetKeyState ( W_KEY_ESC ) )
			                SetGameState ( GAME_STATE_TITLE );
                    }
                    else
                    {
                        // If the player has no energy, exit the game after the explosions die
                        // down

                        int iExplosionsActive = FALSE;
                        for ( int iCurrExplosion = 0; iCurrExplosion < MAX_EXPLOSION_COUNT; ++ iCurrExplosion )
                        {
                            if ( g_Explosions [ iCurrExplosion ].iIsActive )
                            {
                                iExplosionsActive = TRUE;
                                break;
                            }
                        }
                        if ( ! iExplosionsActive )
                            SetGameState ( GAME_STATE_GAME_OVER );
                    }
                }
                break;
            }

            // Congrats screen

            case GAME_STATE_CONGRATS:
            {
                // Draw the congrats screen

                W_BlitImage ( g_CongratsScreen, 0, 0 );

                // If any key is pressed, move back to the title screen

				if ( W_GetAnyKeyState () )
					SetGameState ( GAME_STATE_TITLE );

                break;
            }

            // Game over screen

            case GAME_STATE_GAME_OVER:
            {
                // Draw the congrats screen

                W_BlitImage ( g_GameOverScreen, 0, 0 );

                // If any key is pressed, move back to the title screen

				if ( W_GetAnyKeyState () )
					SetGameState ( GAME_STATE_TITLE );

                break;
            }
        }

        while ( TRUE )
        {   
            iCurrTime = W_GetHighPerformanceTickCount ();

            if ( ( iCurrTime - iStartTime ) > FPS_LOCK_FRAME_DUR )
                break;
        }
    }

    /******************************************************************************************
    *
    *   Init ()
    *
    *   Initializes the application by loading the necessary resources and initializing globals
    *   and structures.
    */

    void Init ()
    {
        // ---- Initialize XtremeScript

        XS_Init ();

        // Register the host API

        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "GetRandInRange", HAPI_GetRandInRange );

        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "ToggleRoomLights", HAPI_ToggleRoomLights );
        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "MoveEnemyDroid", HAPI_MoveEnemyDroid );
        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "GetEnemyDroidX", HAPI_GetEnemyDroidX );
        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "GetEnemyDroidY", HAPI_GetEnemyDroidY );
        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "IsEnemyDroidAlive", HAPI_IsEnemyDroidAlive );
        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "FireEnemyDroidGun", HAPI_FireEnemyDroidGun );

        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "GetPlayerDroidX", HAPI_GetPlayerDroidX );
        XS_RegisterHostAPIFunc ( XS_GLOBAL_FUNC, "GetPlayerDroidY", HAPI_GetPlayerDroidY );

        // Load the scripts

        XS_LoadScript ( "Scripts/Ambient.xse", g_iAmbientThreadIndex, XS_THREAD_PRIORITY_USER );
        XS_LoadScript ( "Scripts/Blue_Droid.xse", g_iBlueDroidThreadIndex, XS_THREAD_PRIORITY_USER );
        XS_LoadScript ( "Scripts/Grey_Droid.xse", g_iGreyDroidThreadIndex, XS_THREAD_PRIORITY_USER );
        XS_LoadScript ( "Scripts/Red_Droid.xse", g_iRedDroidThreadIndex, XS_THREAD_PRIORITY_USER );

        // ---- Load Sounds

        W_LoadSound ( "Sound/Intro/Intro.wav", & g_IntroMusic, FALSE );

        W_LoadSound ( "Sound/Interface/Browse.wav", & g_BrowseSound, FALSE );
        W_LoadSound ( "Sound/Interface/Select.wav", & g_SelectSound, FALSE );

        W_LoadSound ( "Sound/Rooms/Door_Open.wav", & g_DoorOpenSound, FALSE );
        W_LoadSound ( "Sound/Rooms/Lights_Flicker.wav", & g_LightsFlickerSound, FALSE );

        W_LoadSound ( "Sound/Keys/Get_Key.wav", & g_GetKeySound, FALSE );
        W_LoadSound ( "Sound/Keys/Panel_Activate.wav", & g_PanelActivateSound, FALSE );
        W_LoadSound ( "Sound/Keys/All_Panels_Activated.wav", & g_AllPanelsActivatedSound, FALSE );

        W_LoadSound ( "Sound/Weapons/Player_Laser.wav", & g_PlayerLaserSound, FALSE );
        W_LoadSound ( "Sound/Weapons/Enemy_Laser.wav", & g_EnemyLaserSound, FALSE );
        W_LoadSound ( "Sound/Weapons/Explosion.wav", & g_ExplosionSound, FALSE );

        W_LoadSound ( "Sound/Game_Over/Theme.wav", & g_GameOverSound, FALSE );

        // Set the game state to the title screen

        SetGameState ( GAME_STATE_TITLE );
    }

    /******************************************************************************************
    *
    *   ShutDown ()
    *
    *   Shuts down the game by freeing resources.
    */
    
    void ShutDown ()
    {
        // ---- Unload the sounds

        W_FreeSound ( & g_IntroMusic );

        W_FreeSound ( & g_BrowseSound );
        W_FreeSound ( & g_SelectSound );

        W_FreeSound ( & g_DoorOpenSound );
        W_FreeSound ( & g_LightsFlickerSound );

        W_FreeSound ( & g_GetKeySound );
        W_FreeSound ( & g_PanelActivateSound );

        W_FreeSound ( & g_PlayerLaserSound );
        W_FreeSound ( & g_EnemyLaserSound );
        W_FreeSound ( & g_ExplosionSound );

        W_FreeSound ( & g_AllPanelsActivatedSound );

        W_FreeSound ( & g_GameOverSound );

        // ---- Shut down the scripting system

        XS_ShutDown ();
    }

    /******************************************************************************************
    *
    *   InitGame ()
    *
    *   Initializes the game logic.
    */

    void InitGame ()
    {
        // Seed the random number generator

        srand ( time ( NULL ) );

        // ---- Initialize the main game flags

        g_iIsGameOver = FALSE;
        g_iGameOverTime = 0;

        // ---- Initialize the player

        // Initialize the player droid

        InitDroid ( g_Player.Droid, 319, 239, NORTH, DROID_TYPE_WHITE, MAX_ENERGY );

        // Set the room

        g_Player.iRoomX = 2;
        g_Player.iRoomY = 4;

        // Set the keys collected to zero

        for ( int iCurrKey = 0; iCurrKey < KEY_COUNT; ++ iCurrKey )
            g_Player.iKeys [ iCurrKey ] = FALSE;

        // Set the activated key panels to zero

        for ( int iCurrKeyPanel = 0; iCurrKeyPanel < KEY_COUNT; ++ iCurrKeyPanel )
            g_Player.iActiveKeyPanels [ iCurrKeyPanel ] = FALSE;
        
        // ---- Clear all the major structures

        // Droids

        for ( int iCurrDroid = 0; iCurrDroid < ENEMY_DROID_COUNT; ++ iCurrDroid )
            g_EnemyDroids [ iCurrDroid ].iIsActive = FALSE;

        // Explosions

        for ( int iCurrExplosion = 0; iCurrExplosion < MAX_EXPLOSION_COUNT; ++ iCurrExplosion )
            g_Explosions [ iCurrExplosion ].iIsActive = FALSE;

        // Lasers

        for ( int iCurrLaser = 0; iCurrLaser < MAX_LASER_COUNT; ++ iCurrLaser )
            g_Lasers [ iCurrLaser ].iIsActive = FALSE;

        // ---- Initialize the interface

        // Initialize the map cursor blink timer

        g_MapCursorBlinkTimer = W_InitTimer ( MAP_CURSOR_BLINK_RATE );

        // ---- Initialize the room

        InitRoom ( g_iRooms [ g_Player.iRoomX ][ g_Player.iRoomY ] );
    }

    /******************************************************************************************
    *
    *   HAPI_GetRandInRange ()
    *
    *       int GetRandInRange ( int Min, int Max )
    *
    *   Returns a random integer within the specified range.
    */

    void HAPI_GetRandInRange ( int iThreadIndex )
    {
        int iMin,
            iMax;
        
        // Read the min and max parameters in, backwards

        iMin = XS_GetParamAsInt ( iThreadIndex, 1 );
        iMax = XS_GetParamAsInt ( iThreadIndex, 0 );

        // Return a random number

        XS_ReturnInt ( iThreadIndex, 2, W_GetRandInRange ( iMin, iMax ) );
    }

    /******************************************************************************************
    *
    *   HAPI_ToggleRoomLights ()
    *
    *       void ToggleRoomLights ()
    *
    *   Toggles the room lights.
    */

    void HAPI_ToggleRoomLights ( int iThreadIndex )
    {
        // If the lights are on, turn them off; otherwise, turn them on

        if ( g_CurrRoom.iLights == LIGHTS_ON )
            g_CurrRoom.iLights = LIGHTS_OFF;
        else
        {
            g_CurrRoom.iLights = LIGHTS_ON;
            W_PlaySound ( g_LightsFlickerSound );
        }

        // Return nothing to XtremeScript

        XS_Return ( iThreadIndex, 0 );
    }

    /******************************************************************************************
    *
    *   HAPI_MoveEnemyDroid ()
    *
    *       void MoveEnemyDroid ( int DroidIndex, int Dir, int Dist )
    *
    *   Moves an enemy droid.
    */

    void HAPI_MoveEnemyDroid ( int iThreadIndex )
    {
        int iDroidIndex;
        int iDir;
        int iDist;
        
        // Read the droid index

        iDroidIndex = XS_GetParamAsInt ( iThreadIndex, 2 );

        // Read in the direction

        iDir = XS_GetParamAsInt ( iThreadIndex, 1 );

        // Read in the distance

        iDist = XS_GetParamAsInt ( iThreadIndex, 0 );

        // Move the droid
        
        MoveDroid ( g_EnemyDroids [ iDroidIndex ], iDir, iDist );

        XS_Return ( iThreadIndex, 3 );
    }

    /******************************************************************************************
    *
    *   HAPI_GetEnemyDroidX ()
    *
    *       int GetEnemyDroidX ( int DroidIndex )
    *
    *   Returns the X coordinate of an enemy droid.
    */

    void HAPI_GetEnemyDroidX ( int iThreadIndex )
    {
        int iDroidIndex;

        // Read the droid index

        iDroidIndex = XS_GetParamAsInt ( iThreadIndex, 0 );

        // Get the droid center

        W_Point DroidCenter;
        GetDroidCenter ( g_EnemyDroids [ iDroidIndex ], DroidCenter );

        // Return the droid center

        XS_ReturnInt ( iThreadIndex, 1, DroidCenter.iX );
    }

    /******************************************************************************************
    *
    *   HAPI_GetEnemyDroidY ()
    *
    *       int GetEnemyDroidY ( int DroidIndex )
    *
    *   Returns the Y coordinate of an enemy droid.
    */

    void HAPI_GetEnemyDroidY ( int iThreadIndex )
    {
        int iDroidIndex;

        // Read the droid index

        iDroidIndex = XS_GetParamAsInt ( iThreadIndex, 0 );

        // Get the droid center

        W_Point DroidCenter;
        GetDroidCenter ( g_EnemyDroids [ iDroidIndex ], DroidCenter );

        // Return the droid center

        XS_ReturnInt ( iThreadIndex, 1, DroidCenter.iY );
    }

    /******************************************************************************************
    *
    *   HAPI_IsEnemyDroidAlive ()
    *
    *       int IsEnemyDroidAlive ( int DroidIndex )
    *
    *   Determines whether or not an enemy droid is alive.
    */

    void HAPI_IsEnemyDroidAlive ( int iThreadIndex )
    {
        int iDroidIndex;

        // Read the droid index

        iDroidIndex = XS_GetParamAsInt ( iThreadIndex, 0 );

        // Return the droid's activity status

        XS_ReturnInt ( iThreadIndex, 1,  g_EnemyDroids [ iDroidIndex ].iIsActive );
    }

    /******************************************************************************************
    *
    *   HAPI_FireEnemyDroidGun ()
    *
    *       void FireEnemeyDroidGun ( int DroidIndex )
    *
    *   Fires an enemy droid's gun
    */

    void HAPI_FireEnemyDroidGun ( int iThreadIndex )
    {
        int iDroidIndex;

        // Read the droid index

        iDroidIndex = XS_GetParamAsInt ( iThreadIndex, 0 );

        // Fire the gun

        FireDroidGun ( g_EnemyDroids [ iDroidIndex ], LASER_TYPE_ENEMY );

        // Return nothing

        XS_Return ( iThreadIndex, 1 );
    }

    /******************************************************************************************
    *
    *   HAPI_GetPlayerDroidX ()
    *
    *       int GetPlayerDroidX ()
    *
    *   Returns the X coordinate of the player droid
    */

    void HAPI_GetPlayerDroidX ( int iThreadIndex )
    {
        // Get the droid center

        W_Point DroidCenter;
        GetDroidCenter ( g_Player.Droid, DroidCenter );

        // Return the droid center

        XS_ReturnInt ( iThreadIndex, 0, DroidCenter.iX );
    }

    /******************************************************************************************
    *
    *   HAPI_GetPlayerDroidY ()
    *
    *       int GetPlayerDroidY ()
    *
    *   Returns the Y coordinate of the player droid
    */

    void HAPI_GetPlayerDroidY ( int iThreadIndex )
    {
        // Get the droid center

        W_Point DroidCenter;
        GetDroidCenter ( g_Player.Droid, DroidCenter );

        // Return the droid center

        XS_ReturnInt ( iThreadIndex, 0, DroidCenter.iY );
    }

// ---- Main ----------------------------------------------------------------------------------

	Main
	{
		// Initialize Wrappuh

		if ( ! W_InitWrappuh ( "Example 06-02", hInstance, iCmdShow ) )
			W_ExitOnError ( "Could not initialize Wrappuh." );

		// Set the video mode

		if ( ! W_SetVideoMode ( 640, 480, 32 ) )
            if ( ! W_SetVideoMode ( 640, 480, 16 ) )
			    W_ExitOnError ( "Could not set video mode." );

        // Initialize the game

        Init ();

		// Start the main loop

		MainLoop
		{
			// Start the current loop iteration

			HandleLoop
			{
                // Handle the current state

                HandleState ();

                // Blit the next frame

                W_BlitFrame ();
			}
		}

        // Shut down the game

        ShutDown ();

		// Shut down Wrappuh

		W_ShutDownWrappuh ();

		// Exit the app

		ExitApp;
    }