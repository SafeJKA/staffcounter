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


#define WINVER 0x0500
#define _WIN32_IE 0x0500
#include "stdafx.h"

#include <stdio.h>

#include "TCHAR.H"

#include <shlobj.h>
#include <Shlwapi.h>
#include <io.h>
#include <eh.h>

HANDLE hFile=0;

/** return path with \\
*/
void GetMyPath(LPTSTR path, int include_name, HMODULE handle)
{
    GetModuleFileName(handle, path, MAX_PATH);
	if ( include_name != 0 ) return;

    LPTSTR ch = _tcsrchr(path, '\\');
    if ( ch ) *(ch+1) = 0;
}


void printf_log(LPCTSTR szData, ...)
{
	
	if (hFile == 0)
	{
		TCHAR my_path[500];
		GetMyPath(my_path, 0);
		_tcscat(my_path, "report.txt");

		/*
		if (_access(my_path, 0) != 0)
		{
			hFile == INVALID_HANDLE_VALUE;
			return;
		}
		*/

		hFile = CreateFile( my_path, GENERIC_WRITE, FILE_SHARE_READ| FILE_SHARE_WRITE ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}

		SetFilePointer(hFile, 0, NULL, FILE_END);

		DWORD dwSize = GetFileSize (hFile, NULL);

		if ( dwSize > (9 * 100 * 1024) ) // Size Checking.
		{			
			CloseHandle(hFile);
			DeleteFile(my_path);
			hFile = CreateFile( my_path, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);
			
		}
	}

	TCHAR Buffer[10000] = {0};
	va_list pArgList;

	va_start(pArgList, szData);
	 // The last argument to wvsprintf points to the arguments
    _vsntprintf(Buffer, sizeof (Buffer), 
                  szData, pArgList) ;
	va_end(pArgList);

#ifdef _X64
	_tcscat(Buffer, _T(" (x64)"));
#endif

	_tcscat(Buffer, _T("\r\n"));

	if (hFile == INVALID_HANDLE_VALUE)
	{		
		return;
	}

	CTime currtime = CTime::GetCurrentTime();
	CString time = currtime.Format("%H:%M:%S  ");

	DWORD unBytesWritten;
	WriteFile(hFile, (LPCTSTR)time, time.GetLength(), &unBytesWritten, NULL);
	WriteFile(hFile, Buffer, strlen(Buffer), &unBytesWritten, NULL);
	FlushFileBuffers(hFile);
	CloseHandle(hFile);
	hFile=0;
}



void enableItems(HWND parent, bool en, ...)
{
	va_list ap;
	int item;
	va_start(ap, en);

	while ((item= va_arg(ap,int)) != 0)	
	{
		EnableWindow( GetDlgItem(parent,item), en);
	}
		
	va_end(ap);
}


void showItems(HWND parent, int show, ...)
{
	va_list ap;
	int item;
	va_start(ap, show);

	while ((item= va_arg(ap,int)) != 0)			
		ShowWindow( GetDlgItem(parent, item), show);
		
	va_end(ap);
}

// Read setting from HKEY_CURRENT_USER first if == ""
//   then read from HKEY_LOCAL_MACHINE
LPCTSTR ReadRegAny(LPCTSTR path, LPCTSTR val_name, LPTSTR ret_value, DWORD buff_len) 
{
	LPCTSTR ret_val = ReadReg(HKEY_CURRENT_USER, path, val_name, ret_value, buff_len);

	if (strlen(ret_val) == 0)
		ret_val = ReadReg(HKEY_LOCAL_MACHINE, path, val_name, ret_value, buff_len);

	return ret_val;
}

DWORD ReadRegAny( LPCTSTR path, LPCTSTR val_name, DWORD i) 
{
	DWORD ret_val = ReadReg(HKEY_CURRENT_USER,  path, val_name, i);

	if ( ret_val == 0)
		ret_val = ReadReg(HKEY_LOCAL_MACHINE, path, val_name, i);

	return ret_val;
}


/** read a global setting from registry (at User)
params - none
key, HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER
*/
LPCTSTR ReadReg(HKEY key, LPCTSTR path, LPCTSTR val_name, LPTSTR ret_value, DWORD buff_len) 
{	
	HKEY hKey;
	
	if (!ret_value) return 0;

	ret_value[0]=0;
		
	if( RegOpenKeyEx(key, path, 0, KEY_QUERY_VALUE, &hKey)) 
		return ret_value;
	
	
	buff_len = (buff_len-1) * sizeof(TCHAR);
	memset(ret_value, 0, buff_len);
	buff_len -= 2;
	DWORD type = REG_SZ;	
	
	LONG ret = RegQueryValueEx(hKey, val_name, NULL, &type, (LPBYTE)ret_value, &buff_len);
	
	if(ERROR_SUCCESS !=ret 	){			
		
	}	else
	{

	ret_value[buff_len]=0;
	ret_value[buff_len+1]=0;
	}
	
	RegCloseKey(hKey);

	return ret_value;
}




/** read a global setting from registry (at User)
params - none
*/
DWORD ReadReg(HKEY key, LPCTSTR path, LPCTSTR val_name, DWORD i) 
{	
	HKEY hKey;
	DWORD len, val[3]={0};
	DWORD type=0;
	
	if( RegOpenKeyEx(key, path, 0, KEY_QUERY_VALUE, &hKey)) 
		if ( RegCreateKey(key, path, &hKey) )
		{
			// read/write HKCU if there is no access rights
			RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_QUERY_VALUE, &hKey);
		}
	
	len = sizeof(DWORD)+1;
	LONG ret = RegQueryValueEx(hKey, val_name, NULL, &type, (LPBYTE)&val, &len);
	if ( ERROR_SUCCESS != ret) {
		
		val[0] = i;
	}

	if (type==REG_SZ) { 
		int i=0;
		if ( _tcslen((LPCTSTR)val) )
			i = _ttoi( (LPCTSTR)val);
		val[0] = i;
	}
		
	RegCloseKey(hKey);

	return val[0];
}


/** write a local setting from registry (at User)
params - none
*/
DWORD ReadReg(HKEY key, LPCTSTR path, LPCTSTR val_name, LPBYTE value, LPDWORD len) 
{	
	HKEY hKey;
	LONG ret;
	DWORD type = REG_BINARY;
		
	if( RegOpenKeyEx(key, path, 0, KEY_QUERY_VALUE, &hKey)) 
		if ( RegCreateKey(key, path, &hKey) )
		{
			// read/write HKCU if there is no access rights
			RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_QUERY_VALUE, &hKey);
		}

	if (!value && len) {		
		ret = RegQueryValueEx(hKey, val_name, 0, &type, NULL, len);
	}
	else {		
		ret = RegQueryValueEx(hKey, val_name, 0, &type, value, len);
		if ( ERROR_SUCCESS != ret) {		
			*len=0;
		}
	}

	RegCloseKey(hKey);

	return *len;
}


/** write a global setting from registry (at User)
params - none
*/
bool WriteReg(HKEY key, LPCTSTR path, LPCTSTR val_name, LPCTSTR value) 
{	
	HKEY hKey;
	LONG ret;
	DWORD dwDisposition=0;
		
	if( RegOpenKeyEx(key, path, 0, KEY_READ|KEY_WRITE, &hKey)) 
		if ( RegCreateKeyEx(key, path, 0, 0, 0, KEY_READ|KEY_WRITE, NULL, &hKey, &dwDisposition) )
		{
			// read/write HKCU if there is no access rights
			if ( RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_READ|KEY_SET_VALUE, &hKey) )
				RegCreateKey(HKEY_CURRENT_USER, path, &hKey);
		}

	if (value==NULL) 
		ret = RegDeleteValue(hKey, val_name);
	else {		
		ret = RegSetValueEx(hKey, val_name, 0, REG_SZ, (LPBYTE)value, _tcslen(value) * sizeof(TCHAR));
		
		
	}
	RegCloseKey(hKey);

	if (ret == ERROR_SUCCESS) 
		return true;
	else
		return false;

}


/** write a to registry (at User)
params - none
*/
void WriteReg(HKEY key, LPCTSTR path, LPCTSTR val_name, LPBYTE value, DWORD len) 
{
	
	HKEY hKey;
	LONG ret;
		
	if( RegOpenKeyEx(key, path, 0, KEY_READ|KEY_WRITE, &hKey)) 
		if ( RegCreateKey(key, path, &hKey) )
		{
			// read/write HKCU if there is no access rights
			if ( RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_READ|KEY_WRITE, &hKey) )
				RegCreateKey(HKEY_CURRENT_USER, path, &hKey);
		}

	if (value==NULL) 
		ret = RegDeleteValue(hKey, val_name);
	else {		
		ret = RegSetValueEx(hKey, val_name, 0, REG_BINARY, value, len);
	
	}

	RegCloseKey(hKey);

	return ;
}



/** write a global setting from registry (at User)
params - none
*/

bool WriteReg(HKEY key, LPCTSTR path, LPCTSTR val_name, DWORD value) 
{	
	HKEY hKey;
	LONG ret=0;
		
	if( RegOpenKeyEx(key, path, 0, KEY_READ|KEY_WRITE, &hKey)) 
		if ( RegCreateKey(key, path, &hKey) )
		{
			// read/write HKCU if there is no access rights
			if ( RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_READ|KEY_WRITE, &hKey) )
				RegCreateKey(HKEY_CURRENT_USER, path, &hKey);
		}

	ret = RegSetValueEx(hKey, val_name, 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));

	RegCloseKey(hKey);

	if (ret == ERROR_SUCCESS) 
		return true;
	else
		return false;

}





/** get disk space in Megabytes
*/
__int64 GetDiskSpaceMB(LPCTSTR root_path) {

	if (root_path==NULL) return 0;
	ULARGE_INTEGER FreeBytesAvailable;    // bytes available to caller
	ULARGE_INTEGER TotalNumberOfBytes;    // bytes on disk
	//ULARGE_INTEGER TotalNumberOfFreeBytes // free bytes on disk

	if ( GetDiskFreeSpaceEx( root_path, &FreeBytesAvailable, 
				&TotalNumberOfBytes, NULL) ) 

		return TotalNumberOfBytes.QuadPart / 1048000;
	else {

		
		if (!root_path) return 0;
		if (_tcslen(root_path)<2) return 0;
		

		return 0;
	}

}

/** get disk Free space in Megabytes
*/
__int64 GetDiskFreeSpaceMB(LPCTSTR root_path) {


	if (!root_path) return 0;
	if (_tcslen(root_path)<2) return 0;

	ULARGE_INTEGER FreeBytesAvailable;    // bytes available to caller
	ULARGE_INTEGER TotalNumberOfBytes;    // bytes on disk
	ULARGE_INTEGER TotalNumberOfFreeBytes; // free bytes on disk

	if ( GetDiskFreeSpaceEx( root_path, &FreeBytesAvailable, 
				&TotalNumberOfBytes, &TotalNumberOfFreeBytes) ) 

		return TotalNumberOfFreeBytes.QuadPart / 1048000;
	else {

	
		return 0;
	}

}

#define GWL_USERDATA        (-21)

// Creates ToolTip for Control in dialog
HWND CreateToolTip(HWND hWnd, UINT idControl, LPCTSTR lpString, UINT iTime, bool baloon  )
{
	DWORD flags = TTS_NOPREFIX | TTS_ALWAYSTIP;
	if (baloon)
		flags = WS_POPUP | TTS_NOPREFIX | TTS_BALLOON | TTS_CLOSE | TTS_ALWAYSTIP;

				// CREATE A TOOLTIP WINDOW
	HWND hWndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, NULL, flags, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL 	);
	
	TOOLINFO ti;	
	SetWindowLong( GetDlgItem(hWnd, idControl), GWL_USERDATA, (LONG)hWndTT);
	
	memset(&ti, 0, sizeof(TOOLINFO) );
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND ;
	
	ti.hwnd = hWnd ;

	ti.uId = (UINT)GetDlgItem(hWnd, idControl);
	ti.lpszText = (LPTSTR)lpString; 
	if (baloon) {
		ti.lpszText = NULL; 
		ti.uId = idControl; 
		ti.uFlags = TTF_TRANSPARENT | TTF_CENTERTIP | TTF_TRACK | TTF_SUBCLASS;
	}
	
	
	SendMessage(hWndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	
	SendMessage(hWndTT, TTM_SETMAXTIPWIDTH, 0, (LPARAM) 200);
	SendMessage(hWndTT, TTM_SETDELAYTIME, TTDT_INITIAL, iTime); // ... time to wait before the ToolTip window appears
	SendMessage(hWndTT, TTM_SETDELAYTIME, TTDT_AUTOPOP, 8000); // ... time to Tip - remains visible 

	return hWndTT;
}

BOOL SelectFolderDialog( HWND hDlg, LPTSTR Title, LPTSTR FolderPath, DWORD flags, DWORD clsid_root )
{
	BROWSEINFO BrowseInfo;
	ITEMIDLIST *pItemIDList;

	ITEMIDLIST *RootIDList = NULL;

	if (clsid_root)
		SHGetFolderLocation(NULL, clsid_root, NULL, NULL, &RootIDList);

	memset( &BrowseInfo, 0, sizeof( BROWSEINFO ) );
	 

	BrowseInfo.hwndOwner = hDlg;
	BrowseInfo.pidlRoot = RootIDList;
	BrowseInfo.pszDisplayName = FolderPath;
	BrowseInfo.lpszTitle = Title;
	if (flags==0)
		BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	else 
		BrowseInfo.ulFlags = flags ;
	BrowseInfo.lpfn = NULL;
	BrowseInfo.lParam = NULL;
	BrowseInfo.iImage = NULL;
	if( ( pItemIDList = SHBrowseForFolder( (LPBROWSEINFO)&BrowseInfo ) ) != NULL &&
		SHGetPathFromIDList( pItemIDList, FolderPath ) )
			return TRUE;
 return FALSE;   

}

/* remove chars that cannot be in the file name
*/
void prepare_file_name_str(LPTSTR file_name)
{
	int len = strlen(file_name);
	while (true)
	{
		int idx = strcspn(file_name, "\\/:8?\"<>|");
		if (idx>=len)
			return;

		file_name[idx]=' ';
	}
}

#define BUFSIZE 500

LPCTSTR get_windows_platform_info()
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;
	static TCHAR tcSysInfo[MAX_PATH+5],bf[50];
	
	memset(tcSysInfo,0,MAX_PATH);
	
	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.
	
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	
	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return tcSysInfo;
	}
	
	switch (osvi.dwPlatformId)
	{
		// Test for the Windows NT product family.
	case VER_PLATFORM_WIN32_NT:

		sprintf( tcSysInfo,TEXT("Windows %d.%d %d"), osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.wProductType );
		
		// Test for specific product on Windows NT 4.0 SP6 and later.
		
		break;
		
      case VER_PLATFORM_WIN32s:
		  
		  _tcscpy(tcSysInfo,TEXT("Microsoft Win32s\n"));
		  break;
   }
   
   
   return tcSysInfo; 
}

/** return path with \\
*/
void GetPodNogamiPath(LPTSTR path, int include_name, HMODULE handle)
{
    GetModuleFileName(handle, path, MAX_PATH);
	if ( include_name != 0 ) return;

    LPTSTR ch = _tcsrchr(path, '\\');
    if ( ch ) *(ch+1) = 0;
}


#define LANG_VALUE "Lang"

// language files
static char* lpStrings = NULL;
static char* lpStringDefs = NULL;
static BOOL	is_lang_unicode = false;
static char* lpStrings_Js = NULL;
static char* lpStrings_English = NULL;


extern HINSTANCE h_lang; // lang hinstance

//  from JS file
LPCTSTR LS(LPCSTR js_string_name) {

	static TCHAR strOut[800];

	strOut[0]=0;

	if (lpStrings_Js == NULL ) {

		TCHAR str1[150];
		char str[150];

		lpStrings_Js = (char*)0xFFFFFFFF; 

		
		TCHAR path[500];
		GetPodNogamiPath(path, false, NULL);
		

		
			
			_tcscat(path,TEXT("languages\\") );
			ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), LANG_VALUE, str1, 30);

			if (_tcslen(str1) == 0)
				_tcscpy( str1, "english" );
			
			_tcscat(path, str1 );
			_tcscat(path, TEXT(".txt") );
		

		

		if ( _taccess(path, 0) ==0 ) {
			
			HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
			DWORD readSz=0, fileSize = GetFileSize(h, NULL);
			lpStrings_Js = new char[fileSize+10];
			memset(lpStrings_Js, 0, fileSize+5);
			ReadFile(h, lpStrings_Js, fileSize, &readSz, NULL);
			CloseHandle(h);

			if (readSz ==0) {
				lpStrings_Js = (char*)0xFFFFFFFF; 

				printf_log("LS: error read %s", path);					
			}
			
		}	
			else
			lpStrings_Js = (char*)0xFFFFFFFF; 

	}

	if (lpStrings_Js != (char*)0xFFFFFFFF ) {
		


		_tcscpy(strOut, _T(""));

		char defString[500] = {0};
		char String[500] = {0};

		memset(String, 0, 490);
		char str[150];

		strcpy(defString, js_string_name); 


		
		if ( strlen(defString) ) {
			
			strcat(defString, " "); 


			const char* lpDef = strstr(lpStrings_Js, defString);

			if (!lpDef) {
				defString[ strlen(defString)-1 ] = '\x9';				
				lpDef = strstr(lpStrings_Js, defString);
				if (!lpDef) {
					defString[ strlen(defString)-1 ] = '=';				
					lpDef = strstr(lpStrings_Js, defString);
				} 
			} 			
			

			if (lpDef ) {
				lpDef++;
				lpDef = strstr(lpDef, "\"");
				if(lpDef) {
					lpDef++;
					const char* strend = strpbrk(lpDef, "\x0D\x0A");  
					if ( strend && strend>lpDef &&strend-lpDef < 490)
						strncpy(String, lpDef, strend-lpDef-1); 
				}

			}

			

			
			if ( strlen(String) ) {
				char* s1 ;
				while (s1 = strstr(String, "\\n") ){				
					*s1 = ' ';
					*(s1+1) = '\n';

				}

				int i1=strlen(String);

				if (String[i1-1]=='\x22') 
					String[i1-1]=0;
			}


		} else {
			
			printf_log("LS(%s)",js_string_name );

			return strOut;
			
			

		}



		if ( strlen(String) ) {

#ifdef UNICODE			
			MultiByteToWideChar(CP_ACP, 0, String, -1, strOut, 750);
#else
			strcpy(strOut, String);
#endif
			return strOut;

		}

		else
			return LS_ENG(js_string_name); // return English Variant
	}

	printf_log("LS(%s).",js_string_name ); // return English Variant
	
	return LS_ENG(js_string_name);
}




// loads language file - English Only
LPCTSTR LS_ENG(LPCSTR js_string_name) {

	static TCHAR strOut[800]= {""};

	strOut[0]=0;

	if (lpStrings_English == NULL ) {

		

		TCHAR str1[150];
		char str[150];

		lpStrings_English = (char*)0xFFFFFFFF; 

		
		TCHAR path[500];
		GetPodNogamiPath(path, false, NULL);

		
			
		_tcscat(path,TEXT("languages\\english.txt") );		
		

		if ( _taccess(path, 0) ==0 ) {
			
			HANDLE h = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
			DWORD readSz=0, fileSize = GetFileSize(h, NULL);
			lpStrings_English = new char[fileSize+10];
			memset(lpStrings_English, 0, fileSize+5);
			ReadFile(h, lpStrings_English, fileSize, &readSz, NULL);
			CloseHandle(h);

			if (readSz ==0) {
				lpStrings_English = (char*)0xFFFFFFFF; 
				printf_log("LS: error read %s", path);
			}
			
		}	
			else
			lpStrings_English = (char*)0xFFFFFFFF; 

	}

	if (lpStrings_English != (char*)0xFFFFFFFF ) {


		_tcscpy(strOut, _T(""));

		char defString[500] = {0};
		char String[500] = {0};

		memset(String, 0, 490);
		char str[150];

		strcpy(defString, js_string_name); 


		
		if ( strlen(defString) ) {
			
			strcat(defString, " "); 


			const char* lpDef = strstr(lpStrings_English, defString);

			if (!lpDef) {
				defString[ strlen(defString)-1 ] = '\x9';				
				lpDef = strstr(lpStrings_English, defString);
				if (!lpDef) {
					defString[ strlen(defString)-1 ] = '=';				
					lpDef = strstr(lpStrings_English, defString);
				} 
			} 			
			

			if (lpDef ) {
				lpDef++;
				lpDef = strstr(lpDef, "\"");
				if(lpDef) {
					lpDef++;
					const char* strend = strpbrk(lpDef, "\x0D\x0A");  
					if ( strend &&  strend>lpDef && strend-lpDef < 490)
						strncpy(String, lpDef, strend-lpDef-1); 
				}

			}

			

			
			if ( strlen(String) ) {
				char* s1 ;
				while (s1 = strstr(String, "\\n") ){				
					*s1 = ' ';
					*(s1+1) = '\n';

				}

				int i1=strlen(String);

				if (String[i1-1]=='\x22') 
					String[i1-1]=0;
			}


		} else {
			
			printf_log("LS_ENG(%s)",js_string_name );
			return strOut;
			
			

		}



		if ( strlen(String) ) {

#ifdef UNICODE			
			MultiByteToWideChar(CP_ACP, 0, String, -1, strOut, 750);
#else
			strcpy(strOut, String);
#endif
			return strOut;

		}
	}		

	printf_log("LS_ENG(%s).",js_string_name );
	strcpy(strOut, js_string_name);

	return strOut;
}


//  if string not found = "" do not load anything (keep UI variant)
//
void LS_UI(HWND hwnd, int id, LPCTSTR string_name)
{
	LPCTSTR str = LS(string_name);

	if (strlen(str) == 0 ) // string was not found even in English Strings...	
		return;						// return and keep UI string

	::SetDlgItemText(hwnd, id, str);
}

void GetSystemErrorMessage(DWORD GLE, LPTSTR szMessage)
{	
	DWORD ret;
	

   
    ret = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,                               // ignored
            GLE,                     // message id
            0, /*MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)*/   // message language
            szMessage,                  // address of buffer pointer
            199,                                  // minimum buffer size
            NULL );                              // no other arguments

	if (0==ret)
		_stprintf(szMessage, TEXT("Information message: 0x%X "), GLE );

}

#include "psapi.h"

LPCTSTR GetFileVersion_ProductName(DWORD processPID)
{
	static TCHAR productName[101]= {0};

	// Structure used to store enumerated languages and code pages.

	HANDLE processHandle = NULL;
	TCHAR filename[MAX_PATH];

	processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processPID);
	if (processHandle != NULL) {
		if (GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH) == 0) {
			return productName;
		} 
		CloseHandle(processHandle);
	} else {
		return productName;
	}


	DWORD               dwSize              = 0;
	BYTE                *pBlock       = NULL;
	
	
	/*getting the file version info size */
	dwSize = GetFileVersionInfoSize( filename, NULL );
	if ( dwSize == 0 )
	{
		return productName;
	}

	pBlock = new BYTE[ dwSize ]; /*allocation of space for the verison size */

	if ( !GetFileVersionInfo( filename, 0, dwSize, pBlock ) ) /*entering all info     data to pbVersionInfo*/
	{
		delete[] pBlock;
		return productName;
	}



	HRESULT hr;

	UINT	cbTranslate;
	TCHAR SubBlock[100];

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	// Read the list of languages and code pages.

	VerQueryValue(pBlock, 
		TEXT("\\VarFileInfo\\Translation"),
		(LPVOID*)&lpTranslate,
		&cbTranslate);

	// Read the file description for each language and code page.

	int i;
	for( i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
	{
		hr = sprintf(SubBlock, 	TEXT("\\StringFileInfo\\%04x%04x\\ProductName"),
			lpTranslate[i].wLanguage,
			lpTranslate[i].wCodePage);
		if (FAILED(hr))
		{
			// TODO: write error handler.
		}

		LPCTSTR lpBuffer=0;
		UINT dwBytes = 90;

		// Retrieve file description for language and code page "i". 
		VerQueryValue(pBlock, 
			SubBlock, 
			(LPVOID*)&lpBuffer, 
			&dwBytes); 

		if ( _tcslen(lpBuffer) ) 
		{
			_tcscpy(productName, lpBuffer);
			break;
		}
	}

	delete[] pBlock;
	return productName;

}