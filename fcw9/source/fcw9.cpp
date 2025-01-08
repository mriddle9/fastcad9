//	======================================================================
//	fcw9.cpp - FastCAD v9 Main Program Application Object
//	----------------------------------------------------------------------
//	Copyright ©2001-2012 Evolution Computing, Inc.
//	All rights reserved
//	======================================================================

#include "fwkw.h"
#include "rscid.h"

int	nRscBase = 0;
wchar_t szAppDataEx[] = L"/Evolution Computing/FastCAD v9";
wchar_t szDocumentsEx[] = L"/FastCAD9";

#define _SPECIFIEDEXPATHS
#include "winmain.h"

#define NOMENU 0

class CApplication : public IMObject
{
public:
	CApplication(void);
	~CApplication(void);
	virtual void* Msg(MSGDISPATCH);
	void* OnOpenFirstDocument(void);
	void* OnOpenNamedDocument(MSGP);

	void* OnDocWinOpening(void);
	void* OnDocWinClosing(void);
	void* OnStartShutdown(void);
	void* OnMenuScript(MSGP);
public:
	void ToggleIDraw(bool fSetupOnly);
public:
	IWindow*	hWindow;
	ICtlTabPages* hTabPages;

	DRECTF		drect;
	int			nOpenDocWindows;
	uint32_t	winStyle;

	uint32_t	 nMenuId;
	uint32_t	ixIDraw;
};

//	referenced externals

IControl* MakeMainCtrl(IControl* hParent, CTLLOCN* pLocn,wchar_t* pTTip);
DLLEXPORT IDialog* MakeDlgAbout(void);

//	======================================================================
//	Application Object Class Factory
//	======================================================================

IApplication* MakeApplication(void)
{
	return (IApplication*) new CApplication();
}

//	======================================================================
//	Application Object Constructor
//	======================================================================

CApplication::CApplication(void)
{
	pApplication = (IMObject*)this;
	hWindow = NULL;
	ixIDraw = 0;
	nMenuId = 10;

	hWinMgr->WMgrSetDPIScale(1.25);
	nOpenDocWindows = 0;
}

//	======================================================================
//	Application Object Destructor
//	======================================================================

CApplication::~CApplication(void)
{
}

//	======================================================================
//	Variable length message receiver/dispatcher
//	======================================================================

wchar_t szAppName[] = L"FastCAD_v9";
static wchar_t szAppVersion[] = VER_DISPLAY_CT;
static char szAppDate[] = __DATE__;

void* CApplication::Msg(MSGDISPATCH)
{
	switch(MSGID)
	{
	//	provide standard information about the program

	case MSG_AskIsApplication:		return IM_RTN_TRUE;
	case MSG_AppVersionText:		return MRTNVAL(szAppVersion);
	case MSG_AppVersionDate:		return MRTNVAL(szAppDate);
	case MSG_AppName:				return MRTNVAL(szAppName);

		//  program startup / document load messages

	case MSG_AppOpenNewDocument:
	case MSG_AppOpenFirstDocument:  return OnOpenFirstDocument();
	case MSG_AppOpenNamedDocument:  return OnOpenNamedDocument(MPPTR);

	case MSG_AppDocWinOpening:      return OnDocWinOpening();
	case MSG_AppDocWinClosing:      return OnDocWinClosing();
	case MSG_AppStartShutdown:      return OnStartShutdown();

	case MSG_ScriptLine:
	case MSG_ScriptText:			return OnMenuScript(MPPTR);
	}
	return IM_RTN_IGNORED;
}

//	======================================================================
//  Open new (unnamed) document - Program Startup load
//	======================================================================

void* CApplication::OnOpenFirstDocument(void)
{
	//	load default resources for windowing
	LoadDefaultResources();

	ToggleIDraw(true); // setup wstyle only

	//	reload session settings to allow pAppDataEx to be processed
	hSession->SsnLoadData();

	SESSION* ssnptr = (SESSION*)hSession->SsnGetDataPtr();
	ssnptr->fUntitledWin = false;
	ssnptr->rbtype = RBO_Rect;
	ssnptr->ttshowms = 5000;

	//	do save session settings on exit
	hSession->SsnSaveData();

	//	load this program's resource file
	hRscMgr->LoadRscFile(TEXT("#rsc/fcw9.rsc"));
	uint32_t nSlot = hXPMgr->GetSlotNr(0x7000);
	nRscBase = nSlot << 16; // local RscBase value

	//  create a main window
	if (!hWindow)
		hWindow = MakeWindow(NULL, winStyle, 100, 100, 1024, 768);
	hWindow->WinSetTitle(((IApplication*)this)->AppName());
	hWindow->CtlGetDRect(&drect);
	//	window has soft blue background
//	hWindow->CtlSetBkgndColor(0xFFE0A080);
	hWindow->CtlSetBkgndColor(COLOR_TRANSPARENT);

	//	load menu file
	((IMenuMgr*)hMenuMgr)->MenuLoadFile(TEXT("#rsc/fcw9.mnu"));

	//	create main program menu - displayed above the tab pages control
	IControl* hMenu = MakeMenuBar(hWindow, 1,nMenuId);
	((IControl*)hMenu)->CtlGradientFill(GF_T2B, 0xFF908070, 0xFF403020);
	((IControl*)hMenu)->CtlSetTextColor(COLOR_WHITE);
	((IControl*)hMenu)->CtlSetBkgndColor(0xFF605040);
	((IControl*)hMenu)->CtlSetFrameType(NOOUTLINE);

	//	create tabbed page controller
	CTLLOCN TPLocn = { CLT_ALL,0,0,0,0 };
	hTabPages = MakeCtlTabPages(hWindow, &TPLocn, 400);
	hTabPages->CtlSetNotify(this);
	hTabPages->CtlSetBkgndColor(0xFF605040); // COLOR_LTBLUE); // unused space on hz bar
	hTabPages->CtlTabDetached(); // tab not attached to body (draw crect)

	//	create our details display control (Status) in a Tab Page container
	//	by specifying nMenuId here, each tab page has its own menu, below the tab pages control
	ICtlTabPage* hTabPage1 = MakeTabPage(hTabPages, NOMENU);
	CTLLOCN CtlLocn = { CLT_ALL,0,0,0,0 };
	IControl* hMainCtrl1 = MakeMainCtrl(hTabPage1, &CtlLocn,L"Main content page 1");
	hSystem->SysSleep(1); // needed to prevent race condition in release build (TODO: why?)
	hTabPage1->CtlSetTabView(hMainCtrl1);

	//	create another details display control
	//	by specifying nMenuId here, each tab page has its own menu, below the tab pages control
	ICtlTabPage* hTabPage2 = MakeTabPage(hTabPages, NOMENU);
	IControl* hMainCtrl2 = MakeMainCtrl(hTabPage2, &CtlLocn,L"Main content page 2");
	hTabPage2->CtlSetTabView(hMainCtrl2);

	// make it currently selected
	hTabPages->CtlPageSelect(hTabPage1);

	//  notify window all controls have been allocated
	//  this will trigger the initial load and draw sequence
	hWindow->WinCtlsChanged();

	//	set initial focus to our control
	hWinMgr->WMgrChangeFocus(hMainCtrl1);

	return IM_RTN_NOTHING;
}

//	======================================================================
//  Open Named document
//	======================================================================

void* CApplication::OnOpenNamedDocument(MSGP)
{
	MPARMPTR(IStr*, hFileName);

	hFileName->Release();
	hFileName = NULL;
	return IM_RTN_NOTHING;
}

//	======================================================================
//  Document Window is opening (Window creation notifies of this)
//	======================================================================

void* CApplication::OnDocWinOpening(void)
{
	nOpenDocWindows++;
	return IM_RTN_NOTHING;
}

//	======================================================================
//  Document Window is closing
//	======================================================================

void* CApplication::OnDocWinClosing(void)
{
	nOpenDocWindows--;
	if (nOpenDocWindows <= 0)
		hApplication->AppStartShutdown();
	return IM_RTN_NOTHING;
}

//	======================================================================
//  Initiate normal application shutdown
//	======================================================================

void* CApplication::OnStartShutdown(void)
{
	hWinMgr->WMgrStartShutdown();
	return IM_RTN_NOTHING;
}

//	======================================================================
//	Process command string from a menu item pick (sent thru hScriptMgr)
//	======================================================================

static wchar_t szCmdList[] = L"EXIT\0QUIT\0ABOUT\0HELP\0IDRAW\0\0";
static wchar_t szPrompt[] = L"Command: ";

void* CApplication::OnMenuScript(MSGP)
{
	MPARMPTR(wchar_t*, pCmd);

	//	process command string
	uint32_t nCmd = (uint32_t)hSvc->SvcListFindStrNr(szCmdList,pCmd);
	switch(nCmd)
	{
	case 1:	// EXIT
	case 2:	// QUIT
		hApplication->AppStartShutdown();
		break;

	case 3:	// ABOUT
		MakeDlgAbout();
		break;

	case 4:	// HELP
	{
		//  get desired path to help file
		IStr* hHelpFile = MakeStr(hApplication->AppDataDirPath());
		hHelpFile->StrAppendW(TEXT("FCW9.html"));

		//  save help text to app data dir
		wchar_t* pHelpText = RscTextFile(RSC_HelpFile);
		IStr* hHelpText = MakeStr(pHelpText);
		hHelpText->StrSaveFile(hHelpFile->StrTextPtr());

		//  ask OS to open it as a document (it is an .html file)
		hWinMgr->WMgrShellOpenDoc(hHelpFile->StrTextPtr());

		// cleanup
		hHelpFile->Release();
		hHelpText->Release();
		break;
	}
	case 5: // IDRAW
		ToggleIDraw(false); // perform actual IDraw change
		break;
	default:
		//	bad command
		break;
	}

	return IM_RTN_NOTHING;
}

//	======================================================================
//	Toggle between CIDraw and GDI+ Draw
//	----------------------------------------------------------------------
//	DX2DIDRAW: no gradient fills, no justified text
//      loses pRasterTarget when printer ops, need to recreate it
//	GDIPIDRAW: no gradient fill
//	CIDraw: no arabic joiners, limited RTL text support
//		(if any - code might have workaround)

//	With winstyle alpha, there is no:
//		Title bar
//		Title bar icons: sysmenu, win title, minimize, maximize, close
//		Window edges do not grow/shrink
//		No title bar drag
//		So we add our own title bar, outline resize,title bar icons,
//		titlebar drag - see wintest for code for this
//		if we add this, how do we make it linux compatible?
//		It does enable full-window transparency when dragging#ifndef _SYS_MSWIN
//
//	winStyle = WSTYLE_ALPHA;
//	winStyle = WSTYLE_BKTITLED | WSTYLE_DPISCALESIZE | WSTYLE_DPISCALEORG;
//	winStyle = WSTYLE_BKTITLED;
//	winStyle = WSTYLE_TITLED | WSTYLE_DX2DIDRAW;
//	winStyle = WSTYLE_BKTITLED | WSTYLE_GDIPIDRAW;
//	======================================================================

void CApplication::ToggleIDraw(bool fSetupOnly)
{
#ifndef _SYS_MSWIN
	winStyle = WSTYLE_BKTITLED | WSTYLE_DPISCALESIZE | WSTYLE_DPISCALEORG;
	return;
#else	
	//	toggle the winStyle flags
	if (ixIDraw != 0)
		winStyle = WSTYLE_BKTITLED | WSTYLE_GDIPIDRAW;
	else
		winStyle = WSTYLE_BKTITLED;
	ixIDraw = 1 - ixIDraw;

	if (fSetupOnly)
		return;

	//	change window's winstyle settings

	//	resize and redraw hWindow
#endif
}

//	======================================================================
