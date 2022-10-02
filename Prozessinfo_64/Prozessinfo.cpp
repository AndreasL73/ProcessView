// Prozessinfo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "stdio.h"
#include "winspool.h" // fürs ausrucken benötigt
#include "CommDlg.h"  //für common dialoge
#include "tlhelp32.h"
#define MAX_LOADSTRING 100           //stringlaenge
#define ID_LIST 1                     
#define TEXTDATEI "prozesse.txt"   //hier werden die infos gespeichert

// Global Variables:
HINSTANCE hInst;								// Instanz
TCHAR szTitle[MAX_LOADSTRING];								// Titel im Fensterrahmen
TCHAR szWindowClass[MAX_LOADSTRING];								// Name der Fensterklasse
HWND hwndlist; //handle für die listbox
DWORD meineProzesse[1024]; 
DWORD cbNeeded;  // Bytes benötigt für prozessinfo
DWORD cProcesses; //anzahl prozesse
UINT i=0,fensternummer=0; //zaehlvariable
HINSTANCE hInstLib;
OPENFILENAME ofn;

// Vorwärtsdeklarationen
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void sende_prozessinfo(DWORD processID,HWND hwndlist);
void hole_alle_prozesse(HWND hwndlist);
void speichere_in_datei(HWND hwndlist);
BOOL drucke_auf_drucker(HWND hwndlist);
HDC GetPrinterDC(void);
void hole_fensterliste();
BOOL CALLBACK EnumProc(HWND ehandle,LPARAM y);

////////////////////////////////////////////////////////////////
// PSAPI.dll Zeiger auf externe PSAPI.dll Funktionen, noetig fuer prozessinfos
////////////////////////////////////////////////////////////////   
      BOOL (WINAPI *lpfEnumProcesses)( DWORD *, DWORD cb, DWORD * );
      BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *,
         DWORD, LPDWORD );
      DWORD (WINAPI *lpfGetModuleFileNameEx)( HANDLE, HMODULE,
         LPTSTR, DWORD );
      DWORD (WINAPI *lpfGetModuleBaseName)(HANDLE,HMODULE,LPTSTR,DWORD);  

//////////////////////////////////////////////////////////////////////

// Type definitions for pointers to call tool help functions

typedef BOOL (WINAPI* MODULEWALK)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

typedef BOOL (WINAPI* THREADWALK)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);

typedef BOOL (WINAPI* PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);

typedef HANDLE (WINAPI* CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);

// File scope globals. These pointers are declared because of the need

// to dynamically link to the functions. They are exported only by

// the Windows kernel. Explicitly linking to them will make this

// application unloadable in Windows NT and will produce an ugly

// system dialog box


static CREATESNAPSHOT pCreateToolhelp32Snapshot = NULL;
static MODULEWALK pModule32First = NULL;
static MODULEWALK pModule32Next = NULL;
static PROCESSWALK pProcess32First = NULL;
static PROCESSWALK pProcess32Next = NULL;
static THREADWALK pThread32First = NULL;
static THREADWALK pThread32Next = NULL;

// Function that initializes tool help functions

BOOL InitToolhelp32(void)
{
    BOOL bRet = FALSE;
    HINSTANCE hKernel = NULL;

    // Obtain the module handle of the kernel to retrieve addresses

    // of the tool helper functions

    hKernel = GetModuleHandle("KERNEL32.DLL");

    if(hKernel)
    {
    pCreateToolhelp32Snapshot = 
    (CREATESNAPSHOT)GetProcAddress(hKernel,    "CreateToolhelp32Snapshot");

    pModule32First = (MODULEWALK)GetProcAddress(hKernel,"Module32First");

    pModule32Next = (MODULEWALK)GetProcAddress(hKernel,"Module32Next");

    pProcess32First=(PROCESSWALK)GetProcAddress(hKernel,"Process32First");

    pProcess32Next = (PROCESSWALK)GetProcAddress(hKernel,"Process32Next");

    pThread32First = (THREADWALK)GetProcAddress(hKernel,"Thread32First");

    pThread32Next = (THREADWALK)GetProcAddress(hKernel,"Thread32Next");

    // All addresses must be non-NULL to be successful.

    // If one of these addresses is NULL, one of the needed

    // list cannot be walked.

    bRet = pModule32First && pModule32Next && pProcess32First &&
           pProcess32Next && pThread32First && pThread32Next &&
           pCreateToolhelp32Snapshot;
    }
    else
        bRet = FALSE; // could not even get the handle of kernel


    return bRet;
}


 


/////////////////////////////////////////////////////////////////
//alle Desktop Child windows abfragen
////////////////////////////////////////////////////////////////
void hole_fensterliste()
{
EnumWindows((WNDENUMPROC)EnumProc,0);
};


///////////////////////////////////////////////////////////////////////////
//callback funktion für  enumwindows(), liste alle fenster in listbox
//////////////////////////////////////////////////////////////////////////
BOOL CALLBACK EnumProc(HWND ehandle, LPARAM y)
{
	char buffer[1024]={0};
	char buffer2[1024]={0};

	RECT meinrect;
	GetWindowRect(ehandle,&meinrect);
	if(IsWindowVisible(ehandle))
	{
		GetWindowText(ehandle,buffer2,sizeof (buffer2));

		sprintf(buffer,"Handle: %08i    Groesse: %04ix%04i    Titel: %-30s ",ehandle,meinrect.right-meinrect.left,meinrect.bottom-meinrect.top,buffer2);


		SendMessage(hwndlist,LB_ADDSTRING,0,(WPARAM) buffer);

	}
   return TRUE;
};











//////////////////////////////////////////////////////////////////////////////////
//ListBox auf StandardDrucker drucken
//////////////////////////////////////////////////////////////////////////////////
BOOL drucke_auf_drucker(HWND hwndlist)
{
    HDC hdc=GetDC(hwndlist);
	HDC handlefuerdrucker;
	handlefuerdrucker=GetPrinterDC();
	TEXTMETRIC meinemetric;
	GetTextMetrics(hdc,&meinemetric);

    int texthoehe=meinemetric.tmHeight;
	int textbreite=meinemetric.tmWeight;
	static DOCINFO di={sizeof(DOCINFO),TEXT("Print1:Druck")};
	BOOL bSuccess=TRUE;

	if(NULL==(handlefuerdrucker)  )
	{
		MessageBox(NULL,TEXT("Kein Drucker vorhanden !"),TEXT("Druck"),SW_SHOW);
	};

   if(StartDoc(handlefuerdrucker,&di)>0)
   {
     if(StartPage(handlefuerdrucker)>0)
	 {
     ///////////////////////////////////
	 ///ausdruck kommt hier !
     //////////////////////////////////
       
		 
		 
		for(int i=0;i<(SendMessage(hwndlist,LB_GETCOUNT,0,0));++i)	  
	
	
        
		 {
      char tchBuffer[250];
	  SendMessage(hwndlist,LB_GETTEXT,i,(LPARAM)tchBuffer);
      int nSize=sprintf(tchBuffer, "%s",tchBuffer); 
      TextOut(handlefuerdrucker, textbreite+2, texthoehe*i*6, tchBuffer, nSize); 
         };




	 if(EndPage(handlefuerdrucker)>0) EndDoc(handlefuerdrucker);
	 else bSuccess=FALSE;

	 }
   else bSuccess=FALSE;
   
   
   
   }

DeleteDC(handlefuerdrucker);
return bSuccess;


};

////////////////////////////////////////////////////////////////////////
//// Standard Drucker Device Context holen. wie im Petzold beschrieben
///////////////////////////////////////////////////////////////////////

HDC GetPrinterDC(void)
{

	HDC hdc=NULL;
	PRINTDLG prndlg;
	memset(&prndlg,0, sizeof(PRINTDLG)); 
	prndlg.lStructSize=sizeof(PRINTDLG);
	prndlg.Flags= PD_RETURNDC | PD_PRINTSETUP;;
	prndlg.hwndOwner= NULL;
	prndlg.hDevMode= NULL;
	prndlg.hDevNames= NULL;
	prndlg.hDC = NULL;

	if(PrintDlg(&prndlg)==TRUE)
	hdc=prndlg.hDC;
	
	return hdc;
}


//////////////////////////////////////////////////////////////////////////
//alle ListBox Eintraege werden in TEXTDATEI geschrieben
/////////////////////////////////////////////////////////////////////////	  
void speichere_in_datei(HWND hwndlist)
{   
	FILE *meinedatei;
	TCHAR szPath[MAX_PATH]={0};
	GetCurrentDirectory(sizeof(szPath),szPath);
	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
	TCHAR teststring[MAX_PATH];
	teststring[0]='\0';
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrInitialDir = szPath;
	ofn.lpstrFilter=TEXT("Text Dateien (*.txt)\0*.txt\0All Files (*.*)\0\\*.*\0");
	ofn.lpstrDefExt="txt";
	ofn.nMaxFile=sizeof(szPath);
	ofn.lpstrFile=teststring;




	if(GetSaveFileName(&ofn)==TRUE)
    {
	lstrcpy(teststring,ofn.lpstrFile);
	meinedatei=fopen(teststring,"w");
	fprintf(meinedatei,"Prozesse,Windows (C)2010 Andreas Leinz\n\n");
        
          
		for(int i=0;i<(SendMessage(hwndlist,LB_GETCOUNT,0,0));++i)
		{
			char puffer[250]={0};
			SendMessage(hwndlist,LB_GETTEXT,i,(LPARAM)puffer);
			fprintf(meinedatei,"%s\n",puffer);
        };
        fclose(meinedatei);
	};

		
};





 

/////////////////////////////////////////////////////////////////////////////////////////////////
//psapi.dll laden, alle Prozesse abfragen,Prozessliste durchlaufen und sendeprozessinfo aufrufen 
//////////////////////////////////////////////////////////////////////////////////////////////////
void hole_alle_prozesse(HWND hwndlist)
{

  if(!InitToolhelp32())
   {
     MessageBox(0,"Unable to initialize tool help functions !", "Error",MB_OK);
   }
  HANDLE hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  PROCESSENTRY32 pe;
  pe.dwSize = sizeof(pe);
  BOOL i = pProcess32First(hSnapshot, &pe);
  if (!i)
       ::MessageBox(0,"Could not list processes !", "Error",MB_OK);
     char buffer[1024]={0};

   sprintf(buffer,"PID: %04d --  Threads: %04d --  %s",pe.th32ProcessID,pe.cntThreads,pe.szExeFile);

   SendMessage(hwndlist,LB_ADDSTRING,0,(LPARAM)buffer);
  while( pProcess32Next(hSnapshot, &pe))
   {
   char buffer[1024]={0};

   sprintf(buffer,"PID: %04d --  Threads: %04d --  %s",pe.th32ProcessID,pe.cntThreads,pe.szExeFile);

   SendMessage(hwndlist,LB_ADDSTRING,0,(LPARAM)buffer);

   };

 
CloseHandle(hSnapshot); // Done with this snapshot. Free it



};



////////////////////////////////////////////////////////////////////
/// prozessinfos an Listbox senden
///////////////////////////////////////////////////////////////////
void sende_prozessinfo(DWORD processID,HWND hwndlist)
{

	hInstLib = LoadLibraryA( "PSAPI.DLL" ) ;
         if( hInstLib == NULL )
            MessageBox(NULL,"Konnte Psapi.dll nicht finden","Fehler",SW_SHOW) ;
     

    // Get procedure addresses.
         lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*))
            GetProcAddress( hInstLib, "EnumProcesses" ) ;
         lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,
            DWORD, LPDWORD)) GetProcAddress( hInstLib,
            "EnumProcessModules" ) ;
         lpfGetModuleFileNameEx =(DWORD (WINAPI *)(HANDLE, HMODULE,
            LPTSTR, DWORD )) GetProcAddress( hInstLib,
            "GetModuleFileNameExA" ) ;
         lpfGetModuleBaseName=(DWORD (WINAPI *)(HANDLE, HMODULE,
            LPTSTR, DWORD )) GetProcAddress( hInstLib,
            "GetModuleFileNameExA" ) ;



	char szProcessName[MAX_PATH]="unbekannter Prozess";

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,processID);

	if (hProcess)
    {
	HMODULE hMod;
	DWORD cbNeeded;
	  if(lpfEnumProcessModules(hProcess,&hMod,sizeof(hMod),&cbNeeded))
	  {
		
		  lpfGetModuleFileNameEx(hProcess,hMod,szProcessName,sizeof(szProcessName));
	//	  lpfGetModuleBaseName(hProcess,hMod,szProcessName,sizeof(szProcessName));
	  }
	
	
	
	}
    char buffer[1024]={0};

	sprintf(buffer,"PID: %04d     %s",processID,szProcessName);

SendMessage(hwndlist,LB_ADDSTRING,0,(LPARAM)buffer);

CloseHandle(hProcess);
FreeLibrary( hInstLib ) ;

};











/////////////////////////////////////////////////////////////////////////////
//Winmain , Programmeinstiegspunkt
////////////////////////////////////////////////////////////////////////////




int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	
	MSG msg;
	HACCEL hAccelTable; //Accelerators sind SchnellstartTasten

///Initialisiere globale Strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PROZESSINFO, szWindowClass, MAX_LOADSTRING);
// Fensterklasse registrieren und sichtbar machen
	MyRegisterClass(hInstance);

// Initialisierungsroutine InitInstance() initialisiert
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_PROZESSINFO);

// Nachrichtenwarteschlange abfragen:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNKTION: MyRegisterClass()
//
//  Zweck: Registriert die Fenster Klasse
//


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_PROZESSINFO);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	=CreateSolidBrush(0); //schwarzer hintergrund !!! ;-)    //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_PROZESSINFO;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

/////////////////////////////////////////////////////////////////////
//   FUNCTION: InitInstance(HANDLE, int)
//
//   Zweck: speichert Instanz handle and zeigt  Hauptfenster
/////////////////////////////////////////////////////////////////////

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   //int cxChar=LOWORD(GetDialogBaseUnits()),cyChar=HIWORD(GetDialogBaseUnits());

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   RECT r;
   GetClientRect(hWnd,&r);
   int cx,cy;
   cx=r.right-r.left;
   cy=r.bottom-r.top;
    // meine listbox entsteht hier !
   hwndlist=CreateWindow(TEXT("listbox"),NULL,WS_SIZEBOX|WS_CAPTION|WS_CHILD|WS_VISIBLE|LBS_STANDARD,
	   0,0,cx,cy,hWnd,(HMENU) ID_LIST,NULL,hInst);
   
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//  FUNKTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  ZWECK:  verarbeitet Nachrichten an das Hauptfenster
//
//  WM_COMMAND	- bearbeitet MenueNachrichten
//  WM_PAINT	- zeichnet das Hauptfenster
//  WM_DESTROY	- sendet Abschiedsnachricht und beendet Programm
//
//////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				
		    //auffrischen prozessliste
			case IDS_STRING110:
				{
                SendMessage(hwndlist,LB_RESETCONTENT,0,0); //listbox loeschen 
				hole_alle_prozesse(hwndlist);
				
				};			
				break;
            
            // ausdrucken prozessliste  
            case IDS_STRING111: drucke_auf_drucker(hwndlist);
				break;
            
            // abspeichern prozessliste
            case IDS_STRING112: speichere_in_datei(hwndlist);
				break;
            
            case IDS_STRING113:
				{SendMessage(hwndlist,LB_RESETCONTENT,0,0); //listbox loeschen 
                 hole_fensterliste();    

				}
				break;
            
            case ID_FENSTER_SPEICHERN:speichere_in_datei(hwndlist);break;

			case ID_FENSTER_DRUCKEN:drucke_auf_drucker(hwndlist);break; 
			
			
			
			case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rt;
			GetClientRect(hWnd, &rt);
         //SendMessage(hwndlist,LB_RESETCONTENT,0,0);
			//hole_alle_prozesse(hwndlist);
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////
// die Nachrichtenbearbeitungsroutine für mein InfoFenster (AboutBox)
/////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
