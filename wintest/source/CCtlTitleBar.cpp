//	======================================================================
//  CCtlTitleBar Class Implementation: CCtlFrame                           
//	----------------------------------------------------------------------
//  Copyright ©2012-2021 Evolution Computing. All rights reserved.
//	======================================================================

#include "fwkw.h"

DLLEXPORT IDialog* MakeDlgAbout(void);

//	======================================================================
//  Class Definition
//	======================================================================

class CCtlTitleBar : public CControl
{
public:
	CCtlTitleBar(IControl* hParentP, wchar_t* pTitleText);
	~CCtlTitleBar(void);
	void* Msg(MSGDISPATCH);
	void* OnLocate(MSGP);
	void* OnKeyDown(MSGP);
	void* OnCtlDraw(MSGP);
	void* OnMouseEvent(MSGP);
public:
public:
	IControl*	hParent;
	IImage*		hClose;
	IImage*		hSysMenu;
	IImage*		hDoMax;
	IImage*		hDoUnMax;
	IImage*		hMinimize;
	IStr*		hWinTitle;

	bool		fTranslucent;
	POINT		nowat;
	POINT		prior;

	RECT		RClose;
	int32_t		RCloseX;
	RECT		RSysMenu;
	int32_t		RSysMenuX;
	RECT		RMax;
	int32_t		RMaxX;
	RECT		RMin;
	int32_t		RMinX;
	RECT		RTitle;
	int32_t		RTitleX;
	int32_t		RTitleY;
};

//	======================================================================
//  Class Factory
//	======================================================================

IControl* MakeCtlTitleBar(IControl* hParent,wchar_t* pTitleText)
{
	return (IControl*)new CCtlTitleBar(hParent,pTitleText);
}

//	======================================================================
//  Constructor
//	======================================================================

static CTLLOCN ctrllocn = { CLT_TOP,0,0,0,30 };

CCtlTitleBar::CCtlTitleBar(IControl* hParentP, wchar_t* pTitleText)
	: CControl(hParentP,&ctrllocn,0)
{
	hParent = hParentP;
	hWinTitle = MakeStr(pTitleText);
	hClose = RscImage(RSCIMG_Close);
	hSysMenu = RscImage(RSCIMG_SysMenu);
	hDoMax = RscImage(RSCIMG_DoMax);
	hDoUnMax = RscImage(RSCIMG_DoUnMax);
	hMinimize = RscImage(RSCIMG_Minimize);
	fTranslucent = false;
}

//	======================================================================
//  Destructor
//	======================================================================

CCtlTitleBar::~CCtlTitleBar(void)
{
}

//	======================================================================
//  Message dispatch
//	======================================================================

void* CCtlTitleBar::Msg(MSGDISPATCH)
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

void* CCtlTitleBar::OnLocate(MSGP)
{
	MPARMPTR(RECT*, pARect);

	MSGFWDBASE(CControl, MSG_CtlLocate);

	return IM_RTN_NOTHING;
}

//	======================================================================
//  A key has been pressed, and this control has the focus
//	======================================================================

void* CCtlTitleBar::OnKeyDown(MSGP)
{
	MPARMINT(wchar_t, key);

	if (key == KEY_ESCAPE)
		hApplication->AppStartShutdown();

	return IM_RTN_TRUE; // key was used
}

//	======================================================================
//  A mouse event has occured
//	----------------------------------------------------------------------
//	TODO: handle edge or full drag crossing monitor boundaries
//	where monitors have different resolutions
//	Can have unexpected large size and position changes, flicker
//	======================================================================

void* CCtlTitleBar::OnMouseEvent(MSGP)
{
	MPARMPTR(MOUSEPKT*, ppkt);

	//	see if parent wants to process mouse event
	//	(edge grow / shrink)
	if (!hParent->CtlMouseEvent(ppkt))
		return IM_RTN_NOTHING;

	hWinMgr->WMgrSetCursor(CSR_ARROW);

	RECT r;
	POINT delta;

	switch (ppkt->mevent)
	{
	case MOUSE_RBUP:
		break;

	case MOUSE_LBDOWN:
		if (hSvc->SvcInsideRect(ppkt->pt.x, ppkt->pt.y, &RTitle))
		{
			//	start titlebar drag
			hWindow->WinGetScreenLoc(&r);
			nowat.x = r.left;
			nowat.y = r.top;
			prior.x = ppkt->scrnpt.x;
			prior.y = ppkt->scrnpt.y;
			hWinMgr->WMgrMouseGrab((IControl*)this);
			fTranslucent = true;
			hParent->CtlSetTranslucent(fTranslucent);
			hWindow->WinNeedsRedraw();
		}
		break;

	case MOUSE_LBTRACKDOWN:
		if (fTranslucent)
		{
			//	doing titlebar drag
			hWindow->WinGetScreenLoc(&r);
			delta.x = ppkt->scrnpt.x - prior.x;
			delta.y = ppkt->scrnpt.y - prior.y;
			prior.x = ppkt->scrnpt.x;
			prior.y = ppkt->scrnpt.y;
			hWinMgr->WMgrSetCursor(CSR_4WAY);
			//	drag window with no size changes
			//	move ruler as sized and oriented
			nowat.x += delta.x;
			nowat.y += delta.y;
			hWindow->WinMove(&nowat);
		}
		break;

	case MOUSE_TRACKUP:
		break;

	case MOUSE_LBUP:
		if (fTranslucent)
		{
			//	doing titlebar drag
			fTranslucent = false;
			hParent->CtlSetTranslucent(fTranslucent);
			break;
		}

		if (hSvc->SvcInsideRect(ppkt->pt.x, ppkt->pt.y, &RClose))
		{
			hWindow->WinClose();
			break;
		}
		if (hSvc->SvcInsideRect(ppkt->pt.x, ppkt->pt.y, &RMax))
		{
			if (hWindow->WinAskMaximized())
				hWindow->WinUnMaximize();
			else
				hWindow->WinMaximize();
			break;
		}
		if (hSvc->SvcInsideRect(ppkt->pt.x, ppkt->pt.y, &RMin))
		{
			hWindow->WinMinimize();
			break;
		}
		if (hSvc->SvcInsideRect(ppkt->pt.x, ppkt->pt.y, &RSysMenu))
		{
			MakeDlgAbout();
//			hWindow->WinSysMenu();
			break;
		}
		break;

	default:
		break;
	}

	return IM_RTN_NOTHING;
}

//	======================================================================
//  This control needs its content drawn
//	======================================================================

void* CCtlTitleBar::OnCtlDraw(MSGP)
{
	MPARMPTR(IDraw*, hIDrawP);
	hIDraw = hIDrawP;

	//	calculate icon pick locations
	RClose = CRect;
	RClose.left = CRect.right - 40;
	RClose.right = RClose.left + 20;
	RCloseX = (RClose.left + RClose.right) / 2;

	RSysMenu = CRect;
	RSysMenu.left = CRect.left + 20;
	RSysMenu.right = RSysMenu.left + 20;
	RSysMenuX = (RSysMenu.left + RSysMenu.right) / 2;

	RMax = CRect;
	RMax.right = RClose.left - 20;
	RMax.left = RMax.right - 20;
	RMaxX = (RMax.left + RMax.right) / 2;

	RMin = CRect;
	RMin.right = RMax.left - 20;
	RMin.left = RMin.right - 20;
	RMinX = (RMin.left + RMin.right) / 2;

	RTitle = CRect;
	RTitle.left = RSysMenu.right + 10;
	RTitle.right = RMin.left - 20;
	RTitleX = RTitle.left + 6;
	RTitleY = RTitle.bottom - 9;

	//	determine fill color
	bool fTranslucent = hParent->CtlAskTranslucent();
	uint32_t nBkgndTranslucent = (nBkgndColor & 0x00FFFFFF) + 0x40000000;
	uint32_t nBkColor = fTranslucent ? nBkgndTranslucent : nBkgndColor;
	hIDraw->IDrawSetAlphaMode(ABLEND_BLENDED);

	//	fill background
	hIDraw->IDrawSetLColor(COLOR_TRANSPARENT);
	hIDraw->IDrawSetFColor(nBkColor);
	hIDraw->IDrawRectPF(&drect);

	//	fill background
	hIDraw->IDrawSetFColor(nBkColor);
	hIDraw->IDrawRectPF(&drect);

	//	draw title bar icons
	int32_t icony = (CRect.bottom - CRect.top + 1) / 2 + CRect.top;

	hIDraw->IDrawImageAt(hSysMenu,RSysMenuX,icony, 1.0,true);
	hIDraw->IDrawImageAt(hClose,RCloseX, icony, 1.0,true);
	hIDraw->IDrawImageAt(hMinimize, RMinX, icony, 1.0, true);

	if(hWindow->WinAskMaximized())
		hIDraw->IDrawImageAt(hDoUnMax, RMaxX, icony, 1.0, true);
	else
		hIDraw->IDrawImageAt(hDoMax, RMaxX, icony, 1.0, true);

	hIDraw->IDrawSetFont(NULL, 16.0, 0.0);
	hIDraw->IDrawSetTColor(COLOR_WHITE);
	hIDraw->IDrawTextAt(hWinTitle->Text(),RTitleX,RTitleY);

	//	draw any child controls
	((IControl*)this)->CtlDrawSL(hIDraw);
	return IM_RTN_NOTHING;
}

//	======================================================================
