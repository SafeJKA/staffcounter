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

// Dlg_StaffWizard.cpp : implementation file
//

#include "stdafx.h"
#include "MainWnd.h"
#include "Dlg_StaffWizard.h"

#include "common1.h"
#include <atlimage.h>
#include "report.h"

bool RegisterMySelf(bool remove, bool allusers);

//CDemoRedirector CDlg_StaffWizard::m_redir = CDemoRedirector();

// CDlg_StaffWizard dialog

IMPLEMENT_DYNAMIC(CDlg_StaffWizard, CDialog)

CDlg_StaffWizard::CDlg_StaffWizard(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_StaffWizard::IDD, pParent)
	, _name(_T("")) 
	, _email(_T(""))
	, _autoregister(false)
{

}

CDlg_StaffWizard::~CDlg_StaffWizard()
{
}

void CDlg_StaffWizard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlg_StaffWizard)
	DDX_Text(pDX, IDC_EDIT7, _name);
	DDX_Text(pDX, IDC_EDIT1, _email);
	DDX_Text(pDX, IDC_EDIT9, _device_id);
	DDX_Check(pDX, IDC_CHECK3, _autoregister);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC2, _animation);
}


BEGIN_MESSAGE_MAP(CDlg_StaffWizard, CDialog)
	ON_BN_CLICKED(IDOK, &CDlg_StaffWizard::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT1, &CDlg_StaffWizard::OnEnChangeEdit1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CDlg_StaffWizard::OnBnClickedCancel)

	ON_STN_CLICKED(IDC_STATIC4, &CDlg_StaffWizard::OnStnClickedStatic4)
	ON_EN_CHANGE(IDC_EDIT9, &CDlg_StaffWizard::OnEnChangeEdit9)
END_MESSAGE_MAP()


// CDlg_StaffWizard message handlers

void CDlg_StaffWizard::OnBnClickedOk()
{
	UpdateData();
	// TODO: Add your control notification handler code here

	TCHAR text[200];
	TCHAR command[100]={0};
	report newReport;

	if ( strlen(_device_id) ==0 )
	{
		if ( strlen(_email) < 5)
		{
			AfxMessageBox(LS("Please enter Email"));
			return;
		}

		if ( strlen(_name) < 3)
		{
			AfxMessageBox(LS("Please enter user name"));
			return;
		}

		//wsprintf(text,"%s \"%s\" web_create_id test", _email, _name);		
		SetTimer(1, 3000, NULL);

		_animation.Play();
		_animation.ShowWindow(SW_SHOW);	
		SetDlgItemText(IDC_STATIC3, "");				
		enableItems(m_hWnd, 0, IDOK, 0);
		showItems(m_hWnd, 1, IDC_STATIC4, 0);

		
		newReport.send_report("web_create_id", _email, _name, "");


	} else // with entered device id we Request Settings
	{
		//wsprintf(text,"%s test web_get_settings test", _device_id);
		SetTimer(2, 3000, NULL);

		_animation.Play();
		_animation.ShowWindow(SW_SHOW);	
		SetDlgItemText(IDC_STATIC3, "");				
		enableItems(m_hWnd, 0, IDOK, 0);
		showItems(m_hWnd, 1, IDC_STATIC4, 0);

		newReport.send_report("web_get_settings", _device_id, "", "");
	}

	

	


	//OnOK();
}

void CDlg_StaffWizard::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void RunMySelf()
{
	char szModuleName[512];
	GetModuleFileName(NULL, szModuleName, 512);    
	strcat(szModuleName, " -m");

	

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	char prg_path[1000];	
	GetMyPath(prg_path, false);	

	
	memset( &pi, 0, sizeof( PROCESS_INFORMATION ) );
	memset( &si, 0, sizeof( STARTUPINFO ) );

	si.cb = sizeof(STARTUPINFO );
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;	
	
	int ret = CreateProcess( NULL, szModuleName, NULL, NULL, FALSE, /*IDLE_PRIORITY_CLASS*/ NORMAL_PRIORITY_CLASS  , NULL, prg_path ,&si,&pi );

	printf_log("RunMySelf %d %X %s %s", ret, GetLastError(), szModuleName , prg_path);

	if (ret ==0)
	{
		GetModuleFileName(NULL, szModuleName, 512);    
		ShellExecute(NULL, "open", szModuleName, "-m", NULL, SW_SHOW);
	}
	  
}

void CDlg_StaffWizard::OnTimer(UINT_PTR nIDEvent)
{
	
	// Create Dev ID Done
	if (nIDEvent == 1)
	{
		//if ( WaitForSingleObject(m_redir.GetChildProcessHandle(), 10) != WAIT_TIMEOUT )
		{		
			KillTimer(1);
			TCHAR last_err[600] = {""};
			
			ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", last_err, 400);
			CString s1;
			if (strlen(last_err) )
				s1.Format("Status: %s ",  last_err);		
			else 
				s1.Format("Status: OK");		
			SetDlgItemText(IDC_STATIC3, s1);		

			if ( strlen(last_err) ==0 || strstr(last_err, "OK") ) // continue getting Settings
			{
				TCHAR devid[100]= {""};
				ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "smtp_to", devid, 90);

				if ( strlen(devid) ==0)
				{
					_animation.Stop();
					_animation.ShowWindow(SW_HIDE);	
					showItems(m_hWnd, 0, IDC_STATIC4, 0);
					enableItems(m_hWnd, 1, IDOK, 0);
					SetDlgItemText(IDC_STATIC3, "Device ID is empty. Please run StaffCounter 'as Administrator'.");		
					return;

				}

				
				report newReport;
				newReport.send_report("web_get_settings", devid, "", "");

				SetTimer(2, 2000, NULL);
				enableItems(m_hWnd, 0, IDOK, 0);

				showItems(m_hWnd, 1, IDC_STATIC4, 0);

			} else
			{
				_animation.Stop();
				_animation.ShowWindow(SW_HIDE);	
				showItems(m_hWnd, 0, IDC_STATIC4, 0);
				enableItems(m_hWnd, 1, IDOK, 0);

			}

			// check if DSN is created and request Config from the server

		} 
	}

	// Get Settings Done
	if (nIDEvent == 2)
	{
		KillTimer(2);
		_animation.Stop();
		_animation.ShowWindow(SW_HIDE);	
		//enableItems(m_hWnd, 1, IDOK, 0);

		TCHAR last_err[600] = {""};
		ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", last_err, 400);
		CString s1;
		last_err[17]=0; 
		if (strlen(last_err) )
			s1.Format("Status: %s ",  last_err);		
		else 
			s1.Format("Status: OK");		
		SetDlgItemText(IDC_STATIC3, s1);		

		if ( strlen(last_err) ==0 || strstr(last_err, "OK") ) // continue getting Settings
		{
			AfxMessageBox("Activated successfully!", MB_ICONINFORMATION);

			// save this if enough privileges... 
			WriteReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "AdminEmail", (LPCTSTR)_email);
			WriteReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "AutoRegisterUsers", _autoregister);
			WriteReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "AllUsers", _autoregister); // this is the same -

			TCHAR devid[100]= {""};
			ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "smtp_to", devid, 90);
			if ( strlen(devid) ==0 )
			{
				TCHAR cmdline_str[100]= {""};
				strcpy(cmdline_str, _device_id);
				strcat(cmdline_str, " Report web_post ");						
				WriteReg(HKEY_CURRENT_USER, "Software\\StaffCounter", "report_cmdline", cmdline_str);
				WriteReg(HKEY_CURRENT_USER, "Software\\StaffCounter", "smtp_to", _device_id);
			}

			// Run the myself (StaffCounter.exe -m) 
			// 

			DWORD log_allusers = ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter") , "AllUsers", 0 );	

			if (log_allusers == 0)
			{
				 // remember current user ID - 
				char username[100];
				DWORD sz=50;
				GetUserName( username, &sz);
				WriteReg( HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter") , "selected_users", username);
			}


			RegisterMySelf(false, log_allusers);
			RunMySelf();
			EndDialog(IDOK);

		}	else
		{
			_animation.Stop();
			_animation.ShowWindow(SW_HIDE);	
			showItems(m_hWnd, 0, IDC_STATIC4, 0);
			enableItems(m_hWnd, 1, IDOK, 0);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL CDlg_StaffWizard::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_STATIC12, LS("Device ID:"));
	SetDlgItemText(IDC_STATIC13, LS("employee name"));	
	//SetDlgItemText(IDC_STATIC14, LS("Device ID:"));

	TCHAR userName[100]={""}; DWORD unLen=90; 	
	GetUserName(userName, &unLen);
	//SetDlgItemText(IDC_EDIT7, userName);
	_name = userName;

	_animation.LoadAnimatedGif("wait26trans.gif");
	_animation.ShowWindow(SW_HIDE);

	TCHAR text[600] = {""};
	ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "AdminEmail", text, 400);
	_email = text;

	char prg_path[100];	
	GetMyPath(prg_path, false);	
	strcat(prg_path, "logo.png");	
	CImage  img;
	img.Load(prg_path);	
	SendDlgItemMessage(IDC_STATIC7, STM_SETIMAGE, IMAGE_BITMAP , (LPARAM)img.Detach() ); 
	

	UpdateData(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_StaffWizard::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

/*
void CDlg_StaffWizard::OnBnClickedCancel2()
{	
	showItems(m_hWnd, 1, IDC_EDIT9, IDC_STATIC9, IDC_CHECK3, IDC_STATIC14,  0);
}
*/

int CDlg_StaffWizard::AutoRegisterUser(void)
{
	TCHAR email[600] = {""};
	TCHAR user_name[600] = {""};
	TCHAR device_id[600] = {""};
	TCHAR text[1000];
	DWORD dwSz=50;

	// generate device id
	GetUserName(user_name, &dwSz);
	ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "AdminEmail", email, 400);
	

	report newReport;
	newReport.send_report("web_create_id", email, user_name, "");

	

	// get settings
	ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "smtp_to", device_id, 90);

	if ( strlen(device_id) == 0 )
		return 0;

	wsprintf(text,"%s test web_get_settings test", device_id);

	

	newReport.send_report("web_get_settings",device_id, "", "" );

	return 1;

}

void CDlg_StaffWizard::OnStnClickedStatic4()
{
	// TODO: Add your control notification handler code here
}

void CDlg_StaffWizard::OnEnChangeEdit9()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
