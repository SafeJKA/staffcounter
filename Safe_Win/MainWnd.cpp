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


#include "stdafx.h"
#include "MainWnd.h"
#include "MainWndDlg.h"
#include "Dlg_StaffWizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainWndApp

BEGIN_MESSAGE_MAP(CMainWndApp, CWinApp)
	//{{AFX_MSG_MAP(CMainWndApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainWndApp construction

CMainWndApp::CMainWndApp()
{
	printf_log("CMainWndApp %s --------------", __DATE__);
	
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMainWndApp object

CMainWndApp theApp;
HINSTANCE h_lang;

bool UploadLogDataNow(int flush_buffer);

/////////////////////////////////////////////////////////////////////////////
// CMainWndApp initialization

void set_foreground(HWND hWnd)
{
	HWND hCurrWnd;
	int iMyTID;
	int iCurrTID;
	
	hCurrWnd = ::GetForegroundWindow();
	iMyTID   = GetCurrentThreadId();
	iCurrTID = GetWindowThreadProcessId(hCurrWnd,0);
	
	AttachThreadInput(iMyTID, iCurrTID, TRUE);
	
	SetForegroundWindow(hWnd);
	BringWindowToTop(hWnd);
    
	AttachThreadInput(iMyTID, iCurrTID, FALSE);	
}


BOOL CMainWndApp::InitInstance()
{
	CoInitialize(NULL);
	
	char str[500];	
	GetMyPath(str, false, NULL);		
	SetCurrentDirectory(str);

	TCHAR ProductName[100]= {"StaffCounter"};
	TCHAR WinCaption[200]={"Staff Counter"};

	_tcscpy(ProductName, ::LS("StaffCounter") );
	
	ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "ProductName", ProductName, 90);		

	printf_log("InitInstance 1 %s", ProductName);
	
	if (strlen(ProductName))	
	{
		strcpy(WinCaption, ProductName);				
		strcat(WinCaption, " []");

		TCHAR *new_app_name = new char[100];
		_tcscpy(new_app_name, ProductName);

		m_pszAppName = new_app_name;	

	}
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	pDlg = new CMainWndDlg();
	pDlg->_start_in_reg_mode = 0;

	printf_log("InitInstance 2");

	TCHAR path[2048];
	GetModuleFileName(NULL,path,500);

	path[3]=0;
	if (GetDriveType(path)==DRIVE_REMOVABLE)
	{
			//copy_stealth();
			ExitProcess(1);
			return false;

	} else
	if ( strlen(m_lpCmdLine) ) 
	{
		if(!strcmp(m_lpCmdLine,"/upload"))
		{
			// upload any data and exit
			
			UploadLogDataNow(0);			
			ExitProcess(1);
			return false;
		}			

		if( FindWindow(NULL, WinCaption) )
			ExitProcess(0);
			
		if(!strcmp(m_lpCmdLine,"/stealth"))
		{
			//copy_stealth();
			ExitProcess(1);
			return false;
		}

		if(!strcmp(m_lpCmdLine,"/init"))
		{
			TCHAR str[700];
			GetSystemErrorMessage(0, str); // operation successfully completed			
			AfxMessageBox(str, MB_ICONINFORMATION);	

			WriteReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter") , "selected_users", "");
			WriteReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter") , "AllUsers", 1);
			pDlg->_start_in_reg_mode = 1;
			Sleep(3000);			
		}	
		
		pDlg->_start_minimized = true;

	} 
	else 
	{
		
		printf_log("InitInstance 1.3 %s", WinCaption);	
			
		HWND wnd = FindWindow(NULL, WinCaption);
		if (wnd) {
			printf_log("already run");
			ShowWindow(wnd, SW_SHOW);
			ShowWindow(wnd, SW_RESTORE);
			set_foreground(wnd);
			return false;
		}		
	}

	printf_log("InitInstance 3");

	// check for StaffCounter	
	if ( strcmp(ProductName, "StaffCounter") == 0)
	{
		TCHAR devid_str[100]= {""}; 

		ReadReg( HKEY_CURRENT_USER,  TEXT("SOFTWARE\\StaffCounter"), "smtp_to", devid_str, 90);

		if ( strlen(devid_str) ==0 && ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "AutoRegisterUsers", 0) )
		{						
			// auto register user...
			CDlg_StaffWizard::AutoRegisterUser();

		} else
		if ( strlen(devid_str) ==0) // User needs to fill the data now
		{
			// Run Wizard 
			CDlg_StaffWizard dlg;

			if (dlg.DoModal() == IDOK)
			{
				return false; // CDlg_StaffWizard will do the Job itself and start monitoring
			} 				
			return false; 
		} 

		TCHAR cmdline_str[500] = {""};
		ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "report_cmdline", cmdline_str, 90);
		if ( strlen(cmdline_str) ==0)
		{
			strcpy(cmdline_str, devid_str);
			strcat(cmdline_str, " Report web_post ");						
			WriteReg(HKEY_CURRENT_USER, "Software\\StaffCounter", "report_cmdline", cmdline_str);

			// todo - read settings now...			
		}

		//pDlg->_start_minimized = true; // activate recording automatically... 
	}


	m_pMainWnd = pDlg;
	if ( !pDlg->Create(IDD_MAINWND_DIALOG) ) 
	{
		printf_log("Dlg Create Failed");
		pDlg->DoModal();
	}


	return TRUE; // start the application's message pump
}

int CMainWndApp::ExitInstance() 
{
	CoUninitialize();	
	return CWinApp::ExitInstance();
}
