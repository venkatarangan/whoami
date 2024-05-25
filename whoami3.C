#include <windows.h>
#include <winsock.h>
#include "WhoAmI3.h"

#define MY_TIMER 1

LRESULT CALLBACK	WndProc ( HWND, UINT, WPARAM, LPARAM );
void				PrintNames ( int , int , HDC, HWND);
int					WinSockInit ( ) ;
int					iWinSock;

struct hostent FAR * tHostDetails;
 

char				*MyCmdLine;
int					nTimer;



int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					PSTR szCmdLine, int iCmdShow )
{
	HWND hwnd;
	MSG msg;
	WNDCLASSEX wndclass;
	static char szAppName[]="Who am I?";

	MyCmdLine = malloc ( sizeof( char ) * strlen( szCmdLine ) );
	strcpy( MyCmdLine,szCmdLine );

	wndclass.cbSize			= sizeof ( wndclass );
	wndclass.style			= CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc	= WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hInstance;
	wndclass.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
	wndclass.hCursor		= LoadCursor ( NULL, IDC_ARROW );
	wndclass.hbrBackground	= ( HBRUSH ) ( COLOR_WINDOW + 1 );
	wndclass.lpszMenuName	= "WhoAmI3";
	wndclass.lpszClassName	= szAppName;
	wndclass.hIconSm		= LoadIcon ( NULL, IDI_APPLICATION );

	RegisterClassEx ( &wndclass );

	iWinSock = WinSockInit();

	hwnd = CreateWindow ( szAppName,				// Window Class Name
						szAppName,			    // Window Caption
						WS_POPUPWINDOW | WS_CAPTION | WS_BORDER |WS_SYSMENU|WS_MINIMIZEBOX,	// Window style
//						WS_OVERLAPPEDWINDOW,	// Window style
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						XSIZE,					// initial x Size
						YSIZE,						// initial y Size
						NULL,
						NULL,
						hInstance,
						NULL );

	ShowWindow ( hwnd, iCmdShow );
	UpdateWindow ( hwnd );

	while ( GetMessage( &msg, NULL, 0, 0 ) )	{
		TranslateMessage ( &msg );
		DispatchMessage ( &msg );
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc ( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam )
{
	HDC hdc;
	PAINTSTRUCT ps;
	//RECT rect;
	TEXTMETRIC tm;
	char Temp [ 255 ];
	HMENU hMenu;
	//HWND hwndDesktop;

	static int cxChar, cxCaps, cyChar;	

	switch( iMsg )	{
		case WM_CREATE:

			hdc = GetDC( hwnd );
			GetTextMetrics ( hdc, &tm );
			cxChar = tm.tmAveCharWidth; 
			cxCaps = ( tm.tmPitchAndFamily & 1 ? 3 : 2 ) * cxChar / 2;
			cyChar = tm.tmHeight + tm.tmExternalLeading;
			ReleaseDC( hwnd, hdc );
			nTimer = SetTimer(hwnd, MY_TIMER,10000, NULL);
			return 0;

		case WM_TIMER:
			InvalidateRect ( hwnd, NULL, TRUE );
			return 0;
			
		case WM_PAINT:

			hdc = BeginPaint ( hwnd, &ps );
			PrintNames( cxChar, cyChar, hdc, hwnd );
			EndPaint ( hwnd, &ps );
			return 0;

		case WM_SYSCOLORCHANGE:

			InvalidateRect ( hwnd, NULL, TRUE );
			return 0;

		case WM_LBUTTONUP:
			InvalidateRect ( hwnd, NULL, TRUE );
			return 0;

		case WM_COMMAND:

			hMenu = GetMenu(hwnd);
			switch ( LOWORD( wParam ) ) {

				case IDM_REFRESH:
					InvalidateRect ( hwnd, NULL, TRUE );
					return 0;

				case IDM_EXIT:

					SendMessage( hwnd, WM_CLOSE, 0, 0 );
					return 0;
				case IDM_ABOUT:
					wsprintf ( 
								Temp, "%s\n%s\n%s\n",
								"\"Who Am I Pro\" Ver. 3.0",
								"T.N.C.Venkatarangan, Chennai, India",
								"http://www.venkatarangan.com"
								);
					MessageBox (
								hwnd, Temp,
								"About",
								MB_ICONINFORMATION | MB_APPLMODAL 
								);
					return 0;

				case IDM_CASCADE:
					CascadeWindows( GetDesktopWindow(), MDITILE_SKIPDISABLED, NULL, 0, NULL);
					return 0;
				case IDM_TILE:
					TileWindows( GetDesktopWindow(), MDITILE_SKIPDISABLED, NULL, 0, NULL);
					return 0;
				case IDM_RESTORE:
					//SendMessage( GetDesktopWindow(), WM_MDIRESTORE, NULL, NULL);
					return 0;
			}


		case WM_CLOSE:
			if (iWinSock == 0) 
				WSACleanup( );

			DestroyWindow ( hwnd );
			return 0;

		case WM_DESTROY:

			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc ( hwnd, iMsg, wParam, lParam );

}

void PrintNames( int cxChar, int cyChar, HDC hdc, HWND hwnd) {
	
	int Row=0;
	long lTickCount;
	long lMinutes;
	long lSeconds;
	long lHours;

 
	BOOL Ret;

	char Temp[ 255 ];
	char szTmp[ 255 ];

	char UserName[ MYMAX_USERNAME_LENGTH ];
	DWORD NameLength = MYMAX_USERNAME_LENGTH;

	char CompName[ MYMAX_COMPUTERNAME_LENGTH + 1 ];
	DWORD CompLength =	MYMAX_COMPUTERNAME_LENGTH;


	char FAR pHostName[255];
	int iReturnCode;

	GetWindowsDirectory(szTmp, 254);
	SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

	if ( !( Ret = GetComputerName( CompName,&CompLength ) ))	{
	
			MessageBox( hwnd, "Unable to get Computer Name", 
				"Net Initialization",  MB_ICONEXCLAMATION | MB_APPLMODAL);

	}
	else	{

			strcpy( Temp, "Computer Name: ");
			strcat( Temp, CompName );
			TextOut( hdc, cxChar, cyChar * Row, Temp, strlen( Temp ) );
			Row++;
	}

	strcpy( Temp, "HostName (FQDN): ");

	if (iWinSock==0) { 
		iReturnCode = gethostname ( pHostName, 254 ); 
		if (iReturnCode  == 0 ) {
			tHostDetails = gethostbyname ( pHostName );
  			strcat( Temp, tHostDetails->h_name);
		}
		else {
			strcat( Temp, "Error in getting Hostname.");
		}
	}
	else {
			strcat( Temp, "Not able to initialise WinSock 1.1");
	}

	TextOut( hdc, cxChar, cyChar * Row, Temp, strlen( Temp ) );
	Row+=2;

	if (!(Ret = GetUserName(UserName,&NameLength) ) ) 	{

		MessageBox(hwnd,"Unable to get UserName",
			"Net Initialization", 
			 MB_ICONEXCLAMATION|MB_APPLMODAL);

	}
	else {

		strcpy( Temp, "Current logged-in user: ");
		strcat( Temp, UserName );
		TextOut( hdc, cxChar, cyChar * Row, Temp, strlen( Temp ) );
		Row++;

	}


	lTickCount = GetTickCount() ;

	lMinutes = lTickCount / MILLISEC_TO_MIN;
	lSeconds = (lTickCount / 1000 ) - lMinutes*60;

	lHours = lMinutes / 60;
	lMinutes = (lTickCount / MILLISEC_TO_MIN) - (lHours*60);

	wsprintf (
			Temp, "Windows Directory: %s", szTmp);

	TextOut( hdc, cxChar, cyChar * Row, Temp, strlen( Temp ) );
	Row+=2;

	/* if seconds 
	wsprintf (
			Temp,
			"%s%d%s%d%s%d%s",
			"Windows is running for (H:M:S): ",
			lHours, ":",
			lMinutes, ":",
			lSeconds, " "
			);

  */
	wsprintf (
			Temp,
			"%s%d%s%d",
			"Windows is running for (H:M:S): ",
			lHours, ":",
			lMinutes
			);

	TextOut( hdc, cxChar, cyChar * Row, Temp, strlen( Temp ) );
}		


int WinSockInit(){
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 1, 1 );
 
	err = WSAStartup( wVersionRequested, &wsaData );

	if ( LOBYTE( wsaData.wVersion ) != 1 ||   HIBYTE( wsaData.wVersion ) != 1 ) {
		WSACleanup( );
		return 10; 
	}

	return err;
}