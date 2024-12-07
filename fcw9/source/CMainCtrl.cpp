//	======================================================================
//	CMainCtrl.cpp - Main Window Custom Control
//	----------------------------------------------------------------------
//	Copyright ©2012-2024 Evolution Computing, Inc.
//	All rights reserved
//	======================================================================

#include "fwkw.h"
#include "rscid.h"

//	======================================================================
//	Class definition
//	======================================================================

class CMainCtrl : public CControl
{
public:
	CMainCtrl(IControl* hParent,CTLLOCN* pLocn);
	~CMainCtrl(void);
	virtual void* Msg(MSGDISPATCH);
	void* OnKey(MSGP);
	void* OnDraw(MSGP);
	void* OnMouseEvent(MSGP);
public:
//	IImage*		hPortrait;
//	IStr*		hPortraitName;
//	uint32_t	nNowFramed;
//	RECT		frame[32];
};

//	======================================================================
//	Class factory
//	======================================================================

IControl* MakeMainCtrl(IControl* hParent, CTLLOCN* pLocn)
{
	return (IControl*) new CMainCtrl(hParent, pLocn);
}

//	======================================================================
//	Constructor
//	======================================================================

static uint32_t ctrlid = 100;

CMainCtrl::CMainCtrl(IControl* hparent, CTLLOCN* pLocn)
	:CControl(hparent, (CTLLOCN*)pLocn,0)
{
	nBkgndColor = COLOR_LTBLUE;
//	hPortrait = MakeImage(TEXT("%LogicalPgmPath/rsc/Mike.png"));
//	hPortraitName = MakeStr(TEXT("Mike"));
//	nNowFramed = 0;
}

//	======================================================================
//	Destructor
//	======================================================================

CMainCtrl::~CMainCtrl(void)
{
//	if(hPortrait)
//		hPortrait->Release();
}

//	======================================================================
//	Variable length message receiver/dispatcher
//	======================================================================

void* CMainCtrl::Msg(MSGDISPATCH)
{
	switch(MSGID)
	{
	case MSG_DbOClass:			return MRTNVAL(FWK_CTRLCLASS);
	case MSG_DbOType:           return MRTNVAL(CTRL_CONTAINER);
	case MSG_CtlDraw:			return OnDraw(MPPTR);
	case MSG_CtlMouseEvent:		return OnMouseEvent(MPPTR);
	case MSG_CtlNotifyKeyDown:	return OnKey(MPPTR);
	default:
		return MSGFWDBASE(CControl,MSGID);
	};
}

//	======================================================================
//	OnDraw - draw contents
//	======================================================================

//	bidirectional body text containing some Hebrew Unicode
//	type "nzk yuc" to enter mazel tov using hebrew keyboard IME
wchar_t szBiDiText[] = L"Arabic joiners missing from CIDraw, ok in GDI+ and DX2d:\n"
L"Say \x05de\x05d6\x05dc \x05d8\x05d5\x05d1 and smile.\n"
L" Arabic \x064A\x064F\x0633\x0627\x0648\x0650\x064a text.\n"
L"\nCorrect Hebrew display is: Say \x05de \x05d6 \x05dc \x05d8 \x05d5 \x05d1 and smile.\n";
//  Correct Hebrew display is: Say מזל טוב and smile.
//  OSX NDrawMacText.h and iOS NDrawiOSText.h display mixed text correctly

void* CMainCtrl::OnDraw(MSGP)
{
	MPARMPTR(IDraw*, hIDrawP);
	hIDraw = hIDrawP;

//	----------------------------------------------------------------------

	//	background fill image
//	IImage* hBkgnd = RscImage(RSC_LinenBkgnd);
//	if(hBkgnd)
//		hIDraw->IDrawTileFillRect(hBkgnd,&CRect);
//	else
//	{
//		hIDraw->IDrawSetFColor(nBkgndColor);
//		hIDraw->IDrawSetLColor(COLOR_BLACK);
//		hIDraw->IDrawRectP(&CRect);
//	}

	//	A CWindow frame is 3 pixels: Black, then 2 frame color rects
	//	the CWindow frame draw does NOT draw a 4th inner pixel frame
	//	In most programs, that is the CContainer control's frame
	//	CWindow frames are drawn by hWinMgr->WMgrDrawFrame()
	//	as a part of refreshing the CWindow to the screen

	//	fill our control with BLACK
	hIDraw->IDrawSetFColor(COLOR_BLACK);
	hIDraw->IDrawSetLColor(COLOR_TRANSPARENT);
	hIDraw->IDrawRectP(&CRect);

	//  draw full CRect diagonal line
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
	hSvc->SvcStrDateTime(hTimeText,&now,0);

	hIDraw->IDrawSetTColor(COLOR_WHITE);
	hIDraw->IDrawSetFont(NULL,16.0,0.0);
	hIDraw->IDrawTextAt(hTimeText->StrTextPtr(),CRect.left+10,CRect.top+23);
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
	DRECTF trect2 = { {50,150},{900,400} };
	hIDraw->IDrawText(szBiDiText, &trect2);

	//	draw an image of correctly rendered Arabic text
	hIDraw->IDrawTextAt(L"Correct Arabic display is: ", 50, 350);
	IImage* hArabic = RscImage(RSCIMG_Arabic);
	hIDraw->IDrawImageAt(hArabic, 290, 324, 1.0, false);

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

	DRECTF trect3 = { {50,400},{600,500} };
	hIDraw->IDrawText(hSzScale->StrTextPtr(), &trect3);

//	----------------------------------------------------------------------

	//	Notify all immediate children to draw
	IControl* hCtrl = (IControl*)DbFirstSL();
	while (hCtrl && (hCtrl != (IControl*)hSLEnd))
	{
		hCtrl->CtlDraw(hIDraw);
		hCtrl = (IControl*)hCtrl->DbNextMain();
	}

	return IM_RTN_NOTHING;
}

//	======================================================================
//	OnMouse - process mouse event
//	======================================================================

void* CMainCtrl::OnMouseEvent(MSGP)
{
	MPARMPTR(MOUSEPKT*,ppkt);

	hWinMgr->WMgrSetCursor(CSR_ARROW);

	switch(ppkt->mevent)
	{
	case MOUSE_LBDOWN:
		//	which picture is the mouse over?
/*
		for(uint32_t ix=0;ix<6;ix++)
		{
			if((ppkt->pt.x >= frame[ix].left)
			&&(ppkt->pt.x <= frame[ix].right)
			&&(ppkt->pt.y >= frame[ix].top)
			&&(ppkt->pt.y <= frame[ix].bottom))
			{
				nNowFramed = ix;
				((IControl*)this)->CtlNeedsRedraw();
			}
		}
*/
		break;
	}

	return IM_RTN_NOTHING;
}

//	======================================================================
//	OnKey - process keypress
//	======================================================================

void* CMainCtrl::OnKey(MSGP)
{
	MPARMINT(wchar_t,key);

	if(key == KEY_ESCAPE)
	{
		//	end the program when the escape key is pressed
		hApplication->AppStartShutdown();
		return IM_RTN_TRUE;	// keypress was used
	}

	return (void*)IM_RTN_FALSE; // we did not use the keypress
}

//	======================================================================

