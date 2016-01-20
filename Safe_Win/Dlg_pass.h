#if !defined(AFX_DLG_PASS_H__77A17FAF_DAD9_4021_8C2D_4D822998C524__INCLUDED_)
#define AFX_DLG_PASS_H__77A17FAF_DAD9_4021_8C2D_4D822998C524__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Dlg_pass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlg_pass dialog

class CDlg_pass : public CDialog
{
// Construction
public:
	CDlg_pass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlg_pass)
	enum { IDD = IDD_DIALOG1 };
	BOOL	_enable_password;
	CString	_password;
	CString	_confirmation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlg_pass)
	public:
	virtual INT_PTR DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlg_pass)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLG_PASS_H__77A17FAF_DAD9_4021_8C2D_4D822998C524__INCLUDED_)
