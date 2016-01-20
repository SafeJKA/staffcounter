#pragma once

#include "GifAnimation.h"
//#include "demoredir.h"
// CDlg_StaffWizard dialog

class CDlg_StaffWizard : public CDialog
{
	DECLARE_DYNAMIC(CDlg_StaffWizard)

	//static CDemoRedirector m_redir;
public:
	CDlg_StaffWizard(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_StaffWizard();

// Dialog Data
	enum { IDD = IDD_STAFF_WIZ };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CGifAnimation  _animation;
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnEnChangeEdit1();

public:
	CString _name;
public:
	CString _email;
	CString _device_id;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedCancel();
public:
	afx_msg void OnBnClickedCancel2();
public:
	BOOL _autoregister;
public:
	static int AutoRegisterUser(void);
public:
	afx_msg void OnStnClickedStatic4();
public:
	afx_msg void OnEnChangeEdit9();
};
