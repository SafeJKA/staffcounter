// MainWndDlg.h : header file
//

#if !defined(AFX_MAINWNDDLG_H__ED175F31_EFDE_4CE3_AB4A_690508FE5312__INCLUDED_)
#define AFX_MAINWNDDLG_H__ED175F31_EFDE_4CE3_AB4A_690508FE5312__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMainWndDlg dialog


class CMainWndDlg : public CDialog
{
	
// Construction
public:
	ULONG GetLocalUsers(/*CStringArray &arrUsers*/);
	void FillUserCheckBoxes();
	TCHAR _logged_users[400]; // the array of users to log
	char _username[100]; // current user name

	void DeleteOldLogFiles();
	void Monitor_ActiveWindowCaption();

	bool _working;
	BOOL _start_minimized;
	int _start_in_reg_mode;
	BOOL _safe_logger;

	CMainWndDlg(CWnd* pParent = NULL);	// standard constructor
	int _log_details; // if 1 - means log FULL URL, Win caption changes. 
	int _log_details_url;
	int _log_files_usage; // if 1 - means log all Used files and folders
	BOOL	_log_urls;

	TCHAR _report_cmd_line[300]; // Command line with email or device ID 
	TCHAR _last_docName[300];

	BOOL _product_staff; // 1 - staffcounter 
	BOOL _log_format_details_dur; // 1 - staffcounter 
	TCHAR _ProductName[200];
	BOOL logBrowserTitleAsURL; //
	
	bool IsUserInputIdle();

	CTime last_10min_time; /// when it was 10 minutes ago

	void ReloadConfig();
	bool IsThisStringURL(LPCTSTR url);

	char _current_class_name[100]; // current app class name	
	


// Dialog Data
	//{{AFX_DATA(CMainWndDlg)
	enum { IDD = IDD_MAINWND_DIALOG };
	CStatic	_ads;
	BOOL	_log_allusers;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainWndDlg)
	afx_msg LRESULT OnTaskBarCreated(WPARAM wp, LPARAM lp);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CTime _resume_time; ///< at what day, hour, min - the StaffCounter should resume logging

	// Generated message map functions
	//{{AFX_MSG(CMainWndDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnStop();
	virtual void OnCancel();
	afx_msg void OnStatic3();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnOpenLogFolder();
	afx_msg void OnClearLogs();
	afx_msg void OnEndSession(BOOL bEnding);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT onPowerChanges(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()


private:
	void Tray_Start();
	void Tray_Stop();
	void Tray_Tip(CString tipTitle, CString tipText);
	void OnStop_();
	void OnStart(BOOL reload_cfg =0);

	OSVERSIONINFO verInfo;
	
	void Log_Windows_Explorer_Path(HWND wnd);
	BOOL Log_OperaURL(HWND wnd);
	BOOL Log_FireFoxURL(HWND wnd);
	BOOL Log_InternetExplorerURL(HWND wnd);
	BOOL Log_ChromeURL(HWND wnd);

	bool IsWindowsVista(void);

	TCHAR _last_opera_url_entry[500];
	TCHAR _last_folder_path[6500];
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnBnClickedCancel();
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedCancel1();

public:
	// start log, set UI, delete old log files
	int StartLog_ReloadConfig(void);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINWNDDLG_H__ED175F31_EFDE_4CE3_AB4A_690508FE5312__INCLUDED_)
