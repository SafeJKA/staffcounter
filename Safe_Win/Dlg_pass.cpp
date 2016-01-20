/**********************************************************************
** Copyright (C) 2009-2016 Tesline-Service S.R.L.  All rights reserved.
**
** StaffCounter Agent for Windows 
** 
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://StaffCounter.net/ for GPL licensing information.
**
** Contact info@rohos.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/



// Dlg_pass.cpp : implementation file
//

#include "stdafx.h"
#include "MainWnd.h"
#include "Dlg_pass.h"

#include "common1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlg_pass dialog


CDlg_pass::CDlg_pass(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_pass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlg_pass)	
	_password = _T("");	
	//}}AFX_DATA_INIT
}


void CDlg_pass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlg_pass)
	
	DDX_Text(pDX, IDC_EDIT1, _password);
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlg_pass, CDialog)
	//{{AFX_MSG_MAP(CDlg_pass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlg_pass message handlers

void CDlg_pass::OnOK() 
{		
	UpdateData();

	TCHAR str[106];
	ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Explorer") , "Tesl_key0", str, 100);
	
	if (_password != str) {
		AfxMessageBox("Wrong password. Please enter again", MB_ICONWARNING);
		return;
	}

	CDialog::OnOK();
}

INT_PTR CDlg_pass::DoModal() 
{
	TCHAR str[106];
	ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Explorer") , "Tesl_key0", str, 100);

	if ( _tcslen(str) == 0 ) {
		return IDOK; // no password protection;
	}

	return CDialog::DoModal();
}

BOOL CDlg_pass::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_STATIC1, LS("Please provide password to access this program"));
	SetDlgItemText(IDC_STATIC2, LS("Password"));

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
