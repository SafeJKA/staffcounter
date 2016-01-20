// MainWnd.h : main header file for the MAINWND application
//

#if !defined(AFX_MAINWND_H__E224136D_7D45_4EA9_99FF_4382B3E619BB__INCLUDED_)
#define AFX_MAINWND_H__E224136D_7D45_4EA9_99FF_4382B3E619BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "MainWndDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CMainWndApp:
// See MainWnd.cpp for the implementation of this class
//

class CMainWndApp : public CWinApp
{
	CMainWndDlg *pDlg;
public:
	CMainWndApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainWndApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMainWndApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINWND_H__E224136D_7D45_4EA9_99FF_4382B3E619BB__INCLUDED_)
