//	======================================================================
//  CCtlMainFrame Class Implementation: CCtlFrame                           
//	----------------------------------------------------------------------
//  Copyright ©2012-2021 Evolution Computing. All rights reserved.
//	======================================================================

#include "fwkw.h"

IControl* MakeCtlTitleBar(IControl* hParent,wchar_t* pTitleText);
IControl* MakeCtlContent(IControl* hParent);

#define COLOR_BLUEGREY 0xFF605040
#define COLOR_SOFTBLUE 0xFFE0A080
#define COLOR_TITLEBAR COLOR_BLUEGREY

//	======================================================================
//  Class Definition
//	======================================================================

class CCtlMainFrame : public CControl
{
public:
	CCtlMainFrame(IControl* hParentP,wchar_t* pLabel);
	~CCtlMainFrame(void);
	void* Msg(MSGDISPATCH);
	void* OnKeyDown(MSGP);
	void* OnCtlDraw(MSGP);
	void* OnMouseEvent(MSGP);
public:
public:
	bool		fTranslucent;
	bool		fGrowRight;
	bool		fGrowLeft;
	bool		fGrowBottom;
	bool		fGrowTop;

	POINT		nowat;
	POINT		prior;
	uint32_t	usecsr;

	IControl*	hTitleBar;
	IControl*	hContent;
};

//	======================================================================
//  Class Factory
//	======================================================================

IControl* MakeCtlMainFrame(IControl* hParent,wchar_t* pLabel)
{
	return (IControl*)new CCtlMainFrame(hParent,pLabel);
}

//	======================================================================
//  Constructor
//	======================================================================

CCtlMainFrame::CCtlMainFrame(IControl* hParentP,wchar_t* pLabel)
	: CControl(hParentP,(CTLLOCN*)NULL,0)
{
	fTranslucent = false;
	fGrowRight = fGrowLeft = fGrowTop = fGrowBottom = false;

	IControl* hThis = (IControl*)this; // CCtlMainFrame

	hTitleBar = MakeCtlTitleBar(hThis,pLabel);
	hTitleBar->CtlSetBkgndColor(COLOR_TITLEBAR);

	//	create menu bar
	hMenuBar = MakeMenuBar(hThis, 1, 10);
	((IControl*)hMenuBar)->CtlGradientFill(GF_T2B, 0xFF908070, 0xFF403020);
	((IControl*)hMenuBar)->CtlSetTextColor(COLOR_WHITE);
	((IControl*)hMenuBar)->CtlSetBkgndColor(0xFF605040);
	((IControl*)hMenuBar)->CtlSetFrameType(NOOUTLINE);

	hContent = MakeCtlContent(hThis);
	nBkgndColor = COLOR_LTBLUE;
}

//	======================================================================
//  Destructor
//	======================================================================

CCtlMainFrame::~CCtlMainFrame(void)
{
}

//	======================================================================
//  Message dispatch
//	======================================================================

void* CCtlMainFrame::Msg(MSGDISPATCH)
{
	switch(MSGID)
	{
		case MSG_DbOClass:				return MRTNVAL(FWK_CTRLCLASS);
		case MSG_DbOType:               return MRTNVAL(CTRL_CUSTOM);
		case MSG_AskIsControl:          return IM_RTN_TRUE;
		case MSG_CtlAskTranslucent:		return MRTNVAL(fTranslucent);
		case MSG_CtlSetTranslucent:
		{
			MPARMBOOLV(bool, fTranslucent);
			hWindow->WinNeedsRedraw();
			return IM_RTN_NOTHING;
		}

		case MSG_CtlNeedsRedraw:
		{
			hWindow->WinNeedsRedraw();
			return IM_RTN_FALSE;
		}
		case MSG_CtlDraw:               return OnCtlDraw(MPPTR);
			
		case MSG_CtlMouseEvent:			return OnMouseEvent(MPPTR);
		case MSG_CtlNotifyKeyDown:		return OnKeyDown(MPPTR);

		default:
			return MSGFWDBASE(CControl,MSGID);
	}
	
	return IM_RTN_IGNORED;
}

//	======================================================================
//  A key has been pressed, and this control has the focus
//	======================================================================

void* CCtlMainFrame::OnKeyDown(MSGP)
{
	MPARMINT(wchar_t, key);

	if (key == KEY_ESCAPE)
		hApplication->AppStartShutdown();

	if ((key == 'T') || (key == 't'))
	{
		fTranslucent = !fTranslucent;
		hWindow->WinNeedsRedraw();
	}
	return IM_RTN_TRUE; // key was used
}

//	======================================================================
//  A mouse event has occured
//	----------------------------------------------------------------------
//	TODO: handle edge or full drag crossing monitor boundaries
//	where monitors have different resolutions
//	Can have unexpected large size and position changes, flicker
//	======================================================================

void* CCtlMainFrame::OnMouseEvent(MSGP)
{
	MPARMPTR(MOUSEPKT*, ppkt);
	RECT r;
	POINT delta;
	POINT origin;
	SIZE sz;

	hWinMgr->WMgrSetCursor(CSR_ARROW);

	switch (ppkt->mevent)
	{
	case MOUSE_RBUP:
		//	DEBUG: Quit if right click
		//	If we used MOUSE_RBDOWN, desktop would process the RBUP click...
		hWinMgr->WMgrStartShutdown();
		break;

	case MOUSE_LBDOWN:

		if (usecsr == CSR_ARROW)
			return IM_RTN_TRUE;

		hWindow->WinGetScreenLoc(&r);
		nowat.x = r.left;
		nowat.y = r.top;
		prior.x = ppkt->scrnpt.x;
		prior.y = ppkt->scrnpt.y;
		hWinMgr->WMgrMouseGrab((IControl*)this);
		fTranslucent = true;
		hWindow->WinNeedsRedraw();

		//	down near right or left edge?
		if (ppkt->pt.x > (drect.origin.x + drect.size.width - 20))
			fGrowRight = true;
		if (ppkt->pt.x < (drect.origin.x + 20))
			fGrowLeft = true;

		//	down near top or bottom edge?
		if (ppkt->pt.y > (drect.origin.y + drect.size.height - 20))
			fGrowBottom = true;
		if (ppkt->pt.y < (drect.origin.y + 20))
			fGrowTop = true;

		break;

	case MOUSE_LBTRACKDOWN:

		hWindow->WinGetScreenLoc(&r);
		delta.x = ppkt->scrnpt.x - prior.x;
		delta.y = ppkt->scrnpt.y - prior.y;
		prior.x = ppkt->scrnpt.x;
		prior.y = ppkt->scrnpt.y;

		switch (usecsr)
		{
		case 1: // move top edge
			hWinMgr->WMgrSetCursor(CSR_MOVEV);
			//	Vert resize and reorigin
			sz.cx = r.right - r.left;
			sz.cy = r.bottom - r.top - delta.y;
			origin.x = r.left;
			origin.y = r.top + delta.y;
			hWindow->WinSetOriginAndSize(&origin, &sz);
			break;

		case 2: // move right edge
			hWinMgr->WMgrSetCursor(CSR_MOVEH);
			//	Hz resize - grow right
			sz.cx = r.right - r.left + delta.x;
			sz.cy = r.bottom - r.top;
			hWindow->WinResize(&sz);
			break;

		case 3: // move top and right edge
			sz.cx = r.right - r.left + delta.x;
			sz.cy = r.bottom - r.top - delta.y;
			origin.x = r.left;
			origin.y = r.top + delta.y;
			hWindow->WinSetOriginAndSize(&origin, &sz);
			break;

		case 4: // move bottom edge
			hWinMgr->WMgrSetCursor(CSR_MOVEV);
			//	Vert resize - grow bottom
			sz.cx = r.right - r.left;
			sz.cy = r.bottom - r.top + delta.y;
			hWindow->WinResize(&sz);
			break;

		case 6: // move bottom and right edge
			sz.cx = r.right - r.left + delta.x;
			sz.cy = r.bottom - r.top + delta.y;
			hWindow->WinResize(&sz);
			break;

		case 8: // move left edge
			hWinMgr->WMgrSetCursor(CSR_MOVEH);
			//	Hz resize and reorigin - grow left
			sz.cx = r.right - r.left - delta.x;
			sz.cy = r.bottom - r.top;
			origin.x = r.left + delta.x;
			origin.y = r.top;
			hWindow->WinSetOriginAndSize(&origin, &sz);
			break;

		case 9: // move top and left edge
			sz.cx = r.right - r.left - delta.x;
			sz.cy = r.bottom - r.top - delta.y;
			origin.x = r.left + delta.x;
			origin.y = r.top + delta.y;
			hWindow->WinSetOriginAndSize(&origin, &sz);
			break;

		case 12: // move bottom and left edge
			sz.cx = r.right - r.left - delta.x;
			sz.cy = r.bottom - r.top + delta.y;
			origin.x = r.left + delta.x;
			origin.y = r.top;
			hWindow->WinSetOriginAndSize(&origin, &sz);
			break;

		default: // will be 13 = CSR_ARROW

			//	we do not grab mouse event for full drag
			if (usecsr == CSR_ARROW)
				return IM_RTN_TRUE;

			hWinMgr->WMgrSetCursor(CSR_4WAY);
			//	drag window with no size changes
			//	move ruler as sized and oriented
			nowat.x += delta.x;
			nowat.y += delta.y;
			hWindow->WinMove(&nowat);
		}
		break;

	case MOUSE_TRACKUP:
		usecsr = 0;
		if (ppkt->pt.x > (drect.origin.x + drect.size.width - 10))
			usecsr = 2;
		if (ppkt->pt.x < (drect.origin.x + 10))
			usecsr = 8;

		//	down near top or bottom edge?
		if (ppkt->pt.y > (drect.origin.y + drect.size.height - 10))
			usecsr += 4;
		if (ppkt->pt.y < (drect.origin.y + 10))
			usecsr += 1;

		if (usecsr == 0)
			usecsr = CSR_ARROW;
		hWinMgr->WMgrSetCursor((STDCURSOR)usecsr);

		if (usecsr == CSR_ARROW)
			return IM_RTN_TRUE;

		break;

	case MOUSE_LBUP:
		fGrowRight = fGrowLeft = fGrowTop = fGrowBottom = false;
		hWinMgr->WMgrMouseGrab(nullptr);
		fTranslucent = false;
		hWindow->WinNeedsRedraw();

		if (usecsr == CSR_ARROW)
			return IM_RTN_TRUE;

		break;
			
	default:
		if (usecsr == CSR_ARROW)
			return IM_RTN_TRUE;
		break;
	}

	return IM_RTN_NOTHING;
}

//	======================================================================
//  This control needs its content drawn
//	======================================================================

void* CCtlMainFrame::OnCtlDraw(MSGP)
{
	MPARMPTR(IDraw*,hIDrawP);
	hIDraw = hIDrawP;

	uint32_t nBkgndTranslucent = (nBkgndColor & 0x00FFFFFF) + 0xC0000000;
	uint32_t nBkColor = fTranslucent ? nBkgndTranslucent : nBkgndColor;
	hIDraw->IDrawSetAlphaMode(ABLEND_BLENDED);

	//	get control bounds
	hWindow->CtlGetDRect(&drect);

	//	fills entire window
	hIDraw->IDrawSetLColor(COLOR_TRANSPARENT);
	hIDraw->IDrawClearBkgnd(nBkColor);

	//	draw any child controls
	((IControl*)this)->CtlDrawSL(hIDraw);
	return IM_RTN_NOTHING;
}

//	======================================================================
