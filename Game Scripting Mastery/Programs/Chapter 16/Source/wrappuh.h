/*

	Project.

		Wrappuh

	Abstract.

		Main header file

	Date Created.

		2.6.2002

	Author.

		Alex Varanese

*/

#ifndef WRAPPUH
#define WRAPPUH

// ---- Includes ------------------------------------------------------------------------------

	// ---- Language --------------------------------------------------------------------------

        #include <direct.h>
        #include <stdlib.h>
		#include <iostream.h>
		#include <conio.h>
		#include <stdlib.h>
		#include <malloc.h>
		#include <memory.h>
		#include <string.h>
		#include <stdarg.h>
		#include <stdio.h>
		#include <math.h>
		#include <io.h>
		#include <fcntl.h>
		#include <time.h>
        #include <limits.h>

	// ---- Win32 -----------------------------------------------------------------------------

		#define WIN32_LEAN_AND_MEAN

		#include <windows.h>
		#include <windowsx.h>
        #include <objbase.h>
		#include <mmsystem.h>

	// ---- DirectX ---------------------------------------------------------------------------

		#include <ddraw.h>
		#include <dinput.h>
		#include <dmusici.h>

	// ---- Wrappuh ---------------------------------------------------------------------------

		#include "keymap.h"

// ---- Constants -----------------------------------------------------------------------------

    #define PI                                  3.1415926535
    #define MAX_RADIAN                          ( 2 * PI )

    #define SOUND_CHANNEL_COUNT                 2

    #define DM_NUM_SEGMENTS                     64

    #define MIDI_NULL                           0
    #define MIDI_LOADED                         1
    #define MIDI_PLAYING                        2
    #define MIDI_STOPPED                        3


    #define MAX_SOUNDS                          256

    #define SOUND_NULL                          0
    #define SOUND_LOADED                        1
    #define SOUND_PLAYING                       2
    #define SOUND_STOPPED                       3

    #ifndef DSBCAPS_CTRLDEFAULT
    #define DSBCAPS_CTRLDEFAULT ( DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME )
    #endif

// ---- Data Types ----------------------------------------------------------------------------

	// ---- Video -----------------------------------------------------------------------------

        typedef struct
        {
            int iX0,
                iX1,
                iY0,
                iY1;
        }
            W_Rect;

        typedef struct
        {
            int iX,
                iY;
        }
            W_Point;

		typedef struct
		{
			LPDIRECTDRAWSURFACE4 pDDSrfc;
			int iXRes,
				iYRes;
			int iXMax,
				iYMax;
            W_Rect ClipRect;
			int iPitch;
		}
			W_Image;

	// ---- Audio -----------------------------------------------------------------------------

		typedef struct
		{
            int iChannelIndex;
            int iChannels [ SOUND_CHANNEL_COUNT ];
		}
			W_Sound;

        typedef struct pcm_sound_typ
	    {
	        LPDIRECTSOUNDBUFFER dsbuffer;
	        int state;
	        int rate;
	        int size;
	        int id;
	    }
            pcm_sound, *pcm_sound_ptr;

        typedef struct DMUSIC_MIDI_TYP
        {
            IDirectMusicSegment * dm_segment;
            IDirectMusicSegmentState * dm_segstate;
            int id;
            int state;
        }
            DMUSIC_MIDI, *DMUSIC_MIDI_PTR;

	// ---- Timers ----------------------------------------------------------------------------

		typedef int W_TimerHandle;
        typedef INT64 W_Int64;

// ---- Macros --------------------------------------------------------------------------------

	// ---- Win32 Abstraction -----------------------------------------------------------------

		#define Main																							\
																												\
			int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow )

		#define MainLoop	\
							\
			MSG CurrMssg;	\
			while ( TRUE )

		#define HandleLoop								\
														\
			W_GetKbrdState ();							\
			W_HandleTimers ();							\
			CurrMssg = W_HandleWin32MssgLoop ();		\
			if ( CurrMssg.message == WM_QUIT )			\
				break;									\
			if ( ! g_bAppExit )

		#define ExitApp				\
									\
			return CurrMssg.wParam;

    // ---- Audio -----------------------------------------------------------------------------

        #define DSVOLUME_TO_DB(volume) ((DWORD)(-30*(100 - volume)))
        #define MULTI_TO_WIDE( x,y )  MultiByteToWideChar( CP_ACP,MB_PRECOMPOSED, y,-1,x,_MAX_PATH);
        #define DD_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// ---- Public Interface ----------------------------------------------------------------------

	// ---- Misc ------------------------------------------------------------------------------

		void W_ExitOnError ( char * pstrErrorMssg );
		void W_Exit ();

	// ---- Initialization --------------------------------------------------------------------

		bool W_InitWrappuh ( char * pstrAppName, HINSTANCE hInstance, int iCmdShow );
		void W_ShutDownWrappuh ();

		LRESULT CALLBACK W_MainWindowHndlr ( HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam );
		MSG W_HandleWin32MssgLoop ();

	// ---- Video -----------------------------------------------------------------------------

		bool W_SetVideoMode ( int iXRes, int iYRes, int iColorDepth );

		int W_GetScreenXRes ();
		int W_GetScreenYRes ();
		int W_GetScreenXMax ();
		int W_GetScreenYMax ();

		bool W_LockFrame ();
		bool W_BlitFrame ();
		bool W_ClearFrame ();

		bool W_LoadImage ( char * pstrBMPFilename, W_Image * Image );
		void W_FreeImage ( W_Image * Image );

		bool W_BlitImage ( W_Image Image, int iX, int iY );

		void W_DrawPoint ( UCHAR iR, UCHAR iG, UCHAR iB, int iX, int iY );

		bool W_LoadFont ( char * pstrBMPFilename, int iCellXRes, int iCellYRes );
		void W_FreeFont ();

		bool W_DrawTextString ( char * pstrTextString, int iX, int iY );
		int W_GetStringPixelLength ( char * pstrTextString );

        W_Rect W_OffsetRect ( W_Rect Rect, int iX, int iY );
        bool W_DoRectsIntersect ( W_Rect * Rect0, W_Rect * Rect1 );

        int W_GetRandInRange ( int iMin, int iMax );

	// ---- Input -----------------------------------------------------------------------------

		void W_GetKbrdState ();
		int W_GetKeyState ( int iScanCode );
		int W_GetAnyKeyState ();

        void W_EnableKeyDelay ();
        void W_DisableKeyDelay ();

	// ---- Audio -----------------------------------------------------------------------------

		bool W_LoadSound ( char * pstrWAVFilename, W_Sound * Sound, bool bLoop );
		void W_FreeSound ( W_Sound * Sound );

		bool W_PlaySound ( W_Sound & Sound );
		bool W_StopSound ( W_Sound Sound );
        void W_StopAllSounds ();

        int DSound_Load_WAV(char *filename, int control_flags = DSBCAPS_CTRLDEFAULT);
        int DSound_Replicate_Sound(int source_id);
        int DSound_Play(int id, int flags=0, int volume=0, int rate=0, int pan=0);
        int DSound_Stop_Sound(int id);
        int DSound_Stop_All_Sounds(void);
        int DSound_Init(void);
        int DSound_Shutdown(void);
        int DSound_Delete_Sound(int id);
        int DSound_Delete_All_Sounds(void);
        int DSound_Status_Sound(int id);
        int DSound_Set_Volume(int id,int vol);
        int DSound_Set_Freq(int id,int freq);
        int DSound_Set_Pan(int id,int pan);

        int DMusic_Load_MIDI(char *filename);
        int DMusic_Play(int id);
        int DMusic_Stop(int id);
        int DMusic_Shutdown(void);
        int DMusic_Delete_MIDI(int id);
        int DMusic_Delete_All_MIDI(void);
        int DMusic_Status_MIDI(int id);
        int DMusic_Init(void);

	// ---- Timer -----------------------------------------------------------------------------

		DWORD W_GetTickCount ();

		W_TimerHandle W_InitTimer ( int iLength );
		void W_ClearTimer ( W_TimerHandle hTimer );
		void W_HandleTimers ();
		bool W_GetTimerState ( W_TimerHandle hTimer );
        void W_Delay ( unsigned int iLength );

        W_Int64 W_GetHighPerformanceTickCount ();

#endif

// ---- Global Variables ----------------------------------------------------------------------

	// ---- Win32 -----------------------------------------------------------------------------

		extern bool g_bAppExit;
