/*

	Project.

		Wrappuh
	
	Abstract.

		Main source file

	Date Created.

		2.6.2002

	Author.

		Alex Varanese

*/

// ---- Includes ------------------------------------------------------------------------------

	#include "wrappuh.h"

// ---- Constants -----------------------------------------------------------------------------

	// ---- Video -----------------------------------------------------------------------------

		#define DEF_IMAGE_MASK_COLOR_15		EncodePixel15 ( 31, 0, 31 )
		#define DEF_IMAGE_MASK_COLOR_16		EncodePixel16 ( 31, 0, 31 )
		#define DEF_IMAGE_MASK_COLOR_32		EncodePixel32 ( 255, 0, 255 )

		#define DEF_FONT_CHAR_COUNT			93

		#define DEF_SPACE_PRCNT				.5
		#define DEF_KERN					1

	// ---- Input -----------------------------------------------------------------------------

		#define KEY_DELAY					135

	// ---- Timers ----------------------------------------------------------------------------

		#define MAX_TIMER_COUNT				256

// ---- Data Structures -----------------------------------------------------------------------

	// ---- Video -----------------------------------------------------------------------------

		typedef WORD Pixel15;
		typedef WORD Pixel16;
		typedef DWORD Pixel32;

		typedef struct
		{
			UCHAR R;
			UCHAR G;
			UCHAR B;
			UCHAR Alpha;
		}
			RGBTriad;

		typedef struct
		{
			int iXRes,
				iYRes;
			int iXMax,
				iYMax;
			int iColorDepth;
			int iPitch;
		}
			VideoContext;

		typedef struct
		{
			int iCellXRes,
				iCellYRes;
			int iCellFullXRes,
				iCellFullYRes;
			int iCellPitch;

			int iSpaceXRes;
			
			int iCharRowSize;
			int iCharRowRes;
			int iCharRowMaxX;
		}
			FontDesc;

		typedef struct
		{
			int iLeftKern,
				iRightKern;
			int iXRes;

			int iX,
				iY;
		}
			FontCharDesc;

	// ---- Timers ----------------------------------------------------------------------------

		typedef struct
		{
			bool bIsNull;

			int iLength;
			int iActiveTime;
			bool bIsActive;
		}
			Timer;
	
// ---- Global Variables ----------------------------------------------------------------------

	// ---- Win32 -----------------------------------------------------------------------------

		HWND g_hMainWindow;
		HINSTANCE g_hInstance;

		extern bool g_bAppExit				= FALSE;

	// ---- DirectX ---------------------------------------------------------------------------

		// ---- Video -------------------------------------------------------------------------
		
			LPDIRECTDRAW4 g_pDDIntrfc4		= NULL;

			LPDIRECTDRAWSURFACE4 g_pPrimDDSrfc = NULL;
			LPDIRECTDRAWSURFACE4 g_pBackDDSrfc = NULL;
			DDSURFACEDESC2 g_DDSrfcDesc;

			Pixel15 * g_pFrameBuffer15		= NULL;
			Pixel16 * g_pFrameBuffer16		= NULL;
			Pixel32 * g_pFrameBuffer32		= NULL;
			int g_iFrameBufferSize;

			DDBLTFX g_DDBlitFX;

			VideoContext g_VideoContext;

			W_Image g_FontImage;
			FontDesc g_FontDesc;
			FontCharDesc g_FontCharDesc [ DEF_FONT_CHAR_COUNT ];

		// ---- Input -------------------------------------------------------------------------

			IDirectInput8 * g_pDIIntrfc		= NULL;

			LPDIRECTINPUTDEVICE8 g_pDIKbrd	= NULL;
			BYTE g_KbrdState [ 256 ];
			unsigned int g_KbrdDelay [ 256 ];
			bool g_KbrdFrameState [ 256 ];

            int g_iKeyDelayActive           = TRUE;

		// ---- Audio -------------------------------------------------------------------------

			IDirectMusicLoader8 * g_pDMLoaderIntrfc = NULL;
			IDirectMusicPerformance8 * g_pDMPerformanceIntrfc = NULL;

            LPDIRECTSOUND lpds = NULL;
            DSBUFFERDESC dsbd;
            DSCAPS dscaps;
            HRESULT dsresult;
            DSBCAPS	dsbcaps;
            LPDIRECTSOUNDBUFFER	lpdsbprimary = NULL;
            pcm_sound sound_fx[MAX_SOUNDS];

            WAVEFORMATEX pcmwf;

            IDirectMusicPerformance * dm_perf = NULL;
            IDirectMusicLoader * dm_loader = NULL;

            DMUSIC_MIDI dm_midi[DM_NUM_SEGMENTS];
            int dm_active_id = -1;

		// ---- Timers ------------------------------------------------------------------------

            INT64 g_iTimerFreqPerMs;
            Timer g_Timers [ MAX_TIMER_COUNT ];

        // ---- Misc --------------------------------------------------------------------------

            FILE * g_pErrorFile = NULL;

// ---- Macros --------------------------------------------------------------------------------

	// ---- Misc ------------------------------------------------------------------------------

		#define GetNextMul4( X )						\
		{												\
			X = X % 4 == 0 ? X : X + ( 4 - ( X % 4 ) );	\
		}

        #define sfprintf( String )                      \
        {                                               \
            if ( g_pErrorFile )                         \
                fprintf ( g_pErrorFile, String );       \
        }

	// ---- Win32 -----------------------------------------------------------------------------

		#define InitWin32Struct( Win32Struct )						\
		{															\
			memset ( & Win32Struct, 0, sizeof ( Win32Struct ) );	\
			Win32Struct.dwSize = sizeof ( Win32Struct );			\
		}

	// ---- Video -----------------------------------------------------------------------------

		#define EncodePixel15( iR, iG, iB )									\
																			\
			( ( iB & 31 ) | ( ( iG & 31 ) << 5 ) | ( ( iR & 31 ) << 10 ) )

		#define EncodePixel16( iR, iG, iB )									\
																			\
			( ( iB & 31 ) | ( ( iG & 63 ) << 6 ) | ( ( iR & 31 ) << 11 ) )

		#define EncodePixel32( iR, iG, iB )		\
												\
			( iB | ( iG << 8 ) | ( iR << 16 ) )

// ---- Public Interface ----------------------------------------------------------------------

	// ---- Misc ------------------------------------------------------------------------------

		/**************************************************************************************
		*
		*	W_ExitOnError ()
		*
		*	Terminates the program and displays an error message in a dialog box.
		*/
		
		void W_ExitOnError ( char * pstrErrorMssg )
		{
			MessageBox ( g_hMainWindow, pstrErrorMssg, "Fatal Error", MB_OK );
			W_Exit ();
		}

		/*************************************************************************************
		*
		*	W_Exit ()
		*
		*	Terminates the program.
		*/

		void W_Exit ()
		{
			SendMessage ( g_hMainWindow, WM_CLOSE, 16, 0 );
			g_bAppExit = TRUE;
		}

	// ---- Initialization --------------------------------------------------------------------

		/**************************************************************************************
		*
		*	W_InitWrappuh ()
		*
		*	Initializes Wrappuh.
		*/

		bool W_InitWrappuh ( char * pstrAppName, HINSTANCE hInstance, int iCmdShow )
		{
            g_pErrorFile = fopen ( "log.txt", "w" );

            sfprintf ( "Wrappuh Logfile\n\n" );

            sfprintf ( "---- Initializing ----------------------------------------------------------------\n\n" );

			// ---- Win32
            
            sfprintf ( " - Initializing Win32...\n" );

			WNDCLASSEX MainWindow;

			MainWindow.cbSize = sizeof ( WNDCLASSEX );
			MainWindow.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
			MainWindow.cbClsExtra = 0;
			MainWindow.cbWndExtra = 0;
			MainWindow.lpfnWndProc = W_MainWindowHndlr;
			MainWindow.hInstance = hInstance;
			MainWindow.hbrBackground = ( HBRUSH ) GetStockObject ( BLACK_BRUSH );
			MainWindow.hIcon = LoadIcon ( NULL, IDI_APPLICATION );
			MainWindow.hIconSm = LoadIcon ( NULL, IDI_APPLICATION );
			MainWindow.hCursor = LoadCursor ( NULL, IDC_ARROW );
			MainWindow.lpszMenuName = NULL;
			MainWindow.lpszClassName = pstrAppName;

			RegisterClassEx ( & MainWindow );

			if ( ! ( g_hMainWindow = CreateWindowEx ( NULL,
													  pstrAppName,
													  pstrAppName,
													  WS_POPUP | WS_VISIBLE,
													  0,
													  0,
													  512,
													  512,
													  NULL,
													  NULL,
													  hInstance,
													  NULL ) ) )
				return FALSE;

				g_hInstance = hInstance;

			// ---- DirectX
            
                sfprintf ( " - Initializing DirectDraw...\n" );

				// ---- DirectDraw

				LPDIRECTDRAW pDDIntrfc1	= NULL;

                sfprintf ( "    - Creating the interface...\n" );

				if ( FAILED ( DirectDrawCreate ( NULL, & pDDIntrfc1, NULL ) ) )
					return FALSE;

                sfprintf ( "    - Querying the interface...\n" );

				if ( FAILED ( pDDIntrfc1->QueryInterface ( IID_IDirectDraw4, ( LPVOID * ) & g_pDDIntrfc4 ) ) )
					return FALSE;

				pDDIntrfc1->Release ();
				pDDIntrfc1 = NULL;

                sfprintf ( "    - Setting the cooperative level\n" );

				if ( FAILED ( g_pDDIntrfc4->SetCooperativeLevel ( g_hMainWindow, DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN ) ) )
					return FALSE;

				// ---- DirectInput

                sfprintf ( " - Initiailizing DirectInput...\n" );

				if ( FAILED ( DirectInput8Create ( g_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, ( void ** ) & g_pDIIntrfc, NULL ) ) )			
					return FALSE;

				// ---- Initialize keyboard

                sfprintf ( "    - Initializing the keyboard...\n" );

				if ( FAILED ( g_pDIIntrfc->CreateDevice ( GUID_SysKeyboard, & g_pDIKbrd, NULL ) ) )
					return FALSE;

                sfprintf ( "        - Setting the data format...\n" );

				if ( FAILED ( g_pDIKbrd->SetDataFormat ( & c_dfDIKeyboard ) ) )
					return FALSE;

                sfprintf ( "        - Setting the cooperative level...\n" );

				if ( FAILED ( g_pDIKbrd->SetCooperativeLevel ( g_hMainWindow, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND ) ) )
					return FALSE;

                sfprintf ( "        - Acquiring the device...\n" );

				if ( FAILED ( g_pDIKbrd->Acquire () ) )
					return FALSE;

				// ---- COM

                sfprintf ( " - Initiaizing COM...\n" );

				if ( FAILED ( CoInitialize ( NULL ) ) )
					return FALSE;

				// ---- DirectSound

                sfprintf ( " - Initializing DirectSound...\n" );

                if ( ! DSound_Init () )
                    return FALSE;

				// ---- Timers
    
                sfprintf ( " - Querying the high-performance timer...\n" );

                QueryPerformanceFrequency ( ( LARGE_INTEGER * ) & g_iTimerFreqPerMs );
                g_iTimerFreqPerMs /= 1000;

				for ( int iCurrTimerIndex = 0; iCurrTimerIndex < MAX_TIMER_COUNT; ++ iCurrTimerIndex )
					g_Timers [ iCurrTimerIndex ].bIsNull = TRUE;

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_ShutDownWrappuh ()
		*
		*	Shuts down Wrappuh.
		*/

		void W_ShutDownWrappuh ()
		{
            sfprintf ( "\n---- Shutting Down ---------------------------------------------------------------\n\n" );

			// ---- DirectX
		
				// ---- DirectDraw

                sfprintf ( " - Shutting down DirectDraw...\n" );

                sfprintf ( "    - Freeing the back buffer...\n" );
				
				if ( g_pBackDDSrfc )
				{
					g_pBackDDSrfc->Release ();
					g_pBackDDSrfc = NULL;
				}

                sfprintf ( "    - Freeing the back buffer...\n" );

				if ( g_pPrimDDSrfc )
				{
					g_pPrimDDSrfc->Release ();
					g_pPrimDDSrfc = NULL;
				}

                sfprintf ( "    - Freeing the interface...\n" );

				if ( g_pDDIntrfc4 )
				{
					g_pDDIntrfc4->Release ();
					g_pDDIntrfc4 = NULL;
				}

				ShowCursor ( TRUE );

				// ---- DirectInput

                sfprintf ( " - Shutting down DirectInput...\n" );

                sfprintf ( "    - Unacquiring the keyboard..\n" );

				if ( g_pDIKbrd )
				{
					g_pDIKbrd->Unacquire ();
					g_pDIKbrd->Release ();
					g_pDIKbrd = NULL;
				}

                sfprintf ( "    - Freeing the interface...\n" );

				if ( g_pDIIntrfc )
				{
					g_pDIIntrfc->Release ();
					g_pDIIntrfc = NULL;
				}

				// ---- DirectSound

                sfprintf ( " - Shutting down DirectSound...\n" );

                DSound_Shutdown ();

				// ---- COM

                sfprintf ( " - Uninitializing COM...\n" );

				CoUninitialize ();

                if ( g_pErrorFile )
                    fclose ( g_pErrorFile );
		}

		/**************************************************************************************
		*
		*	W_MainWindowHndlr ()
		*
		*	Basic, bare-bones handler for incoming Windows messages.
		*/

		LRESULT CALLBACK W_MainWindowHndlr ( HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam )
		{
			switch ( uMessage )
			{
				case WM_DESTROY:
				{
					PostQuitMessage ( 0 );
					return 0;
				}
				break;

				default:
				{
					return DefWindowProc ( hWindow, uMessage, wParam, lParam );
				}
			}

			return 0;
		}


		/**************************************************************************************
		*
		*	W_HandleWin32MssgLoop ()
		*
		*	Handles the Win32 message loop.
		*/

		MSG W_HandleWin32MssgLoop ()
		{
			MSG CurrMssg;

			if ( PeekMessage ( & CurrMssg, NULL, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage ( & CurrMssg );
				DispatchMessage ( & CurrMssg );
			}

			return CurrMssg;
		}

	// ---- Video -----------------------------------------------------------------------------

		/**************************************************************************************
		*
		*	W_SetVideoMode ()
		*
		*	Sets the video mode.
		*/

		bool W_SetVideoMode ( int iXRes, int iYRes, int iColorDepth )
		{
			if ( FAILED ( g_pDDIntrfc4->SetDisplayMode ( iXRes, iYRes, iColorDepth, 0, 0 ) ) )
				return FALSE;

			if ( g_pPrimDDSrfc )
			{
				g_pPrimDDSrfc->Release ();
				g_pPrimDDSrfc = NULL;
			}

			if ( g_pBackDDSrfc )
			{
				g_pBackDDSrfc->Release ();
				g_pBackDDSrfc = NULL;
			}

			InitWin32Struct ( g_DDSrfcDesc );
			g_DDSrfcDesc.dwSize = sizeof ( DDSURFACEDESC2 );
			g_DDSrfcDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
			g_DDSrfcDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_SYSTEMMEMORY;
			g_DDSrfcDesc.dwBackBufferCount = 1;
			if ( FAILED ( g_pDDIntrfc4->CreateSurface ( & g_DDSrfcDesc, & g_pPrimDDSrfc, NULL ) ) )
				return FALSE;

			g_DDSrfcDesc.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

			if ( FAILED ( g_pPrimDDSrfc->GetAttachedSurface ( & g_DDSrfcDesc.ddsCaps, & g_pBackDDSrfc ) ) )
				return FALSE;

			LPDIRECTDRAWCLIPPER pScreenDDClip = NULL;
			LPRGNDATA pDDClipRegion;
			RECT ClipRegion;

			if ( FAILED ( g_pDDIntrfc4->CreateClipper ( 0, & pScreenDDClip, NULL ) ) )
				return FALSE;

			ClipRegion.left = 0;
			ClipRegion.top = 0;
			ClipRegion.right = iXRes;
			ClipRegion.bottom = iYRes;

			pDDClipRegion = ( LPRGNDATA ) malloc ( sizeof ( RGNDATAHEADER ) + sizeof ( RECT ) );
			memcpy ( pDDClipRegion->Buffer, & ClipRegion, sizeof ( ClipRegion ) );

			pDDClipRegion->rdh.dwSize = sizeof ( RGNDATAHEADER );
			pDDClipRegion->rdh.iType = RDH_RECTANGLES;
			pDDClipRegion->rdh.nCount = 1;
			pDDClipRegion->rdh.nRgnSize = sizeof ( RECT );
			
			pDDClipRegion->rdh.rcBound.left = ClipRegion.left;
			pDDClipRegion->rdh.rcBound.top = ClipRegion.top;
			pDDClipRegion->rdh.rcBound.right = ClipRegion.right;
			pDDClipRegion->rdh.rcBound.bottom = ClipRegion.bottom;

			if ( FAILED ( pScreenDDClip->SetClipList ( pDDClipRegion, 0 ) ) )
			{
				free ( pDDClipRegion );
				return FALSE;
			}

			if ( FAILED ( g_pBackDDSrfc->SetClipper ( pScreenDDClip ) ) )
			{
				free ( pDDClipRegion );
				return FALSE;
			}

			free ( pDDClipRegion );

			g_VideoContext.iXRes = iXRes;
			g_VideoContext.iYRes = iYRes;
			g_VideoContext.iXMax = iXRes - 1;
			g_VideoContext.iYMax = iYRes - 1;
			g_VideoContext.iColorDepth = iColorDepth;

			ShowCursor ( FALSE );

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_GetScreenXRes ()
		*
		*	Returns the screen's horizontal resolution.
		*/

		int W_GetScreenXRes ()
		{
			return g_VideoContext.iXRes;
		}

		/**************************************************************************************
		*
		*	W_GetScreenYRes ()
		*
		*	Returns the screen's vertical resolution.
		*/

		int W_GetScreenYRes ()
		{
			return g_VideoContext.iYRes;
		}

		/**************************************************************************************
		*
		*	W_GetScreenXMax ()
		*
		*	Returns the screen's maximum horizontal pixel.
		*/

		int W_GetScreenXMax ()
		{
			return g_VideoContext.iXMax;
		}

		/**************************************************************************************
		*
		*	W_GetScreenYMax ()
		*
		*	Returns the screen's maximum vertical pixel.
		*/

		int W_GetScreenYMax ()
		{
			return g_VideoContext.iYMax;
		}

		/**************************************************************************************
		*
		*	W_LockFrame ()
		*
		*	Locks the framebuffer.
		*/

		bool W_LockFrame ()
		{
			InitWin32Struct ( g_DDSrfcDesc );

			if ( FAILED ( g_pBackDDSrfc->Lock ( NULL, & g_DDSrfcDesc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ) ) )
				return FALSE;

			g_pFrameBuffer15 = ( Pixel15 * ) g_DDSrfcDesc.lpSurface;
			g_pFrameBuffer16 = ( Pixel16 * ) g_DDSrfcDesc.lpSurface;
			g_pFrameBuffer32 = ( Pixel32 * ) g_DDSrfcDesc.lpSurface;

			g_iFrameBufferSize = g_DDSrfcDesc.lPitch * g_VideoContext.iYRes;
			g_VideoContext.iPitch = g_DDSrfcDesc.lPitch;

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_BlitFrame ()
		*
		*	Unlocks and blits the framebuffer to the screen.
		*/
	
		bool W_BlitFrame ()
		{
			if ( FAILED ( g_pPrimDDSrfc->Flip ( NULL, DDFLIP_WAIT ) ) )
				return FALSE;
	
			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_ClearFrame ()
		*
		*	Clears the framebuffer.
		*/

		bool W_ClearFrame ()
		{
			InitWin32Struct ( g_DDBlitFX );
			g_DDBlitFX.dwFillColor = 0;

			if ( FAILED ( g_pBackDDSrfc->Blt ( NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, & g_DDBlitFX ) ) )
				return FALSE;

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_LoadImage ()
		*
		*	Loads a BMP file to a Wrappuh image.
		*/

		bool W_LoadImage ( char * pstrBMPFilename, W_Image * Image )
		{
			HFILE hFile;
			OFSTRUCT FileData;

			BITMAPFILEHEADER BMPFileHeader;
			BITMAPINFOHEADER BMPImageHeader;

			if ( ( hFile = OpenFile ( pstrBMPFilename, & FileData, OF_READ ) ) == -1 )
				return FALSE;

			_lread ( hFile, & BMPFileHeader, sizeof ( BITMAPFILEHEADER ) );
			if ( BMPFileHeader.bfType != 0x4D42 )
			{
				_lclose ( hFile );
				return FALSE;
			}

			_lread ( hFile, & BMPImageHeader, sizeof ( BITMAPINFOHEADER ) );
			if ( BMPImageHeader.biBitCount != 24 )
			{
				_lclose ( hFile );
				return FALSE;
			}

			int iScanlineByteSize = BMPImageHeader.biWidth * 3;
			
			GetNextMul4 ( iScanlineByteSize );

			int iScanlinePad = iScanlineByteSize - ( BMPImageHeader.biWidth * 3 );
			int iBMPFileImageByteSize = ( BMPImageHeader.biWidth * 3 + iScanlinePad ) * BMPImageHeader.biHeight;

			_lseek ( hFile, BMPFileHeader.bfOffBits, SEEK_SET );

			UCHAR * pImageBuffer = NULL;
			if ( ! ( pImageBuffer = ( UCHAR * ) malloc ( iBMPFileImageByteSize ) ) )
			{
				_lclose ( hFile );
				return FALSE;
			}
	
			_lread ( hFile, pImageBuffer, iBMPFileImageByteSize );
			
			int iCurrSourcePixelOffs = 0;
			RGBTriad CurrSourcePixelTriad;

			InitWin32Struct ( g_DDSrfcDesc );

			g_DDSrfcDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;

			int iXRes = BMPImageHeader.biWidth;
			GetNextMul4 ( iXRes );

			g_DDSrfcDesc.dwWidth = iXRes;
			g_DDSrfcDesc.dwHeight = BMPImageHeader.biHeight;

			switch ( g_VideoContext.iColorDepth )
			{
				case 15:
				{
					g_DDSrfcDesc.ddckCKSrcBlt.dwColorSpaceLowValue = DEF_IMAGE_MASK_COLOR_15;
					g_DDSrfcDesc.ddckCKSrcBlt.dwColorSpaceHighValue = DEF_IMAGE_MASK_COLOR_15;
					break;
				}

				case 16:
				{
					g_DDSrfcDesc.ddckCKSrcBlt.dwColorSpaceLowValue = DEF_IMAGE_MASK_COLOR_16;
					g_DDSrfcDesc.ddckCKSrcBlt.dwColorSpaceHighValue = DEF_IMAGE_MASK_COLOR_16;
					break;
				}

				case 32:
				{
					g_DDSrfcDesc.ddckCKSrcBlt.dwColorSpaceLowValue = DEF_IMAGE_MASK_COLOR_32;
					g_DDSrfcDesc.ddckCKSrcBlt.dwColorSpaceHighValue = DEF_IMAGE_MASK_COLOR_32;
					break;
				}
			}

			Image->iXRes = BMPImageHeader.biWidth;
			Image->iYRes = g_DDSrfcDesc.dwHeight;
			Image->iXMax = Image->iXRes - 1;
			Image->iYMax = Image->iYRes - 1;

			g_DDSrfcDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

			if ( FAILED ( g_pDDIntrfc4->CreateSurface ( & g_DDSrfcDesc, & Image->pDDSrfc, NULL ) ) )
				return FALSE;

			InitWin32Struct ( g_DDSrfcDesc );
			if ( FAILED ( Image->pDDSrfc->Lock ( NULL, & g_DDSrfcDesc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ) ) )
				return FALSE;

			Image->iPitch = g_DDSrfcDesc.lPitch;

			Pixel15 * pSrfcBuffer15 = ( Pixel15 * ) g_DDSrfcDesc.lpSurface;
			Pixel16 * pSrfcBuffer16 = ( Pixel16 * ) g_DDSrfcDesc.lpSurface;
			Pixel32 * pSrfcBuffer32 = ( Pixel32 * ) g_DDSrfcDesc.lpSurface;

            int iX,
                iY;

			for ( iY = BMPImageHeader.biHeight - 1; iY >= 0; -- iY )
			{
				for ( iX = 0; iX < BMPImageHeader.biWidth; ++ iX )
				{
					CurrSourcePixelTriad.R = pImageBuffer [ iCurrSourcePixelOffs + 2 ];
					CurrSourcePixelTriad.G = pImageBuffer [ iCurrSourcePixelOffs + 1 ];
					CurrSourcePixelTriad.B = pImageBuffer [ iCurrSourcePixelOffs ];

					if ( EncodePixel32 ( CurrSourcePixelTriad.R, CurrSourcePixelTriad.G, CurrSourcePixelTriad.B ) == DEF_IMAGE_MASK_COLOR_32 )
					{
						switch ( g_VideoContext.iColorDepth )
						{
							case 15:
								pSrfcBuffer15 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] = ( Pixel15 ) DEF_IMAGE_MASK_COLOR_15;
								break;

							case 16:
								pSrfcBuffer16 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] = ( Pixel16 ) DEF_IMAGE_MASK_COLOR_16;
								break;

							case 32:
								pSrfcBuffer32 [ iY * ( g_DDSrfcDesc.lPitch >> 2 ) + iX ] = ( Pixel32 ) DEF_IMAGE_MASK_COLOR_32;
								break;
						}
					}
					else
					{
						switch ( g_VideoContext.iColorDepth )
						{
							case 15:
								CurrSourcePixelTriad.R >>= 3;
								CurrSourcePixelTriad.G >>= 3;
								CurrSourcePixelTriad.B >>= 3;
								pSrfcBuffer15 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] = ( Pixel15 ) EncodePixel15 ( CurrSourcePixelTriad.R, CurrSourcePixelTriad.G, CurrSourcePixelTriad.B );
								break;

							case 16:
								CurrSourcePixelTriad.R >>= 3;
								CurrSourcePixelTriad.G >>= 3;
								CurrSourcePixelTriad.B >>= 3;
								pSrfcBuffer16 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] = ( Pixel16 ) EncodePixel16 ( CurrSourcePixelTriad.R, CurrSourcePixelTriad.G, CurrSourcePixelTriad.B );
								break;

							case 32:
								pSrfcBuffer32 [ iY * ( g_DDSrfcDesc.lPitch >> 2 ) + iX ] = ( Pixel32 ) EncodePixel32 ( CurrSourcePixelTriad.R, CurrSourcePixelTriad.G, CurrSourcePixelTriad.B );
								break;
						}
					}

					iCurrSourcePixelOffs += 3;
				}
				iCurrSourcePixelOffs += iScanlinePad;
			}

            int iClipFound;
            W_Rect ClipRect;
            ClipRect.iX0 = 0;
            ClipRect.iY0 = 0;
            ClipRect.iX1 = Image->iXMax;
            ClipRect.iY1 = Image->iYMax;

            iClipFound = FALSE;
            for ( iX = 0; iX < Image->iXRes; ++ iX )
			{	
				for ( iY = 0; iY < Image->iYRes; ++ iY )
				{
                    switch ( g_VideoContext.iColorDepth )
                    {
                        case 15:
                            if ( pSrfcBuffer15 != NULL && pSrfcBuffer15 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_15 )
                                iClipFound = TRUE;
                            break;

                        case 16:
                            if ( pSrfcBuffer16 != NULL && pSrfcBuffer16 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_16 )
                                iClipFound = TRUE;
                            break;

                        case 32:
                            if ( pSrfcBuffer32 != NULL && pSrfcBuffer32 [ iY * ( g_DDSrfcDesc.lPitch >> 2 ) + iX ] != DEF_IMAGE_MASK_COLOR_32 )
                                iClipFound = TRUE;
                            break;
                    }
                    if ( iClipFound )
                        break;
                }
                if ( iClipFound )
                    break;
            }
            ClipRect.iX0 = iX;

            iClipFound = FALSE;
            for ( iX = Image->iXMax; iX >= 0; -- iX )
			{	
				for ( iY = 0; iY < Image->iYRes; ++ iY )
				{
                    switch ( g_VideoContext.iColorDepth )
                    {
                        case 15:
                            if ( pSrfcBuffer15 != NULL && pSrfcBuffer15 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_15 )
                                iClipFound = TRUE;
                            break;

                        case 16:
                            if ( pSrfcBuffer16 != NULL && pSrfcBuffer16 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_16 )
                                iClipFound = TRUE;
                            break;

                        case 32:
                            if ( pSrfcBuffer32 != NULL && pSrfcBuffer32 [ iY * ( g_DDSrfcDesc.lPitch >> 2 ) + iX ] != DEF_IMAGE_MASK_COLOR_32 )
                                iClipFound = TRUE;
                            break;
                    }
                    if ( iClipFound )
                        break;
                }
                if ( iClipFound )
                    break;
            }
            ClipRect.iX1 = iX;

            iClipFound = FALSE;
	        for ( iY = 0; iY < Image->iYRes; ++ iY )            
			{	
                for ( iX = 0; iX < Image->iXRes; ++ iX )
				{
                    switch ( g_VideoContext.iColorDepth )
                    {
                        case 15:
                            if ( pSrfcBuffer15 != NULL && pSrfcBuffer15 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_15 )
                                iClipFound = TRUE;
                            break;

                        case 16:
                            if ( pSrfcBuffer16 != NULL && pSrfcBuffer16 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_16 )
                                iClipFound = TRUE;
                            break;

                        case 32:
                            if ( pSrfcBuffer32 != NULL && pSrfcBuffer32 [ iY * ( g_DDSrfcDesc.lPitch >> 2 ) + iX ] != DEF_IMAGE_MASK_COLOR_32 )
                                iClipFound = TRUE;
                            break;
                    }
                    if ( iClipFound )
                        break;
                }
                if ( iClipFound )
                    break;
            }
            ClipRect.iY0 = iY;

            iClipFound = FALSE;
	        for ( iY = Image->iYMax; iY >= 0; -- iY )            
			{	
                for ( iX = 0; iX < Image->iXRes; ++ iX )
				{
                    switch ( g_VideoContext.iColorDepth )
                    {
                        case 15:
                            if ( pSrfcBuffer15 != NULL && pSrfcBuffer15 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_15 )
                                iClipFound = TRUE;
                            break;

                        case 16:
                            if ( pSrfcBuffer16 != NULL && pSrfcBuffer16 [ iY * ( g_DDSrfcDesc.lPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_16 )
                                iClipFound = TRUE;
                            break;

                        case 32:
                            if ( pSrfcBuffer32 != NULL && pSrfcBuffer32 [ iY * ( g_DDSrfcDesc.lPitch >> 2 ) + iX ] != DEF_IMAGE_MASK_COLOR_32 )
                                iClipFound = TRUE;
                            break;
                    }
                    if ( iClipFound )
                        break;
                }
                if ( iClipFound )
                    break;
            }
            ClipRect.iY1 = iY;

            ClipRect.iX1 -= ClipRect.iX0;
            ClipRect.iY1 -= ClipRect.iY0;

            Image->ClipRect = ClipRect;

			if ( FAILED ( Image->pDDSrfc->Unlock ( NULL ) ) )
				return FALSE;
			
			free ( pImageBuffer );
			_lclose ( hFile );

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_FreeImage ()
		*
		*	Frees an image.
		*/

		void W_FreeImage ( W_Image * Image )
		{
			if ( Image->pDDSrfc != NULL )
			{
				Image->pDDSrfc->Release ();
				Image->pDDSrfc = NULL;
			}
		}

		/**************************************************************************************
		*
		*	W_BlitImage ()
		*
		*	Blits an image.
		*/

		bool W_BlitImage ( W_Image Image, int iX, int iY )
		{
			RECT SourceRect;
			SourceRect.left = 0;
			SourceRect.top = 0;
			SourceRect.right = Image.iXRes;
			SourceRect.bottom = Image.iYRes;

			RECT DestRect;
			DestRect.left = iX;
			DestRect.top = iY;
			DestRect.right = iX + Image.iXRes;
			DestRect.bottom = iY + Image.iYRes;


			if ( FAILED ( g_pBackDDSrfc->Blt ( & DestRect, Image.pDDSrfc, & SourceRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL ) ) )
				return FALSE;

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_DrawPoint ()
		*
		*	Draws a point.
		*/

		void W_DrawPoint ( UCHAR iR, UCHAR iG, UCHAR iB, int iX, int iY )
		{
			if ( iX < 0 || iY < 0 || iX > g_VideoContext.iXMax || iY > g_VideoContext.iYMax )
				return;

			switch ( g_VideoContext.iColorDepth )
			{
				case 15:
				{
					g_pFrameBuffer15 [ iY * ( g_VideoContext.iPitch >> 1 ) + iX ] = ( Pixel15 ) EncodePixel15 ( iR, iG, iB );
					break;
				}

				case 16:
				{
					g_pFrameBuffer16 [ iY * ( g_VideoContext.iPitch >> 1 ) + iX ] = ( Pixel16 ) EncodePixel16 ( iR, iG, iB );
					break;
				}

				case 32:
				{
					g_pFrameBuffer32 [ iY * ( g_VideoContext.iPitch >> 2 ) + iX ] = ( Pixel32 ) EncodePixel32 ( iR, iG, iB );
					break;
				}
			}
		}

		/***************************************************************************************
		*
		*	W_LoadFont ()
		*
		*	Loads a font.
		*/

		bool W_LoadFont ( char * pstrFontBMPFilename, int iCellXRes, int iCellYRes )
		{
			if ( ! W_LoadImage ( pstrFontBMPFilename, & g_FontImage ) )
				return FALSE;

			InitWin32Struct ( g_DDSrfcDesc );

			if ( FAILED ( g_FontImage.pDDSrfc->Lock ( NULL, & g_DDSrfcDesc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ) ) )
				return FALSE;

			Pixel15 * pSrfcBuffer15 = NULL;
			Pixel16 * pSrfcBuffer16 = NULL;
			Pixel32 * pSrfcBuffer32 = NULL;

			switch ( g_VideoContext.iColorDepth )
			{
				case 15:
				{
					pSrfcBuffer15 = ( Pixel15 * ) g_DDSrfcDesc.lpSurface;
					break;
				}

				case 16:
				{
					pSrfcBuffer16 = ( Pixel16 * ) g_DDSrfcDesc.lpSurface;
					break;
				}

				case 32:
				{
					pSrfcBuffer32 = ( Pixel32 * ) g_DDSrfcDesc.lpSurface;
					break;
				}
			}

			g_FontDesc.iCellXRes = iCellXRes;
			g_FontDesc.iCellYRes = iCellYRes;
			g_FontDesc.iCellFullXRes = g_FontDesc.iCellXRes + 1;
			g_FontDesc.iCellFullYRes = g_FontDesc.iCellYRes + 1;
			g_FontDesc.iCharRowSize = ( g_FontImage.iXRes - ( g_FontImage.iXRes % ( g_FontDesc.iCellXRes + 1 ) ) ) / ( g_FontDesc.iCellXRes + 1 );
			g_FontDesc.iCharRowRes = g_FontDesc.iCharRowSize * ( g_FontDesc.iCellXRes + 1 );
			g_FontDesc.iCharRowMaxX = g_FontImage.iXRes - 1;

			g_FontDesc.iSpaceXRes = ( int ) ( g_FontDesc.iCellXRes * DEF_SPACE_PRCNT );

			int iCharX = 0, iCharY = 0;
			int iX, iY;
			int iKern, iCurrKern;

			for ( int iCurrFontCharIndex = 0; iCurrFontCharIndex < DEF_FONT_CHAR_COUNT; ++ iCurrFontCharIndex )
			{
				g_FontCharDesc [ iCurrFontCharIndex ].iX = iCharX;
				g_FontCharDesc [ iCurrFontCharIndex ].iY = iCharY;

				iKern = 15;
				iCurrKern = 15;

				for ( iY = iCharY; iY < iCharY + g_FontDesc.iCellYRes; ++ iY )
				{	
					for ( iX = iCharX; iX < iCharX + g_FontDesc.iCellXRes; ++ iX )
					{
						if ( ( pSrfcBuffer15 != NULL && pSrfcBuffer15 [ iY * ( g_FontImage.iPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_15 ) ||
							 ( pSrfcBuffer16 != NULL && pSrfcBuffer16 [ iY * ( g_FontImage.iPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_16 ) ||
							 ( pSrfcBuffer32 != NULL && pSrfcBuffer32 [ iY * ( g_FontImage.iPitch >> 2 ) + iX ] != DEF_IMAGE_MASK_COLOR_32 ) )
						{
							iCurrKern = iX - iCharX;
							break;
						}
					}

					if ( iCurrKern < iKern )
						iKern = iCurrKern;
				}
				g_FontCharDesc [ iCurrFontCharIndex ].iLeftKern = iKern;

				iKern = 15;
				iCurrKern = 15;
				for ( iY = iCharY; iY < iCharY + g_FontDesc.iCellYRes; ++ iY )
				{
					for ( iX = iCharX + g_FontDesc.iCellXRes - 1; iX >= iCharX ; -- iX )
					{
						if ( ( pSrfcBuffer15 != NULL && pSrfcBuffer15 [ iY * ( g_FontImage.iPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_15 ) ||
							 ( pSrfcBuffer16 != NULL && pSrfcBuffer16 [ iY * ( g_FontImage.iPitch >> 1 ) + iX ] != DEF_IMAGE_MASK_COLOR_16 ) ||
							 ( pSrfcBuffer32 != NULL && pSrfcBuffer32 [ iY * ( g_FontImage.iPitch >> 2 ) + iX ] != DEF_IMAGE_MASK_COLOR_32 ) )
						{
							iCurrKern = ( g_FontCharDesc [ iCurrFontCharIndex ].iX + g_FontDesc.iCellXRes - 1 ) - iX;
							break;
						}
					}

					if ( iCurrKern < iKern )
						iKern = iCurrKern;
				}
				g_FontCharDesc [ iCurrFontCharIndex ].iRightKern = iKern;

				g_FontCharDesc [ iCurrFontCharIndex ].iXRes = g_FontDesc.iCellXRes - ( g_FontCharDesc [ iCurrFontCharIndex ].iLeftKern + g_FontCharDesc [ iCurrFontCharIndex ].iRightKern );

				iCharX += g_FontDesc.iCellFullXRes;
				if ( iCharX > g_FontDesc.iCharRowMaxX )
				{
					iCharX = 0;
					iCharY += g_FontDesc.iCellFullYRes;
				}

				g_FontCharDesc [ iCurrFontCharIndex ].iXRes = g_FontDesc.iCellXRes - ( g_FontCharDesc [ iCurrFontCharIndex ].iLeftKern + g_FontCharDesc [ iCurrFontCharIndex ].iRightKern );
			}

			if ( FAILED ( g_FontImage.pDDSrfc->Unlock ( NULL ) ) )
				return FALSE;

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_FreeFont ()
		*
		*	Frees the font.
		*/

		void W_FreeFont ()
		{
			if ( g_FontImage.pDDSrfc != NULL )
			{
				g_FontImage.pDDSrfc->Release ();
				g_FontImage.pDDSrfc = NULL;
			}
		}

		/**************************************************************************************
		*

		*	W_DrawTextString ()
		*
		*	Draws a string of text.
		*/

		bool W_DrawTextString ( char * pstrTextString, int iX, int iY )
		{
			int iCurrChar;

			for ( unsigned int iCharIndex = 0; iCharIndex < strlen ( pstrTextString ); ++ iCharIndex )
			{
				iCurrChar = pstrTextString [ iCharIndex ] - 32;

				if ( iCurrChar == 0 )
					iX += g_FontDesc.iSpaceXRes;
				else
				{
					iX -= g_FontCharDesc [ iCurrChar ].iLeftKern;

					RECT SourceRect;
					SourceRect.left = g_FontCharDesc [ iCurrChar ].iX;
					SourceRect.top = g_FontCharDesc [ iCurrChar ].iY;
					SourceRect.right = g_FontCharDesc [ iCurrChar ].iX + g_FontDesc.iCellXRes - 1;
					SourceRect.bottom = g_FontCharDesc [ iCurrChar ].iY + g_FontDesc.iCellXRes - 1;

					RECT DestRect;
					DestRect.left = iX;
					DestRect.top = iY;
					DestRect.right = iX + g_FontDesc.iCellXRes - 1;
					DestRect.bottom = iY + g_FontDesc.iCellYRes - 1;

					if ( FAILED ( g_pBackDDSrfc->Blt ( & DestRect, g_FontImage.pDDSrfc, & SourceRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL ) ) )
						return FALSE;

					iX += g_FontCharDesc [ iCurrChar ].iLeftKern + g_FontCharDesc [ iCurrChar ].iXRes + DEF_KERN;
				}
			}

			return TRUE;
		}

		/***************************************************************************************
		*
		*	W_GetStringPixelLength ()
		*
		*	Returns the length of a string in pixels.
		*/

		int W_GetStringPixelLength ( char * pstrTextString )
		{
			int iCurrChar;
			int iStringPixelLength = 0;

			for ( unsigned int iCharIndex = 0; iCharIndex < strlen ( pstrTextString ); ++ iCharIndex )
			{
				iCurrChar = pstrTextString [ iCharIndex ] - 32;

				if ( iCurrChar == 0 )
					iStringPixelLength += g_FontDesc.iSpaceXRes;
				else
				{
					iStringPixelLength += g_FontCharDesc [ iCurrChar ].iXRes;
					if ( iCharIndex < strlen ( pstrTextString ) - 1 )
						iStringPixelLength += DEF_KERN;
				}
			}

			return iStringPixelLength;
		}

        /**************************************************************************************
        *
        *   W_OffsetRect ()
        *
        *   Offsets a rectangle into another coordiante space.
        */

        W_Rect W_OffsetRect ( W_Rect Rect, int iX, int iY )
        {
            Rect.iX0 += iX;
            Rect.iY0 += iY;
            Rect.iX1 += iX;
            Rect.iY1 += iY;

            return Rect;
        }

        /**************************************************************************************
        *
        *   W_DoRectsIntersect ()
        *
        *   Determines whether or not two rectangles intersect.
        */

        bool W_DoRectsIntersect ( W_Rect * Rect0, W_Rect * Rect1 )
        {
            int iCenterX0,
                iCenterY0;
            int iCenterX1,
                iCenterY1;

            int iWidth0,
                iHeight0;
            int iWidth1,
                iHeight1;

            iWidth0 = ( Rect0->iX1 - Rect0->iX0 ) >> 1;
            iHeight0 = ( Rect0->iY1 - Rect0->iY0 ) >> 1;
            iWidth1 = ( Rect1->iX1 - Rect1->iX0 ) >> 1;
            iHeight1 = ( Rect1->iY1 - Rect1->iY0 ) >> 1;

            iCenterX0 = ( iWidth0 >> 1 ) + Rect0->iX0;
            iCenterY0 = ( iHeight0 >> 1 ) + Rect0->iY0;
            iCenterX1 = ( iWidth1 >> 1 ) + Rect1->iX0;
            iCenterY1 = ( iHeight1 >> 1 ) + Rect1->iY0;

            int iDeltaX,
                iDeltaY;

            iDeltaX = abs ( iCenterX1 - iCenterX0 );
            iDeltaY = abs ( iCenterY1 - iCenterY0 );

            if ( ( iDeltaX < ( iWidth0 + iWidth1 ) ) &&
                 ( iDeltaY < ( iHeight0 + iHeight1 ) ) )
            {
                return TRUE;
            }

            return FALSE;
        }

	// ---- Input -----------------------------------------------------------------------------

		/**************************************************************************************
		*
		*	W_GetKbrdState ()
		*
		*	Gets the keyboard state
		*/

		void W_GetKbrdState ()
		{
			g_pDIKbrd->GetDeviceState ( 256, ( LPVOID ) g_KbrdState );

			unsigned int iCurrTickCount = GetTickCount ();

			for ( int iCurrKeyIndex = 0; iCurrKeyIndex < 256; ++ iCurrKeyIndex )
			{
				g_KbrdFrameState [ iCurrKeyIndex ] = FALSE;
				if ( g_KbrdState [ iCurrKeyIndex ] )
				{
					if ( g_KbrdDelay [ iCurrKeyIndex ] == 0 || ! g_iKeyDelayActive )
					{
						g_KbrdFrameState [ iCurrKeyIndex ] = TRUE;
						g_KbrdDelay [ iCurrKeyIndex ] = iCurrTickCount + KEY_DELAY;
					}
					else
						if ( iCurrTickCount >= g_KbrdDelay [ iCurrKeyIndex ] )
							g_KbrdDelay [ iCurrKeyIndex ] = 0;
				}
			}
		}

		/**************************************************************************************
		*
		*	W_GetKeyState ()
		*
		*	Returns the state of a given key.
		*/

		int W_GetKeyState ( int iScanCode )
		{
			if ( g_KbrdState [ iScanCode ] && g_KbrdFrameState [ iScanCode ] )
				return TRUE;

			return FALSE;
		}

		/**************************************************************************************
		*
		*	W_GetAnyKeyState ()
		*
		*	Returns whether or not any key has been pressed.
		*/

		int W_GetAnyKeyState ()
		{
			for ( int iCurrKeyIndex = 0; iCurrKeyIndex < 256; ++ iCurrKeyIndex )
				if ( g_KbrdState [ iCurrKeyIndex ] && g_KbrdFrameState [ iCurrKeyIndex ] )
					return TRUE;

			return FALSE;
		}

        /**************************************************************************************
        *
        *   W_EnableKeyDelay ()
        *
        *   Enables the key delay.
        */

        void W_EnableKeyDelay ()
        {
            g_iKeyDelayActive = TRUE;
        }

        /**************************************************************************************
        *
        *   W_DisableKeyDelay ()
        *
        *   Disables the key delay.
        */

        void W_DisableKeyDelay ()
        {
            g_iKeyDelayActive = FALSE;
        }

	// ---- Audio -----------------------------------------------------------------------------

		/**************************************************************************************
		*
		*	W_LoadSound ()
		*
		*	Loads a WAV file to a Wrappuh sound.
		*/

		bool W_LoadSound ( char * pstrWAVFilename, W_Sound * Sound, bool bLoop )
		{
            if ( ( Sound->iChannels [ 0 ] = DSound_Load_WAV ( pstrWAVFilename ) ) == -1 )
                return FALSE;

            for ( int iCurrChannel = 1; iCurrChannel < SOUND_CHANNEL_COUNT; ++ iCurrChannel )
            {
                Sound->iChannels [ iCurrChannel ] = DSound_Replicate_Sound ( Sound->iChannels [ 0 ] );
            }

            Sound->iChannelIndex = 0;

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_FreeSound ()
		*
		*	Frees a sound.
		*/

		void W_FreeSound ( W_Sound * Sound )
		{
            DSound_Delete_Sound( Sound->iChannels [ 0 ] );
		}

		/**************************************************************************************
		*
		*	W_PlaySound ()
		*
		*	Plays a sound.
		*/

		bool W_PlaySound ( W_Sound & Sound )
		{
            if ( ! DSound_Play ( Sound.iChannels [ ( Sound.iChannelIndex ++ ) % SOUND_CHANNEL_COUNT ] ) )
                return FALSE;

			return TRUE;
		}

		/**************************************************************************************
		*
		*	W_StopSound ()
		*
		*	Stops a sound.
		*/

		bool W_StopSound ( W_Sound Sound )
		{
			return TRUE;
		}

        /**************************************************************************************
        *
        *   W_StopAllSounds ()
        *
        *   Stops all currently playing sounds.
        */

        void W_StopAllSounds ()
        {
            DSound_Stop_All_Sounds ();
        }

        ///////////////////////////////////////////////////////////

        int DSound_Load_WAV(char *filename, int control_flags)
        {
        // this function loads a .wav file, sets up the directsound 
        // buffer and loads the data into memory, the function returns 
        // the id number of the sound


        HMMIO 			hwav;    // handle to wave file
        MMCKINFO		parent,  // parent chunk
                        child;   // child chunk
        WAVEFORMATEX    wfmtx;   // wave format structure

        int	sound_id = -1,       // id of sound to be loaded
	        index;               // looping variable

        UCHAR *snd_buffer,       // temporary sound buffer to hold voc data
              *audio_ptr_1=NULL, // data ptr to first write buffer 
	          *audio_ptr_2=NULL; // data ptr to second write buffer

        DWORD audio_length_1=0,  // length of first write buffer
	          audio_length_2=0;  // length of second write buffer
			        
        // step one: are there any open id's ?
        for (index=0; index < MAX_SOUNDS; index++)
	        {	
            // make sure this sound is unused
	        if (sound_fx[index].state==SOUND_NULL)
	           {
	           sound_id = index;
	           break;
	           } // end if

	        } // end for index

        // did we get a free id?
        if (sound_id==-1)
	        return(-1);

        // set up chunk info structure
        parent.ckid 	    = (FOURCC)0;
        parent.cksize 	    = 0;
        parent.fccType	    = (FOURCC)0;
        parent.dwDataOffset = 0;
        parent.dwFlags		= 0;

        // copy data
        child = parent;

        // open the WAV file
        if ((hwav = mmioOpen(filename, NULL, MMIO_READ | MMIO_ALLOCBUF))==NULL)
            return(-1);

        // descend into the RIFF 
        parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

        if (mmioDescend(hwav, &parent, NULL, MMIO_FINDRIFF))
            {
            // close the file
            mmioClose(hwav, 0);

            // return error, no wave section
            return(-1); 	
            } // end if

        // descend to the WAVEfmt 
        child.ckid = mmioFOURCC('f', 'm', 't', ' ');

        if (mmioDescend(hwav, &child, &parent, 0))
            {
            // close the file
            mmioClose(hwav, 0);

            // return error, no format section
            return(-1); 	
            } // end if

        // now read the wave format information from file
        if (mmioRead(hwav, (char *)&wfmtx, sizeof(wfmtx)) != sizeof(wfmtx))
            {
            // close file
            mmioClose(hwav, 0);

            // return error, no wave format data
            return(-1);
            } // end if

        // make sure that the data format is PCM
        if (wfmtx.wFormatTag != WAVE_FORMAT_PCM)
            {
            // close the file
            mmioClose(hwav, 0);

            // return error, not the right data format
            return(-1); 
            } // end if

        // now ascend up one level, so we can access data chunk
        if (mmioAscend(hwav, &child, 0))
           {
           // close file
           mmioClose(hwav, 0);

           // return error, couldn't ascend
           return(-1); 	
           } // end if

        // descend to the data chunk 
        child.ckid = mmioFOURCC('d', 'a', 't', 'a');

        if (mmioDescend(hwav, &child, &parent, MMIO_FINDCHUNK))
            {
            // close file
            mmioClose(hwav, 0);

            // return error, no data
            return(-1); 	
            } // end if

        // finally!!!! now all we have to do is read the data in and
        // set up the directsound buffer

        // allocate the memory to load sound data
        snd_buffer = (UCHAR *)malloc(child.cksize);

        // read the wave data 
        mmioRead(hwav, (char *)snd_buffer, child.cksize);

        // close the file
        mmioClose(hwav, 0);

        // set rate and size in data structure
        sound_fx[sound_id].rate  = wfmtx.nSamplesPerSec;
        sound_fx[sound_id].size  = child.cksize;
        sound_fx[sound_id].state = SOUND_LOADED;

        // set up the format data structure
        memset(&pcmwf, 0, sizeof(WAVEFORMATEX));

        pcmwf.wFormatTag	  = WAVE_FORMAT_PCM;  // pulse code modulation
        pcmwf.nChannels		  = 1;                // mono 
        pcmwf.nSamplesPerSec  = 11025;            // always this rate
        pcmwf.nBlockAlign	  = 1;                
        pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
        pcmwf.wBitsPerSample  = 8;
        pcmwf.cbSize		  = 0;

        // prepare to create sounds buffer
        dsbd.dwSize			= sizeof(DSBUFFERDESC);
        dsbd.dwFlags		= control_flags | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
        dsbd.dwBufferBytes	= child.cksize;
        dsbd.lpwfxFormat	= &pcmwf;

        // create the sound buffer
        if (FAILED(lpds->CreateSoundBuffer(&dsbd,&sound_fx[sound_id].dsbuffer,NULL)))
           {
           // release memory
           free(snd_buffer);

           // return error
           return(-1);
           } // end if

        // copy data into sound buffer
        if (FAILED(sound_fx[sound_id].dsbuffer->Lock(0,					 
								              child.cksize,			
								              (void **) &audio_ptr_1, 
								              &audio_length_1,
								              (void **)&audio_ptr_2, 
								              &audio_length_2,
								              DSBLOCK_FROMWRITECURSOR)))
								         return(0);

        // copy first section of circular buffer
        memcpy(audio_ptr_1, snd_buffer, audio_length_1);

        // copy last section of circular buffer
        memcpy(audio_ptr_2, (snd_buffer+audio_length_1),audio_length_2);

        // unlock the buffer
        if (FAILED(sound_fx[sound_id].dsbuffer->Unlock(audio_ptr_1, 
									            audio_length_1, 
									            audio_ptr_2, 
									            audio_length_2)))
 							             return(0);

        // release the temp buffer
        free(snd_buffer);

        // return id
        return(sound_id);

        } // end DSound_Load_WAV

        ///////////////////////////////////////////////////////////

        int DSound_Replicate_Sound(int source_id)
        {
        // this function replicates the sent sound and sends back the
        // id of the replicated sound, you would use this function
        // to make multiple copies of a gunshot or something that
        // you want to play multiple times simulataneously, but you
        // only want to load once

        if (source_id!=-1)
            {
            // duplicate the sound buffer
            // first hunt for an open id

            for (int id=0; id < MAX_SOUNDS; id++)
                {
                // is this sound open?
                if (sound_fx[id].state==SOUND_NULL)
                    {
                    // first make an identical copy
                    sound_fx[id] = sound_fx[source_id];

                    // now actually replicate the directsound buffer
                    if (FAILED(lpds->DuplicateSoundBuffer(sound_fx[source_id].dsbuffer,
                                                   &sound_fx[id].dsbuffer)))
                        {
                        // reset sound to NULL
                        sound_fx[id].dsbuffer = NULL;
                        sound_fx[id].state    = SOUND_NULL;

                        // return error
                        return(-1);
                        } // end if

                    // now fix up id
                    sound_fx[id].id = id;
            
                    // return replicated sound
                    return(id);

                    } // end if found
  
                } // end for id

            } // end if
        else
           return(-1);
    
        // else failure
        return(-1);

        } // end DSound_Replicate_Sound

        //////////////////////////////////////////////////////////

        int DSound_Init(void)
        {
        // this function initializes the sound system
        static int first_time = 1; // used to track the first time the function
                                   // is entered

        // test for very first time
        if (first_time)
	        {		
	        // clear everything out
	        memset(sound_fx,0,sizeof(pcm_sound)*MAX_SOUNDS);
	        
	        // reset first time
	        first_time = 0;

	        // create a directsound object
	        if (FAILED(DirectSoundCreate(NULL, &lpds, NULL)))
		        return(0);

	        // set cooperation level
	        if (FAILED(lpds->SetCooperativeLevel((HWND)g_hMainWindow,DSSCL_NORMAL)))
		        return(0);

	        } // end if

        // initialize the sound fx array
        for (int index=0; index<MAX_SOUNDS; index++)
	        {
	        // test if this sound has been loaded
	        if (sound_fx[index].dsbuffer)
		        {
		        // stop the sound
		        sound_fx[index].dsbuffer->Stop();

		        // release the buffer
		        sound_fx[index].dsbuffer->Release();
	        
		        } // end if

	        // clear the record out
	        memset(&sound_fx[index],0,sizeof(pcm_sound));

	        // now set up the fields
	        sound_fx[index].state = SOUND_NULL;
	        sound_fx[index].id    = index;

	        } // end for index

        // return sucess
        return(1);

        } // end DSound_Init

        ///////////////////////////////////////////////////////////

        int DSound_Shutdown(void)
        {
        // this function releases all the memory allocated and the directsound object
        // itself

        // first turn all sounds off
        DSound_Stop_All_Sounds();

        // now release all sound buffers
        for (int index=0; index<MAX_SOUNDS; index++)
	        if (sound_fx[index].dsbuffer)
		        sound_fx[index].dsbuffer->Release();

        // now release the directsound interface itself
        if (lpds)
           lpds->Release();

        // return success
        return(1);

        } // end DSound_Shutdown

        ///////////////////////////////////////////////////////////

        int DSound_Play(int id, int flags, int volume, int rate, int pan)
        {
        // this function plays a sound, the only parameter that 
        // works is the flags which can be 0 to play once or
        // DSBPLAY_LOOPING

        if (sound_fx[id].dsbuffer)
	        {
	        // reset position to start
	        if (FAILED(sound_fx[id].dsbuffer->SetCurrentPosition(0)))
		        return(0);
	        
	        // play sound
	        if (FAILED(sound_fx[id].dsbuffer->Play(0,0,flags)))
		        return(0);
	        } // end if

        // return success
        return(1);

        } // end DSound_Play

        ///////////////////////////////////////////////////////////

        int DSound_Set_Volume(int id,int vol)
        {
        // this function sets the volume on a sound 0-100

        if (sound_fx[id].dsbuffer->SetVolume(DSVOLUME_TO_DB(vol))!=DS_OK)
	        return(0);

        // return success
        return(1);

        } // end DSound_Set_Volume

        ///////////////////////////////////////////////////////////

        int DSound_Set_Freq(int id,int freq)
        {
        // this function sets the playback rate

        if (sound_fx[id].dsbuffer->SetFrequency(freq)!=DS_OK)
	        return(0);

        // return success
        return(1);

        } // end DSound_Set_Freq

        ///////////////////////////////////////////////////////////

        int DSound_Set_Pan(int id,int pan)
        {
        // this function sets the pan, -10,000 to 10,0000

        if (sound_fx[id].dsbuffer->SetPan(pan)!=DS_OK)
	        return(0);

        // return success
        return(1);

        } // end DSound_Set_Pan

        ////////////////////////////////////////////////////////////

        int DSound_Stop_Sound(int id)
        {
        // this function stops a sound from playing
        if (sound_fx[id].dsbuffer)
           {
           sound_fx[id].dsbuffer->Stop();
           sound_fx[id].dsbuffer->SetCurrentPosition(0);
           } // end if

        // return success
        return(1);

        } // end DSound_Stop_Sound

        ///////////////////////////////////////////////////////////

        int DSound_Delete_All_Sounds(void)
        {
        // this function deletes all the sounds

        for (int index=0; index < MAX_SOUNDS; index++)
            DSound_Delete_Sound(index);

        // return success always
        return(1);

        } // end DSound_Delete_All_Sounds

        ///////////////////////////////////////////////////////////

        int DSound_Delete_Sound(int id)
        {
        // this function deletes a single sound and puts it back onto the available list

        // first stop it
        if (!DSound_Stop_Sound(id))
           return(0);

        // now delete it
        if (sound_fx[id].dsbuffer)
           {
           // release the com object
           sound_fx[id].dsbuffer->Release();
           sound_fx[id].dsbuffer = NULL;
   
           // return success
           return(1);
           } // end if

        // return success
        return(1);

        } // end DSound_Delete_Sound

        ///////////////////////////////////////////////////////////

        int DSound_Stop_All_Sounds(void)
        {
        // this function stops all sounds

        for (int index=0; index<MAX_SOUNDS; index++)
	        DSound_Stop_Sound(index);	

        // return success
        return(1);

        } // end DSound_Stop_All_Sounds

        ///////////////////////////////////////////////////////////

        int DSound_Status_Sound(int id)
        {
        // this function returns the status of a sound
        if (sound_fx[id].dsbuffer)
	        {
	        ULONG status; 

	        // get the status
	        sound_fx[id].dsbuffer->GetStatus(&status);

	        // return the status
	        return(status);

	        } // end if
        else // total failure
	        return(-1);

        } // end DSound_Status_Sound

        ///////////////////////////////////////////////////////////

        int DMusic_Load_MIDI(char *filename)
        {
        // this function loads a midi segment

        DMUS_OBJECTDESC ObjDesc; 
        HRESULT hr;
        IDirectMusicSegment* pSegment = NULL;

        int index; // loop var
 
        // look for open slot for midi segment
        int id = -1;

        for (index = 0; index < DM_NUM_SEGMENTS; index++)
            {
            // is this one open
            if (dm_midi[index].state == MIDI_NULL)
               {
               // validate id, but don't validate object until loaded
               id = index;
               break;
               } // end if

            } // end for index

        // found good id?
        if (id==-1)
           return(-1);

        // get current working directory
        char szDir[_MAX_PATH];
        WCHAR wszDir[_MAX_PATH]; 

        if(_getcwd( szDir, _MAX_PATH ) == NULL)
          {
          return(-1);;
          } // end if

        MULTI_TO_WIDE(wszDir, szDir);

        // tell the loader were to look for files
        hr = dm_loader->SetSearchDirectory(GUID_DirectMusicAllTypes,wszDir, FALSE);

        if (FAILED(hr)) 
           {
           return (-1);
           } // end if

        // convert filename to wide string
        WCHAR wfilename[_MAX_PATH]; 
        MULTI_TO_WIDE(wfilename, filename);
 
        // setup object description
        DD_INIT_STRUCT(ObjDesc);
        ObjDesc.guidClass = CLSID_DirectMusicSegment;
        wcscpy(ObjDesc.wszFileName, wfilename );
        ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
 
        // load the object and query it for the IDirectMusicSegment interface
        // This is done in a single call to IDirectMusicLoader::GetObject
        // note that loading the object also initializes the tracks and does 
        // everything else necessary to get the MIDI data ready for playback.

        hr = dm_loader->GetObject(&ObjDesc,IID_IDirectMusicSegment, (void**) &pSegment);

        if (FAILED(hr))
           return(-1);
 
        // ensure that the segment plays as a standard MIDI file
        // you now need to set a parameter on the band track
        // Use the IDirectMusicSegment::SetParam method and let 
        // DirectMusic find the trackby passing -1 (or 0xFFFFFFFF) in the dwGroupBits method parameter.

        hr = pSegment->SetParam(GUID_StandardMIDIFile,-1, 0, 0, (void*)dm_perf);

        if (FAILED(hr))
           return(-1);
  
        // This step is necessary because DirectMusic handles program changes and 
        // bank selects differently for standard MIDI files than it does for MIDI 
        // content authored specifically for DirectMusic. 
        // The GUID_StandardMIDIFile parameter must be set before the instruments are downloaded. 

        // The next step is to download the instruments. 
        // This is necessary even for playing a simple MIDI file 
        // because the default software synthesizer needs the DLS data 
        // for the General MIDI instrument set
        // If you skip this step, the MIDI file will play silently.
        // Again, you call SetParam on the segment, this time specifying the GUID_Download parameter:

        hr = pSegment->SetParam(GUID_Download, -1, 0, 0, (void*)dm_perf);

        if (FAILED(hr))
           return(-1);

        // at this point we have MIDI loaded and a valid object

        dm_midi[id].dm_segment  = pSegment;
        dm_midi[id].dm_segstate = NULL;
        dm_midi[id].state       = MIDI_LOADED;
 
        // return id
        return(id);
 
        } // end DMusic_Load_MIDI

        //////////////////////////////////////////////////////////

        int DMusic_Play(int id)
        {
        // play sound based on id

        if (dm_midi[id].dm_segment && dm_midi[id].state!=MIDI_NULL)
           {
           // if there is an active midi then stop it
           if (dm_active_id!=-1)
               DMusic_Stop(dm_active_id);

           // play segment and force tracking of state variable
           dm_perf->PlaySegment(dm_midi[id].dm_segment, 0, 0, &dm_midi[id].dm_segstate);
           dm_midi[id].state = MIDI_PLAYING;

           // set the active midi segment
           dm_active_id = id;
           return(1);
           }  // end if
        else
            return(0);

        } // end DMusic_Play

        //////////////////////////////////////////////////////////

        int DMusic_Stop(int id)
        {
        // stop a midi segment
        if (dm_midi[id].dm_segment && dm_midi[id].state!=MIDI_NULL)
           {
           // play segment and force tracking of state variable
           dm_perf->Stop(dm_midi[id].dm_segment, NULL, 0, 0);
           dm_midi[id].state = MIDI_STOPPED;

           // reset active id
           dm_active_id = -1;

           return(1);
           }  // end if
        else
            return(0);

        } // end DMusic_Stop

        ///////////////////////////////////////////////////////////

        int DMusic_Delete_MIDI(int id)
        {
        // this function deletes one MIDI segment

        // Unload instruments this will cause silence.
        // CloseDown unloads all instruments, so this call is also not 
        // strictly necessary.
        if (dm_midi[id].dm_segment)
           {
           dm_midi[id].dm_segment->SetParam(GUID_Unload, -1, 0, 0, (void*)dm_perf); 

           // Release the segment and set to null
           dm_midi[id].dm_segment->Release(); 
           dm_midi[id].dm_segment  = NULL;
           dm_midi[id].dm_segstate = NULL;
           dm_midi[id].state       = MIDI_NULL;
           } // end if

        return(1);

        } // end DMusic_Delete_MIDI

        //////////////////////////////////////////////////////////

        int DMusic_Delete_All_MIDI(void)
        {
        // delete all the MIDI 
        int index; // loop var

        // free up all the segments
        for (index = 0; index < DM_NUM_SEGMENTS; index++)
            {
            // Unload instruments this will cause silence.
            // CloseDown unloads all instruments, so this call is also not 
            // strictly necessary.
            if (dm_midi[index].dm_segment)
               {
               dm_midi[index].dm_segment->SetParam(GUID_Unload, -1, 0, 0, (void*)dm_perf); 

               // Release the segment and set to null
               dm_midi[index].dm_segment->Release(); 
               dm_midi[index].dm_segment  = NULL;
               dm_midi[index].dm_segstate = NULL;
               dm_midi[index].state       = MIDI_NULL;
               } // end if

            } // end for index

        return(1);

        } // end DMusic_Delete_All_MIDI

        //////////////////////////////////////////////////////////

        int DMusic_Status_MIDI(int id)
        {
        // this checks the status of a midi segment

        if (dm_midi[id].dm_segment && dm_midi[id].state !=MIDI_NULL )
           {
           // get the status and translate to our defines
           if (dm_perf->IsPlaying(dm_midi[id].dm_segment,NULL) == S_OK) 
              dm_midi[id].state = MIDI_PLAYING;
           else
              dm_midi[id].state = MIDI_STOPPED;

           return(dm_midi[id].state);
           } // end if
        else
           return(0);

        } // end DMusic_Status_MIDI

        ///////////////////////////////////////////////////////////

        int DMusic_Init(void)
        {
        // this function initializes directmusic, it also checks if directsound has
        // been initialized, if so it connect the wave output to directsound, otherwise
        // it creates it's own directsound object, hence you must start directsound up
        // first if you want to use both directsound and directmusic

        int index; // looping var

        // set up directmusic
        // initialize COM
        if (FAILED(CoInitialize(NULL)))
           {    
           // Terminate the application.
           return(0);
           }   // end if

        // create the performance
        if (FAILED(CoCreateInstance(CLSID_DirectMusicPerformance,
                                    NULL,
                                    CLSCTX_INPROC,
                                    IID_IDirectMusicPerformance,
                                    (void**)&dm_perf)))    
           {
           // return null        
           return(0);
           } // end if

        // initialize the performance, check if directsound is on-line if so, use the
        // directsound object, otherwise create a new one
        if (FAILED(dm_perf->Init(NULL, lpds, g_hMainWindow)))
           {
           return(0);// Failure -- performance not initialized
           } // end if 

        // add the port to the performance
        if (FAILED(dm_perf->AddPort(NULL)))
           {    
           return(0);// Failure -- port not initialized
           } // end if

        // create the loader to load object(s) such as midi file
        if (FAILED(CoCreateInstance(
                  CLSID_DirectMusicLoader,
                  NULL,
                  CLSCTX_INPROC, 
                  IID_IDirectMusicLoader,
                  (void**)&dm_loader)))
           {
           // error
           return(0);
           } // end if

        // reset all the midi segment objects
        for (index = 0; index < DM_NUM_SEGMENTS; index++)
            {
            // reset the object
            dm_midi[index].dm_segment  = NULL;  
            dm_midi[index].dm_segstate = NULL;  
            dm_midi[index].state       = MIDI_NULL;
            dm_midi[index].id          = index;
            } // end for index

        // reset the active id
        dm_active_id = -1;

        // all good baby
        return(1);

        } // end DMusic_Init

        ////////////////////////////////////////////////////////////

        int DMusic_Shutdown(void)
        {
        // If there is any music playing, stop it. This is 
        // not really necessary, because the music will stop when
        // the instruments are unloaded or the performance is    
        // closed down.
        if (dm_perf)
           dm_perf->Stop(NULL, NULL, 0, 0 ); 

        // delete all the midis if they already haven't been
        DMusic_Delete_All_MIDI();

        // CloseDown and Release the performance object.    
        if (dm_perf)
           {
           dm_perf->CloseDown();
           dm_perf->Release();     
           } // end if

        // Release the loader object.
        if (dm_loader)
           dm_loader->Release();     

        // Release COM
        CoUninitialize(); 

        // return success
        return(1);

        } // end DMusic_Shutdown

	// ---- Timer -----------------------------------------------------------------------------

		/**************************************************************************************
		*
		*	W_GetTickCount ()
		*
		*	Returns the tick count.
		*/

		DWORD W_GetTickCount ()
		{
			return GetTickCount ();
		}

		/**************************************************************************************
		*
		*	W_InitTimer ()
		*
		*	Initializes a timer.
		*/

		W_TimerHandle W_InitTimer ( int iLength )
		{
			bool bWasFreeTimerFound = FALSE;
			for ( int iCurrTimerIndex = 0; iCurrTimerIndex < MAX_TIMER_COUNT; ++ iCurrTimerIndex )
			{
				if ( g_Timers [ iCurrTimerIndex ].bIsNull )
				{
					bWasFreeTimerFound = TRUE;
					g_Timers [ iCurrTimerIndex ].bIsNull = FALSE;
					g_Timers [ iCurrTimerIndex ].iLength = iLength;
					g_Timers [ iCurrTimerIndex ].iActiveTime = GetTickCount () + iLength;
					break;
				}
			}

			if ( bWasFreeTimerFound )
				return iCurrTimerIndex;
			else
				return -1;
		}

		/**************************************************************************************
		*
		*	W_ClearTimer ()
		*
		*	Clears a timer.
		*/

		void W_ClearTimer ( W_TimerHandle hTimer )
		{
			if ( hTimer >= 0 && hTimer < MAX_TIMER_COUNT )
				g_Timers [ hTimer ].bIsNull = TRUE;
		}

		/**************************************************************************************
		*
		*	W_HandleTimers ()
		*
		*	Called once per frame to keep timers running.
		*/

		void W_HandleTimers ()
		{
			int iCurrTickCount = GetTickCount ();

			for ( int iCurrTimerIndex = 0; iCurrTimerIndex < MAX_TIMER_COUNT; ++ iCurrTimerIndex )
				if ( ! g_Timers [ iCurrTimerIndex ].bIsNull )
				{
					g_Timers [ iCurrTimerIndex ].bIsActive = FALSE;
					if ( iCurrTickCount >= g_Timers [ iCurrTimerIndex ].iActiveTime )
					{
						g_Timers [ iCurrTimerIndex ].bIsActive = TRUE;
						g_Timers [ iCurrTimerIndex ].iActiveTime = iCurrTickCount + g_Timers [ iCurrTimerIndex ].iLength;
					}
				}
		}
		
		/**************************************************************************************
		*
		*	W_GetTimerState ()
		*
		*	Returns the state of the timer
		*/

		bool W_GetTimerState ( W_TimerHandle hTimer )
		{
			return g_Timers [ hTimer ].bIsActive;
		}

		/**************************************************************************************
		*
		*	W_Delay ()
		*
		*	Suspends execution of the program for the specified duration.
		*/

		void W_Delay ( unsigned int iLength )
		{
			unsigned int iStartTime = GetTickCount ();

            while ( GetTickCount () < iStartTime + iLength );
		}

        /**************************************************************************************
        *
        *   W_GetHighPerformanceTickCount ()
        *
        *   Returns the tick count from the high-performance timer.
        */

        W_Int64 W_GetHighPerformanceTickCount ()
        {
            W_Int64 iTickCount;

            QueryPerformanceCounter ( ( LARGE_INTEGER * ) & iTickCount );

            return iTickCount / g_iTimerFreqPerMs;
        }

    // ---- Misc ------------------------------------------------------------------------------

        /**************************************************************************************
        *
        *   W_GetRandInRange ()
        *
        *   Returns a random number within the specified range.
        */
        
        int W_GetRandInRange ( int iMin, int iMax )
        {
            return ( rand () % ( iMax - iMin + 1 ) ) + iMin;
        }