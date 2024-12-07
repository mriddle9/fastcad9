 //	==============================================================================
//	CCustomCtl Class Implementation
//	------------------------------------------------------------------------------
//	Copyright ©2012-2016 Evolution Computing. All rights reserved.
//	==============================================================================

#include "fwkw.h"

//	This selects for testing with either a direct system timer or the time manager
#define USE_TIMEMGR	0
#define TMTIMER_TEST	99

//	==============================================================================
//	Class Definition
//	==============================================================================

enum TESTTYPE{TEST_MOVE_WINDOW=0,TEST_RESIZE_WINDOW=1,TEST_DRAW_WINDOW=2};

class CCustomCtl : public CControl
{
public:
	CCustomCtl(IControl* hParentP, wchar_t* pLabel);
	~CCustomCtl(void);
	void* Msg(MSGDISPATCH);
	void* OnKeyDown(MSGP);
	void* OnMouseEvent(MSGP);
	void* OnCtlDraw(MSGP);
	void* OnTimeOut(MSGP);

	void* OnCtlMultiTouch(MSGP);
public:
	void ToggleAnimation(void);
public:
	uint32_t        wstyleflags;
	IStr*       hLabel;

	//	used in animation testing

#if USE_TIMEMGR
	uint32_t		animationTimer;
#else
	TIMER		animationTimer;
#endif

	double		progress;
	double		sign;
	bool		fTimer;
	TESTTYPE	nTestType;

	//	used in processing mouse points

	POINT		priorpt;
	IDraw*		hIDraw;
	IWindow*	hPrWin;

	//	used in processing touch points

	POINT		orgDelta;
	POINT		priorPt;
};

//	==============================================================================
//  Class Factory
//	==============================================================================

IControl* MakeCustomCtl(IControl* hParent,wchar_t* pLabel)
{
	return (IControl*)new CCustomCtl(hParent,pLabel);
}

//	==============================================================================
//  Constructor
//	==============================================================================

CCustomCtl::CCustomCtl(IControl* hParentP,wchar_t* pLabel)
	: CControl(hParentP,(CTLLOCN*)NULL,0)
{
	hLabel = MakeStr(pLabel);
	progress = 0.0;
	sign = 1.0;
	fTimer = false;
	animationTimer = NULL;
	nTestType = TEST_DRAW_WINDOW;
	wstyleflags = ((IWindow*)hParent)->WinStyleFlags();
	
	hWinMgr->WMgrChangeFocus((IControl*)this);
}

//	==============================================================================
//  Destructor
//	==============================================================================

CCustomCtl::~CCustomCtl(void)
{
	hLabel->Release();
}

//	==============================================================================
//  Message dispatch
//	==============================================================================

void* CCustomCtl::Msg(MSGDISPATCH)
{
	switch(MSGID)
	{
		case MSG_DbOClass:					return MRTNVAL(FWK_CTRLCLASS);
		case MSG_DbOType:                   return MRTNVAL(CTRL_CUSTOM);
		case MSG_AskIsControl:              return IM_RTN_TRUE;

		case MSG_CtlNeedsRedraw:
		{
			hWindow->WinNeedsRedraw();
			return IM_RTN_FALSE;
		}
		case MSG_CtlNotifyKeyDown:			return OnKeyDown(MPPTR);
		case MSG_CtlMouseEvent:				return OnMouseEvent(MPPTR);
		case MSG_CtlDraw:                   return OnCtlDraw(MPPTR);
		case MSG_CtlTimeOut:				return OnTimeOut(MPPTR);
		
		case MSG_CtlMultiTouch:             return OnCtlMultiTouch(MPPTR);
		default:
			return MSGFWDBASE(CControl,MSGID);
	}
	
	return IM_RTN_IGNORED;
}

//	==============================================================================
//  A key has been pressed, and no control has the focus
//	==============================================================================

void* CCustomCtl::OnKeyDown(MSGP)
{
	MPARMINT(wchar_t,key);
	
	//	DEBUG: Quit if ESCAPE key is pressed
	if(key == KEY_ESCAPE)
		hWinMgr->WMgrStartShutdown();
	
	//	DEBUG: P key triggers print document message
	if((key == 'P')||(key=='p'))
	{
		IStr* hName = MakeStr(TEXT("printTest.doc"));
		hApplication->AppPrintNamedDocument(hName);
	}
	
	//	DEBUG: O key triggers open document message
	if((key == 'O')||(key=='o'))
	{
		IStr* hName = MakeStr(TEXT("openTest.doc"));
		hApplication->AppOpenNamedDocument(hName);
	}
	
	//	DEBUG: A key toggles animation
	if((key == 'A')||(key=='a'))
	{
		ToggleAnimation();
	}
	
	//	DEBUG: Q key quits the application
	if((key == 'Q')||(key=='q'))
	{
		hWinMgr->WMgrStartShutdown();
	}
	
	//	DEBUG: C key closes the window that has focus
	if((key == 'C')||(key=='c'))
	{
		IWindow* hW = (IWindow*)hFocusWin;
		if(hFocusWin)
			((IWindow*)hFocusWin)->WinClose();
	}
	
	//	DEBUG: M key sets move window test
	if((key == 'M')||(key=='m'))
	{
		nTestType = TEST_MOVE_WINDOW;
	}
	
	//	DEBUG: R key sets resize window test
	if((key == 'R')||(key=='r'))
	{
		nTestType = TEST_RESIZE_WINDOW;
	}
	
	//	DEBUG: D key sets draw in window test
	if((key == 'D')||(key=='d'))
	{
		nTestType = TEST_DRAW_WINDOW;
	}
	
	return IM_RTN_TRUE; // key was used
}

//	==============================================================================
//	Animation Timer callback
//	==============================================================================

#define REFRESH_RATE    40.0

void CCustomCtl::ToggleAnimation(void)
{
	if(animationTimer)
	{
#if USE_TIMEMGR
		hTimeMgr->TMgrRmvTimer(animationTimer);
		animationTimer = 0;
#else
		hSystem->SysStopTimer(animationTimer);
#endif
		animationTimer = NULL;
	}
	
	fTimer = !fTimer;
	if(fTimer)
	{
#if USE_TIMEMGR
		uint32_t msec = uint32_t(1.0 / REFRESH_RATE * 1000.0);
		animationTimer = hTimeMgr->TMgrAddTimer(this, MSG_CtlTimeOut,msec, TMTIMER_TEST);
#else
		double sec = 1.0 / REFRESH_RATE;
		animationTimer = // use this variable to kill the timer explicitly
		hSystem->SysStartTimer((IControl*)this,MSG_CtlTimeOut,sec,true,NULL);
#endif
	}
}

void* CCustomCtl::OnTimeOut(MSGP)
{
	MPARMPTR(void*,pUData);

	progress += 1.0 / REFRESH_RATE * sign;
	
	if(progress > 1.001)
	{
		sign = -1.0;
		progress = 1.0;
	}

	if(progress < 0.0)
	{
		sign = 1.0;
		progress = 0.0;
	}

	((IControl*)this)->CtlNeedsRedraw();

	//	do not cancel the timer
	return IM_RTN_TRUE;
}

//	==============================================================================
//  A mouse event has occured, and no control is active
//	==============================================================================

static int deltax,deltay;
static POINT downOrgPt;

void* CCustomCtl::OnMouseEvent(MSGP)
{
	MPARMPTR(MOUSEPKT*,ppkt);

	switch(ppkt->mevent)
	{
		case MOUSE_RBDOWN:
			//	DEBUG: Quit if right click
			hWinMgr->WMgrStartShutdown();
			break;
			
		case MOUSE_LBDOWN:
		priorpt = ppkt->scrnpt;
		ppkt->hWindow->WinGetOrigin(&downOrgPt);
		deltax = ppkt->scrnpt.x - downOrgPt.x;
		deltay = ppkt->scrnpt.y - downOrgPt.y;
			hIDraw = ppkt->hWindow->CtlGetIDraw();
			break;
			
		case MOUSE_LBTRACKDOWN:
		{
			if(nTestType == TEST_MOVE_WINDOW)
			{
				//	DEBUG: Move window while left button down
				POINT orgpt;
				orgpt.x = ppkt->scrnpt.x - deltax;
				orgpt.y = ppkt->scrnpt.y - deltay;
				ppkt->hWindow->WinMove(&orgpt);
			}
			
			if(nTestType == TEST_RESIZE_WINDOW)
			{
				//	DEBUG: Resize window while left button down
				int deltax,deltay;
				bool ygrowsup;
				SIZE wsize;
				deltax = ppkt->scrnpt.x - priorpt.x;
				deltay = ppkt->scrnpt.y - priorpt.y;
				priorpt = ppkt->scrnpt;
				ygrowsup = ppkt->hWindow->WinGetSize(&wsize);
				if(ygrowsup)
				{
					//  +y goes down screen. Drag down and right to grow
					//  screen origin is in lower-left - we adjust it
					//  so the upper-right stays anchored and the screen
					//  size grows and shrinks from there.
					POINT orgpt;
					deltay *= -1;
					ppkt->hWindow->WinGetOrigin(&orgpt);
					wsize.cx += deltax;
					wsize.cy += deltay;
					orgpt.y -= deltay;
					ppkt->hWindow->WinSetOriginAndSize(&orgpt,&wsize);
				}
				else 
				{
					//  +y goes down screen. Drag down and right to grow
					wsize.cx += deltax;
					wsize.cy += deltay;
					ppkt->hWindow->WinResize(&wsize);
				}
			}
			
			if(nTestType == TEST_DRAW_WINDOW)
			{
				//	DEBUG: Draw mouse tracks
				hIDraw = ppkt->hWindow->WinStartEventDraw();
				float rx,ry;
				hIDraw->IDrawSetLColor(COLOR_BLACK);
				hIDraw->IDrawSetLWeight(1.0);
				rx = ppkt->pt.x;
				ry = ppkt->pt.y;
				hIDraw->IDrawPoint(rx,ry);
				
				ppkt->hWindow->WinEndEventDraw();
			}
		}
			break;
			
		default:
			break;
	}
	
	hWinMgr->WMgrSetCursor(CSR_ARROW);

	return IM_RTN_NOTHING;
}

//	==============================================================================
//  This control needs its content drawn
//	==============================================================================

void* CCustomCtl::OnCtlDraw(MSGP)
{
	MPARMPTR(IDraw*, hIDraw);

	//	background fill
	if (wstyleflags & WSTYLE_ALPHA)
		hIDraw->IDrawClearBkgnd(0x40386800);
	else
		hIDraw->IDrawClearBkgnd(PALETTE_FCW7 + 0x1E);

	hIDraw->IDrawSetLineEndsRounded(false);
	hIDraw->IDrawSetLWeight(1.0);
	hIDraw->IDrawSetLColor(COLOR_RED);
	hIDraw->IDrawSetFColor(COLOR_TRANSPARENT);

	//	control outline
	DRECTF r;
	r.origin.x = 0.0;
	r.origin.y = 0.0;
	r.size.width = drect.size.width;
	r.size.height = drect.size.height;
	hIDraw->IDrawRectP(&r);

	hIDraw->IDrawSetLColor(COLOR_BLUE);
	hIDraw->IDrawLine(0,0,40,0);
	float y = CRect.bottom;
	hIDraw->IDrawLine(0,y,40,y);

	//	crossed diagonal lines
	//	blue from UL to BR
	hIDraw->IDrawSetLWeight(1.0);
	hIDraw->IDrawSetLColor(COLOR_BLUE);
	hIDraw->IDrawLine(drect.origin.x, drect.origin.y,
		drect.origin.x + drect.size.width - 1.0, drect.origin.y + drect.size.height - 1.0);

	//	yellow from LL to UR
	hIDraw->IDrawSetLColor(COLOR_YELLOW);
	hIDraw->IDrawLine(drect.origin.x + drect.size.width - 1.0, drect.origin.y,
		drect.origin.x, drect.origin.y + drect.size.height - 1.0);

	//	circle
	hIDraw->IDrawSetLColor(COLOR_BLUE);
	hIDraw->IDrawSetFColor(COLOR_GREEN);
	hIDraw->IDrawCir(60.0, 60.0, 20.0);

	//	and arcs
	hIDraw->IDrawArc(60.0, 60.0, 24.0, RAD(45.0), RAD(135.0));
	hIDraw->IDrawArc(60.0, 60.0, 26.0, RAD(180.0),RAD(135.0));

	//	point at center
	hIDraw->IDrawSetLColor(COLOR_RED);
	hIDraw->IDrawPoint(60.0, 60.0);

	//  draw text centered inside a rectangle
	hIDraw->IDrawSetFont(NULL, 12.0,0.0);
	r.origin.x = 60.0;
	r.origin.y = 90.0;
	r.size.width = 40.0;
	r.size.height = 14.0; // 2pixels more than text height (12.0 default)

	//	show rectangle containing text
	hIDraw->IDrawSetLColor(COLOR_YELLOW);
	hIDraw->IDrawSetFColor(COLOR_TRANSPARENT);
	hIDraw->IDrawRectP(&r);

	//	point at text origin
	hIDraw->IDrawSetLColor(COLOR_RED);
	hIDraw->IDrawPoint(r.origin.x, r.origin.y);
	hIDraw->IDrawSetTColor(COLOR_BLUE);
	hIDraw->IDrawText(TEXT("Helloj!"), &r);

	//  multiline paragraph text
	r.origin.x = 20.0;
	r.origin.y = 200.0;
	r.size.width = 100.0;
	r.size.height = 60.0;
	hIDraw->IDrawSetLColor(COLOR_YELLOW);
	hIDraw->IDrawRectP(&r);
	hIDraw->IDrawText(TEXT("Hello multiline wrapped text\n2nd line"), &r);

	//	same text centered in rectangle
	r.origin.x = 140.0;
	hIDraw->IDrawRectP(&r);
	hIDraw->IDrawCenteredText(TEXT("Hello multiline wrapped text\n2nd line"), &r);

	//	text at a point
	hIDraw->IDrawTextAt(TEXT("Helloj!"), 60.0, 60.0);

	//	big text in center
	r.origin.x = 0.0;
	r.origin.y = 0.0;
	r.size.width = drect.size.width;
	r.size.height = drect.size.height;

	//	set default text font at big size
	hIDraw->IDrawSetFont(NULL, 72.0, 0.0);
	hIDraw->IDrawSetTColor(0xA0FFFFFF);
	hIDraw->IDrawCenteredText(hLabel->StrTextPtr(), &r);

	//	rotated text at a point
	hIDraw->IDrawSetFont(NULL, 14.0, RAD(45.0));
	hIDraw->IDrawSetTColor(COLOR_WHITE);
	hIDraw->IDrawTextAt(TEXT("pRotated text"), 360.0, 160.0);
	hIDraw->IDrawSetLColor(COLOR_RED);
	hIDraw->IDrawPoint(360.0, 160.0);

	//	progress bar (animation)
	hIDraw->IDrawSetLWeight(8.0);
	hIDraw->IDrawSetLColor(COLOR_RED);
	hIDraw->IDrawSetLineEndsRounded(true);

	if (progress > 0.0)
	{
		double endp = (drect.size.width - 40.0)*progress + 20.5;
		hIDraw->IDrawLine(drect.origin.x + 20.5, 20.5, endp, 20.5);
	}

	//  direct buffer pixel set
	IImage* hDrawToImage = hIDraw->IDrawGetImage();
	if (hDrawToImage)
	{
		IMGINFO* pii = hDrawToImage->ImgInfo();
		uint32_t color = hDrawToImage->ImgDirectColor(COLOR_CYAN);
		// draw on top line (0,0 = bottom left)
		uint32_t* pDest = hDrawToImage->ImgPixelAdr(10, CRect.bottom);
		uint32_t npixels = 10;
		while (npixels--)
			*pDest++ = color;
	}

	//	draw any child controls
	((IControl*)this)->CtlDrawSL(hIDraw);
	return IM_RTN_NOTHING;
}

//	==============================================================================
//  MultiTouch Event
//	==============================================================================

void* CCustomCtl::OnCtlMultiTouch(MSGP)
{
	MPARMPTR(IArray*, hCurTouches);
	GPOINT2F pt;
	GPOINT2F winPt;
	POINT scrnPt;

	//	let child controls get their touches
	((IControl*)this)->CtlMultiTouchSL(hCurTouches);

	//	only handle single touches
	uint32_t nCurTouches = hCurTouches->AryCount();
	if (nCurTouches > 1)
		return IM_RTN_NOTHING;

	EVTOUCH* pTouch = (EVTOUCH*)hCurTouches->AryItemPtr(0);
	switch (pTouch->tstate)
	{
	case evt_begin:
		pTouch->hInCtl = (IControl*)this;
	case evt_track:
		pt.x = pTouch->touchAt.x;
		pt.y = pTouch->touchAt.y;
		winPt = pt;
		((IControl*)this)->CtlPtToWinPtF(&winPt);
		scrnPt.x = winPt.x;
		scrnPt.y = winPt.y;
		hWinMgr->WMgrCvtWinToScreenPt((IControl*)this,&scrnPt);
		break;
	default:
		return IM_RTN_NOTHING;
	}

	switch (nTestType)
	{
		case TEST_MOVE_WINDOW:
		{
			//	This has a lot of visual "jitter" unless we move
			//	the window slowly, from near the center. Is this
			//	an issue with the MSWin touch subsystem, or
			//	do we have a bad assumption?
			//
			//	DEBUG: Move window while touched
			POINT winOrgPt;
			//	get current window origin in screen coordinates
		
			hWindow->WinGetOrigin(&winOrgPt);

			switch (pTouch->tstate)
			{
				case evt_begin:
				{
					orgDelta.x = scrnPt.x - winOrgPt.x;
					orgDelta.y = scrnPt.y - winOrgPt.y;
					break;
				}
				case evt_track:
				{
					//	TODO: we need a "touch grab" in case touch outruns
					//	window move update and moves outside frame
					winOrgPt.x = scrnPt.x - orgDelta.x;
					winOrgPt.y = scrnPt.y - orgDelta.y;
					hWindow->WinMove(&winOrgPt);
					break;
				}
				case evt_end:
				{
					break;
				}
			}
			break;
		}

		case TEST_RESIZE_WINDOW:
		{
			if (pTouch->tstate == evt_begin)
			{
				priorPt = scrnPt;
				break;
			}

			if (pTouch->tstate == evt_track)
			{
				//	TODO: sometimes size jumps to huge;
				//	watching, we never set it to huge size
				//
				//	DEBUG: Resize window while touched
				int deltax, deltay;
				bool ygrowsup;
				SIZE wsize;
				deltax = scrnPt.x - priorPt.x;
				deltay = scrnPt.y - priorPt.y;
				priorPt = scrnPt;

				ygrowsup = hWindow->WinGetSize(&wsize);

				if (ygrowsup)
				{
					//  +y goes down screen. Drag down and right to grow
					//  screen origin is in lower-left - we adjust it
					//  so the upper-right stays anchored and the screen
					//  size grows and shrinks from there.
					POINT orgpt;
					deltay *= -1;
					hWindow->WinGetOrigin(&orgpt);
					wsize.cx += deltax;
					wsize.cy += deltay;
					orgpt.y -= deltay;
					hWindow->WinSetOriginAndSize(&orgpt, &wsize);
				}
				else
				{
					//  +y goes down screen. Drag down and right to grow
					wsize.cx += deltax;
					wsize.cy += deltay;

					hWindow->WinResize(&wsize);
				}
			}
			break;
		}

		case TEST_DRAW_WINDOW:
		{
			//	DEBUG: Draw mouse tracks
			hIDraw = hWindow->WinStartEventDraw();

			float rx, ry;
			hIDraw->IDrawSetLColor(COLOR_BLACK);
			hIDraw->IDrawSetLWeight(1.0);
			rx = winPt.x;
			ry = winPt.y;
			hIDraw->IDrawPoint(rx, ry);

			hWindow->WinEndEventDraw();
			break;
		}
	}

	return IM_RTN_NOTHING;
}


//	==============================================================================
