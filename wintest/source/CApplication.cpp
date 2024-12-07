//	======================================================================
//  wintest Class Implementation: test window appearance issues
//	----------------------------------------------------------------------
//  Copyright ©2012-2024 Evolution Computing. All rights reserved.
//	======================================================================

#include "fwkw.h"
#include "winmain.h"

IControl* MakeCtlMainFrame(IControl* hParent, wchar_t* pLabel);

#define COLOR_BLUEGREY 0xFF605040
#define COLOR_SOFTBLUE 0xFFE0A080
#define COLOR_MAINFRAME COLOR_SOFTBLUE

int	nRscBase = 0;

//	======================================================================
//  Class Definition
//	======================================================================

class CApplication : public IApplication
{
public:
	CApplication(void);
	~CApplication(void);
	void* Msg(MSGDISPATCH);
	
	void* OnOpenFirstDocument(void);
	void* OnOpenNamedDocument(MSGP);

	void* OnDocWinOpening(void);
	void* OnDocWinClosing(void);
	void* OnStartShutdown(void);

	void* OnMenuScript(MSGP);
	void* OnKeyDown(MSGP);
	void* OnCtlDraw(MSGP);

	void* OnPrBeginPrint(MSGP);
public:
	void DoPrintStg(void);
	void DoPrintMPStg(void);
	void DoPrintUCStg(void);
	void DoPrintGraphics(void);
	void DoPrintDlg(void);
public:
	uint32_t	winStyle;
	IWindow*	hWindow;
	DRECTF      drect;
	IControl*	hMainFrame;
	int			nOpenDocWindows;
};

//	======================================================================
//  Class Factory
//	======================================================================

IApplication* MakeApplication(void)
{
	return (IApplication*)new CApplication();
}

//	======================================================================
//  Constructor
//	----------------------------------------------------------------------
//	With winstyle alpha, there is no:
//		Title bar
//		Title bar icons: sysmenu, win title, minimize, maximize, close
//		Window edges do not grow/shrink
//		No title bar drag
//	======================================================================

CApplication::CApplication(void)
{
	pApplication = (IMObject*)this;
	hWindow = NULL;
	//  winStyle = WSTYLE_ALPHA;
		winStyle = WSTYLE_BKTITLED;
	//	winStyle = WSTYLE_BKTITLED | WSTYLE_DPISCALESIZE | WSTYLE_DPISCALEORG;
	//	winStyle = WSTYLE_TITLED | WSTYLE_DX2DIDRAW;
	//	winStyle = WSTYLE_BKTITLED | WSTYLE_GDIPIDRAW;
	//	DX2DIDRAW: no gradient fills, no justified text
	//             loses pRasterTarget when printer ops, need to recreate it
	//	GDIPIDRAW: no gradient fill
	//	CIDraw: no arabic joiners, limited RTL text support
	nOpenDocWindows = 0;
}

//	======================================================================
//  Destructor
//	======================================================================

CApplication::~CApplication(void)
{
    int debug = 0;
}

//	======================================================================
//  Message dispatch
//	======================================================================

static wchar_t szAppVersion[] = VER_DISPLAY_CT;
static char szAppDate[] = __DATE__;
static wchar_t* szAppName = L"wintest\0";

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

		//	this object acts as the only control in the one window

		case MSG_CtlGetWindow:			return MRTNVAL(hWindow);
		case MSG_CtlGetIDraw:			return hWindow->CtlGetIDraw();
		case MSG_CtlNotifyKeyDown:		return OnKeyDown(MPPTR);
		case MSG_CtlDraw:               return OnCtlDraw(MPPTR);

		case MSG_CtlNeedsRedraw:
		{
			hWindow->WinNeedsRedraw();
			return IM_RTN_FALSE;
		}


		case MSG_ScriptLine:
		case MSG_ScriptText:			return OnMenuScript(MPPTR);
		case MSG_PrBeginPrint:			return OnPrBeginPrint(MPPTR);
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

	//	load this program's resource file
	hRscMgr->LoadRscFile(TEXT("#rsc/wintest.rsc"));
	uint32_t nSlot = hXPMgr->GetSlotNr(0x7000);
	nRscBase = nSlot << 16; // local RscBase value

	//	load menu file
	((IMenuMgr*)hMenuMgr)->MenuLoadFile(TEXT("#rsc/wintest.mnu"));

	//  create a main window
	if (!hWindow)
	{
		hWindow = MakeWindow(NULL, winStyle, 400, 200, 600, 520);
//		hWindow = MakeWindow(NULL, winStyle, 3200, 200, 600, 520);
	}

	hWindow->WinSetTitle(this->AppName());
	hWindow->CtlGetDRect(&drect);
	hWindow->CtlSetBkgndColor(COLOR_TRANSPARENT);
	
	hMainFrame = MakeCtlMainFrame(hWindow,this->AppName());
	hWindow->CtlSetNotify(hMainFrame);
	hMainFrame->CtlSetBkgndColor(COLOR_SOFTBLUE);
	//	set initial focus to our control
	hWinMgr->WMgrChangeFocus(hMainFrame);

	//  notify window any controls have been allocated
	//  this will trigger the initial load and draw sequence
	hWindow->WinCtlsChanged();
	
	return IM_RTN_NOTHING;
}

//	======================================================================
//  Open Named document
//	======================================================================

void* CApplication::OnOpenNamedDocument(MSGP)
{
	MPARMPTR(IStr*,hFileName);
	
	hFileName->Release();
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
//  A key has been pressed, and no control has the focus
//	======================================================================

void* CApplication::OnKeyDown(MSGP)
{
	MPARMINT(wchar_t, key);

	if (key == KEY_ESCAPE)
		hWinMgr->WMgrStartShutdown();

	return IM_RTN_TRUE; // key was used
}

//	======================================================================
//  Draw control contents
//	======================================================================

void* CApplication::OnCtlDraw(MSGP)
{
	MPARMPTR(IDraw*,hIDraw);
	
	//	get window bounds for drawing directly
	hWindow->CtlGetDRect(&drect);
	
	//  clear background to light blue palette color
	hIDraw->IDrawClearBkgnd(0xFFE0A080);
		
	return IM_RTN_NOTHING;
}

//	======================================================================
//	Process command string from a menu item pick (sent thru hScriptMgr)
//	======================================================================

static wchar_t szCmdList[] = L"EXIT\0QUIT\0ABOUT\0HELP\0CLOSE\0"
	L"PRINTSTG\0PRINTMPSTG\0PRINTUCSTG\0PRINTGRAPHICS\0PRINTDLG\0\0";

static wchar_t szPrompt[] = L"Command: ";

void* CApplication::OnMenuScript(MSGP)
{
	MPARMPTR(wchar_t*, pCmd);

	//	process command string
	uint32_t nCmd = (uint32_t)hSvc->SvcListFindStrNr(szCmdList, pCmd);
	switch (nCmd)
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
		/*		//	get desired path to help file
				IStr * hHelpFile = MakeStr(L"~/Wormhole/");
				hHelpFile->StrAppendW(TEXT("Wormhole.html"));
				hHelpFile->StrFullFileName();

				// save help text to user home
				wchar_t* pHelpText = RscTextFile(RSC_WinTestHelpHtml);
				IStr* hHelpText = MakeStr(pHelpText);
				hHelpText->StrSaveFile(hHelpFile->StrTextPtr());

				// ask OS to open it as a document (it is an .html file)
				hWinMgr->WMgrShellOpenDoc(hHelpFile->StrTextPtr());

				// cleanup
				hHelpFile->Release();
				hHelpText->Release();
		*/
		//	get path to help file
		IStr* hHelpFile = MakeStr(L"#rsc/wintest.html");
		hHelpFile->StrFullFileName();
		// ask OS to open it as a document (it is an .html file)
		hWinMgr->WMgrShellOpenDoc(hHelpFile->StrTextPtr());
		hHelpFile->Release();
		break;
	}

	case 5: // CLOSE
		hWindow->WinClose();
		break;

	case 6:	// PRINTSTG
		DoPrintStg();
		break;

	case 7:	// PRINTMPSTG
		DoPrintMPStg();
		break;

	case 8:	// PRINTUCSTG
		DoPrintUCStg();
		break;

	case 9:	// PRINTGRAPHICS
		DoPrintGraphics();
		break;

	case 10: // PRINTDLG
		DoPrintDlg();
		break;

	default:
		//	bad command
		break;
	}

	return IM_RTN_NOTHING;
}

//	======================================================================
//  Test printing a half-page string
//	======================================================================

extern wchar_t szBiDiText[];

void CApplication::DoPrintStg(void)
{
	//	load and print half-page text document
	IStr* hTestText = MakeStrFile(L"%LogicalPgmPath/test.txt");

	hPrMgr->PrStrPrint(hTestText, L"Arial", 50);
}

//	======================================================================
//  Test printing a multi-page string
//	======================================================================

void CApplication::DoPrintMPStg(void)
{
	//	load and print multi-page text document
	IStr* hTestText = MakeStrFile(L"%LogicalPgmPath/testmp.txt");

	hPrMgr->PrStrPrint(hTestText, L"Arial", 50);
}

//	======================================================================
//  Test printing a Unicode string
//	======================================================================

//	Studying the text, it is specified in logical left to right order.
//	Our code is performing no BiDirectional span processing.
//	Perhaps our editors are inserting the RTL characters in the
//	necessary LTR order for rendering. This is wrong, as far as a saved
//	file goes.

extern wchar_t szBiDiText[];

#if 0
void CApplication::DoPrintUCStg(void)
{
	//	print bidirectional text string
	IStr* hTestText = MakeStr(szBiDiText);

	hPrMgr->PrStrPrint(hTestText, L"Arial", 50);
}

#else

wchar_t smallBiDiText[] =
{ L"Say \x05de\x05d6\x05dc \x05d8\x05d5\x05d1 and smile.\n" };

void CApplication::DoPrintUCStg(void)
{
	//	print bidirectional text string
	IStr* hTestText = MakeStr(smallBiDiText);

	hPrMgr->PrStrPrint(hTestText, L"Arial", 50);
}

#endif

//	======================================================================
//  Test printing graphic image
//	======================================================================

void CApplication::DoPrintGraphics(void)
{
	if (!hPrMgr->PrSupported())
		return;

	if (hPrMgr->PrStartJob(L"Print test graphics") == 0)
		return;

	IDraw* hIDraw;
	DEVICE	dev;

	hPrMgr->PrStartPage(&dev, &hIDraw);
	RECT* pPageRect = hPrMgr->PrGetPrintRect();
	DRECTF drect;
	drect.origin.x = pPageRect->left;
	drect.origin.y = pPageRect->top;
	drect.size.width = pPageRect->right - pPageRect->left + 1;
	drect.size.height = pPageRect->bottom - pPageRect->top + 1;

	hIDraw->IDrawClearBkgnd(COLOR_WHITE);

	//	outline page - use rounded rect because it supports line weight
	//	rect, line do not (in CIDraw)
	hIDraw->IDrawSetLWeight(8.0);
	hIDraw->IDrawSetLColor(COLOR_BLACK);
	hIDraw->IDrawSetFColor(COLOR_TRANSPARENT);
	hIDraw->IDrawRRect(pPageRect, 16.0);

	//  draw full page diagonal line
	hIDraw->IDrawSetLColor(COLOR_RED);
	hIDraw->IDrawSetLWeight(4.0);
	hIDraw->IDrawLine(drect.origin.x, drect.origin.y,
		drect.origin.x + drect.size.width - 1,
		drect.origin.y + drect.size.height - 1);

	//	set text font, height, and bearing and color
	double scale = 4.0; // 1.5;
	hIDraw->IDrawSetFont(nullptr, 36.0 * scale, 0.0);
	hIDraw->IDrawSetTColor(COLOR_BLACK);
	hIDraw->IDrawSetFColor(COLOR_TRANSPARENT);

	//	draw some bidirectional text (Arial.ttf has hebrew charset)
	DRECTF trect2 = { {100,100},{600 * scale,400 * scale} };
	hIDraw->IDrawText(szBiDiText, &trect2);

	hPrMgr->PrEndPage(); // will use hIDraw BkImage as page source

	hPrMgr->PrEndJob();
}

//	======================================================================
//  Test printing dialog
//	======================================================================

DLLEXPORT IDialog* MakeDlgPrint(IMObject* hNotify, IDocFile* hDoc);

void CApplication::DoPrintDlg(void)
{
	IDialog* hPrintDlg = MakeDlgPrint((IMObject*)this, nullptr);
}

void* CApplication::OnPrBeginPrint(MSGP)
{
	MPARMPTR(IStr*, hDoc);
	return IM_RTN_NOTHING;
}

//	======================================================================
