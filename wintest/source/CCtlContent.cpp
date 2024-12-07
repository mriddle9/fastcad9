//	======================================================================
//  CCtlMainFrame Class Implementation: CCtlFrame                           
//	----------------------------------------------------------------------
//  Copyright ©2012-2021 Evolution Computing. All rights reserved.
//	======================================================================

#include "fwkw.h"
#include "rscid.h"

//	======================================================================
//  Class Definition
//	======================================================================

class CCtlContent : public CControl
{
public:
	CCtlContent(IControl* hParentP);
	~CCtlContent(void);
	void* Msg(MSGDISPATCH);
	void* OnLocate(MSGP);
	void* OnKeyDown(MSGP);
	void* OnCtlDraw(MSGP);
	void* OnMouseEvent(MSGP);
public:
public:
	IControl* hParent;
};

//	======================================================================
//  Class Factory
//	======================================================================

IControl* MakeCtlContent(IControl* hParent)
{
	return (IControl*)new CCtlContent(hParent);
}

//	======================================================================
//  Constructor
//	======================================================================

static CTLLOCN ctrllocn = { CLT_ALL,0,0,0,0 };

CCtlContent::CCtlContent(IControl* hParentP)
	: CControl(hParentP,&ctrllocn,0)
{
	hParent = hParentP;
}

//	======================================================================
//  Destructor
//	======================================================================

CCtlContent::~CCtlContent(void)
{
}

//	======================================================================
//  Message dispatch
//	======================================================================

void* CCtlContent::Msg(MSGDISPATCH)
{
	switch(MSGID)
	{
		case MSG_DbOClass:					return MRTNVAL(FWK_CTRLCLASS);
		case MSG_DbOType:                   return MRTNVAL(CTRL_CUSTOM);
		case MSG_AskIsControl:              return IM_RTN_TRUE;

		case MSG_CtlLocate:					return OnLocate(MPPTR);
		case MSG_CtlNeedsRedraw:
		{
			hWindow->WinNeedsRedraw();
			return IM_RTN_FALSE;
		}
		case MSG_CtlDraw:                   return OnCtlDraw(MPPTR);
			
		case MSG_CtlMouseEvent:				return OnMouseEvent(MPPTR);
		case MSG_CtlNotifyKeyDown:			return OnKeyDown(MPPTR);
			
		default:
			return MSGFWDBASE(CControl,MSGID);
	}
	
	return IM_RTN_IGNORED;
}

//	======================================================================
//	OnLocate - locate control within parent's allocation rectangle
//	======================================================================

void* CCtlContent::OnLocate(MSGP)
{
	MPARMPTR(RECT*, pARect);

	MSGFWDBASE(CControl, MSG_CtlLocate);

	return IM_RTN_NOTHING;
}

//	======================================================================
//  A key has been pressed, and this control has the focus
//	======================================================================

void* CCtlContent::OnKeyDown(MSGP)
{
	MPARMINT(wchar_t, key);

	if (key == KEY_ESCAPE)
		hApplication->AppStartShutdown();

	return IM_RTN_TRUE; // key was used
}

//	======================================================================
//  A mouse event has occured
//	======================================================================

void* CCtlContent::OnMouseEvent(MSGP)
{
	MPARMPTR(MOUSEPKT*, ppkt);

	//	see if parent wants to process mouse event
	//	(edge grow / shrink)
	if (!hParent->CtlMouseEvent(ppkt))
		return IM_RTN_NOTHING;

	hWinMgr->WMgrSetCursor(CSR_ARROW);

	switch (ppkt->mevent)
	{
	case MOUSE_RBUP:
		break;

	case MOUSE_LBDOWN:
		break;

	case MOUSE_LBTRACKDOWN:
		break;

	case MOUSE_TRACKUP:
		break;

	case MOUSE_LBUP:
		break;
			
	default:
		break;
	}

	return IM_RTN_NOTHING;
}

//	======================================================================
//  This control needs its content drawn
//	======================================================================

//	bidirectional body text containing some Hebrew Unicode
//	type "nzk yuc" to enter mazel tov using hebrew keyboard IME
wchar_t szBiDiText[] = L"Arabic joiners missing from CIDraw, ok in GDI+ and DX2d:\n"
L"Say \x05de\x05d6\x05dc \x05d8\x05d5\x05d1 and smile.\n"
L" Arabic \x064A\x064F\x0633\x0627\x0648\x0650\x064a text.\n"
L"\nCorrect Hebrew display is: Say \x05de \x05d6 \x05dc \x05d8 \x05d5 \x05d1 and smile.\n";
//  Correct Hebrew display is: Say מזל טוב and smile.
//  OSX NDrawMacText.h and iOS NDrawiOSText.h display mixed text correctly

void* CCtlContent::OnCtlDraw(MSGP)
{
	MPARMPTR(IDraw*,hIDrawP);
	hIDraw = hIDrawP;

	//	default background is parent's background
	nBkgndColor = hParent->CtlGetBkgndColor();

	//	determine fill color
	bool fTranslucent = hParent->CtlAskTranslucent();
	uint32_t nBkgndTranslucent = (nBkgndColor & 0x00FFFFFF) + 0x40000000;
	uint32_t nBkColor = fTranslucent ? nBkgndTranslucent : nBkgndColor;
	hIDraw->IDrawSetAlphaMode(ABLEND_BLENDED);

	//	fill background
	hIDraw->IDrawSetLColor(COLOR_TRANSPARENT);
	hIDraw->IDrawSetFColor(nBkColor);
	hIDraw->IDrawRectPF(&drect);

	//  draw full screen diagonal line
	hIDraw->IDrawSetLColor(COLOR_RED);
	hIDraw->IDrawLine(drect.origin.x, drect.origin.y,
		drect.origin.x + drect.size.width - 1,
		drect.origin.y + drect.size.height - 1);

//	----------------------------------------------------------------------

	//	show current time and date
	TIME now;
	IStr* hTimeText = MakeStr();
	//	this returns UTC time
	hSystem->SysTime(&now);
	//	this converts UTC to Local Time before formatting:
	hSvc->SvcStrDateTime(hTimeText, &now, 0);

	hIDraw->IDrawSetTColor(COLOR_WHITE);
	hIDraw->IDrawSetFont(NULL, 16.0, 0.0);
	hIDraw->IDrawTextAt(hTimeText->StrTextPtr(), CRect.left + 10, CRect.top + 23);
	hTimeText->Release();

//	----------------------------------------------------------------------

	//	Type of IDraw in use
	wchar_t* pText = hIDraw->IDrawId();
	hIDraw->IDrawTextAt(L"IDraw type:", CRect.left + 10, CRect.top + 50);
	hIDraw->IDrawTextAt(pText, CRect.left + 100, CRect.top + 50);

//	----------------------------------------------------------------------

	//	set text font, height, bearing and color
	hIDraw->IDrawSetFont(nullptr, 20.0, 0.0);
	hIDraw->IDrawSetTColor(COLOR_WHITE);
	hIDraw->IDrawSetFColor(COLOR_TRANSPARENT);

	//	draw some bidirectional text (Arial.ttf has hebrew charset)
	DRECTF trect2 = { {50,120},{900,230} };
	hIDraw->IDrawText(szBiDiText, &trect2);

	//	draw an image of correctly rendered Arabic text
	hIDraw->IDrawTextAt(L"Correct Arabic display is: ", 50, 280);
	IImage* hArabic = RscImage(RSCIMG_Arabic);
	hIDraw->IDrawImageAt(hArabic, 270, 260, 1.0, false);

//	----------------------------------------------------------------------

		//	report print info

		//	screen scaling (xppi/96)
	double wscale = 1.0;
	hWindow->WinDPIScale(&wscale);
	IStr* hSzScale = MakeStr();

	//	get printer information
	wchar_t szInfo[] =
		L"HiDPI scale = %1\n"
		L"Default printer = \n %2\n"
		L"Selected printer = \n  %3\n"
		L"width = %4, height = %5\n"
		L"Xppi = %6, Yppi = %7"
		L"\0";

	IStr* hDfltPrName = hPrMgr->PrDfltPrinter();
	if (!hDfltPrName)
		hDfltPrName = MakeStr("(undefined)");

	//	this messes up CIDrawDX2D IDraw:
	//	pRenderText becomes nullptr, still same IDraw object
	PRGETINFO* pPrInfo = hPrMgr->PrGetInfo();
	//	added this to recreate pRenderTarget...
	hIDraw->IDrawReady();

	if (!pPrInfo->hPrName)
		pPrInfo->hPrName = MakeStr("(undefined)");

	//	format print info
	StrFormat(hSzScale, L"VRRNNNN", szInfo,
		wscale, hDfltPrName, pPrInfo->hPrName,
		pPrInfo->width, pPrInfo->height,
		pPrInfo->xppi, pPrInfo->yppi);

	//	report print info
	hIDraw->IDrawSetFont(nullptr, 18.0, 0.0);
	hIDraw->IDrawSetTColor(COLOR_WHITE);
	hIDraw->IDrawSetFColor(COLOR_TRANSPARENT);

	DRECTF trect3 = { {50,300},{600,400} };
	hIDraw->IDrawText(hSzScale->StrTextPtr(), &trect3);

//	----------------------------------------------------------------------
	//	draw any child controls
	((IControl*)this)->CtlDrawSL(hIDraw);
	return IM_RTN_NOTHING;
}

//	======================================================================
