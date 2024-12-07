//	======================================================================
//	CDlgAboutSimpleApp - About SimpleApp dialog
//	----------------------------------------------------------------------
//	Copyright ©2012 Evolution Computing, Inc.
//	All rights reserved
//	======================================================================

#include "fwkw.h"
#include "rscid.h"

//	======================================================================
//	Class Definition
//	======================================================================

class CDlgAbout : public IMObject
{
public:
	CDlgAbout(void);
	~CDlgAbout(void);
	virtual void* Msg(MSGDISPATCH);
	void* OnDlgInit(MSGP);
	void* OnDlgEnd(MSGP);
public:
	IControl*	hDlgCtl;
	ICtlLabel*	hVerLbl;
	ICtlLabel*	hNameLbl;
	ICtlLabel*	hCompanyLbl;
	ICtlLabel*	hDateLbl;
	ICtlButton*	hPictBtn;
};

//	======================================================================
//	Dialog Class factory
//	======================================================================

static IDialog* hAboutDlg = NULL;

DLLEXPORT IDialog* MakeDlgAbout(void)
{
	//	only create if the dialog is not open
	if(!hAboutDlg)
	{
		hAboutDlg = (IDialog*)new CDlgAbout();
		MakeDlgCtl(hAboutDlg, RscText(RSC_DlgAboutFcw9));
	}
	return hAboutDlg; 
}

//	======================================================================
//	Constructor
//	======================================================================

CDlgAbout::CDlgAbout(void)
{
}

//	======================================================================
//	Destructor
//	======================================================================

CDlgAbout::~CDlgAbout(void)
{
	hAboutDlg = NULL;
}

//	======================================================================
//	Variable length message receiver/dispatcher
//	======================================================================

void* CDlgAbout::Msg(MSGDISPATCH)
{
	switch(MSGID)
	{
	case MSG_DlgInit:		return OnDlgInit(MPPTR);
	case MSG_DlgEnd:		return OnDlgEnd(MPPTR);
	case MSG_DlgSetTitle:
		hDlgCtl->MSGFWD(MSGID);
		hDlgCtl->CtlNeedsRedraw();
		return IM_RTN_NOTHING;
	};

	return IM_RTN_IGNORED;
}

//	======================================================================
//	OnDlgInit - initialize dialog controls now
//	======================================================================

void* CDlgAbout::OnDlgInit(MSGP)
{
	MPARMPTRV(IControl*,hDlgCtl);
	wchar_t szText[MAX_PATH];

	hPictBtn = (ICtlButton*)hDlgCtl->DbFindChildId(500);
	hVerLbl = (ICtlLabel*)hDlgCtl->DbFindChildId(512);
	hDateLbl = (ICtlLabel*)hDlgCtl->DbFindChildId(513);
	hNameLbl = (ICtlLabel*)hDlgCtl->DbFindChildId(514);
	ICtlButton* hBtn = (ICtlButton*)hDlgCtl->DbFindChildId(598);

	IImage* hProgramIcon = MakeImage(RscImage(RSCIMG_AboutImage));

	hPictBtn->CtlSetImage(hProgramIcon);
	hSvc->SvcCopyW(szText,RscText(RSC_Version));
	hSvc->SvcAppendW(szText,(wchar_t*)hApplication->Msg(MSG_AppVersionText,NULL));

	hVerLbl->CtlSetText(szText);
	hDateLbl->CtlSetText(hXPMgr->FwkBuildDate());

	hNameLbl->CtlSetText(RscText(RSC_UserName));
	
	hBtn->CtlSetNotify(this);

	return IM_RTN_NOTHING;
}

//	======================================================================
//	OnDlgEnd - notification: dialog is ending
//	======================================================================

void* CDlgAbout::OnDlgEnd(MSGP)
{
	MPARMINT(int,endid);

	Release();
	return IM_RTN_NOTHING;
}

//	======================================================================
