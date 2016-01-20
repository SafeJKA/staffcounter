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
#include "report.h"

#include <winsock2.h>

#include <tchar.h>
#include "common1.h"

#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")

#include "GenericHTTPClient.h"

#include <iostream>
#include <io.h>
#include "common1.h"


report::report()
{
	hFile = 0;
	strcpy(log_file_name, "report.log");
}

report::~report()
{

}


char* report::make_hex(BYTE *inps, int sz)
{
	static char hex_str[500] = {""};
	memset(hex_str,0,400);

	if(!sz)
		return hex_str;

	int k=0;
	for(int i=0; i<sz && i<55; ++i)
	{
		k = _stprintf(hex_str+(i*3),"%%%.2X",inps[i])+1;
	}
	return hex_str;
}


DWORD report::getLastFilePos(LPCTSTR fileNamePath, LPCTSTR type)
{
	char last_file[70] = {"post_last_file_"};
	char last_file_pos[70] = {"post_last_file_pos_"};
	strcat(last_file, type);
	strcat(last_file_pos, type);

	char str_val[500] = {""};
	DWORD last_pos = 0;

	LPCTSTR fileName = strrchr(fileNamePath, '\\');
	if (fileName)
		fileName++;
	else 
		fileName = fileNamePath;

	ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), last_file, str_val, 400);

	if ( stricmp(fileName, str_val) ==0 )
	{
		// get the last pos and return
		last_pos = ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), last_file_pos, 0);

	} else
	{
		if ( _taccess(fileNamePath, 0) == 0 )
		{
			// new file appear - reset upload stats
			WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), last_file, fileName);
			WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), last_file_pos, (DWORD)0);
		}
	}

	return last_pos;
}

void report::setLastFilePos(LPCTSTR fileNamePath, LPCTSTR type, DWORD new_pos)
{
	
	char last_file_pos[50] = {"post_last_file_pos_"};
	
	strcat(last_file_pos, type);	
	// write the last pos and return
	WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), last_file_pos, new_pos);	

	return ;


}


int report::GetInputIdleSeconds()
{
	LASTINPUTINFO lii;
	lii.cbSize = sizeof(LASTINPUTINFO);

	GetLastInputInfo( &lii);

	DWORD time_span = GetTickCount() - lii.dwTime;

	return time_span / 1000;
}


// strSettingsResp - a settings str returned from the server
//   example: OK. LOG-CONFIG: url APP-CONFIG:allow-app-stop:1, display-mode:1
//
// 
void report::ScanAndUpdateSettings(LPCTSTR strSettingsResp)
{
	bool bSettingsWasChaged = false; // TRUE - if new settings is changed

	int bWebSites = false;
	int bFiles = false;

	int nAllowAppStop = true;
	int nOpenMonitoring = false;

	LPCTSTR pSetting;
	TCHAR strSettingVal[100]= {""};


	pSetting = strstr(strSettingsResp, "url");	
	if (pSetting) bWebSites = true;

	pSetting = strstr(strSettingsResp, "files");	// log window captions?
	if (pSetting) bFiles = true;

	pSetting = strstr(strSettingsResp, "display-mode");	
	memset(strSettingVal, 20, 0);
	if (pSetting) strncpy(strSettingVal, pSetting+13, 2);
	nOpenMonitoring = atoi(strSettingVal);

	pSetting = strstr(strSettingsResp, "allow-app-stop");	
	memset(strSettingVal, 20, 0);
	if (pSetting) strncpy(strSettingVal, pSetting+15, 2);
	nAllowAppStop = atoi(strSettingVal);
	

	BOOL bOldSetting;
	int bOldInt;

	bOldSetting = ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "LogURLs", (DWORD)0); 
	if ( bOldSetting != bWebSites)
	{
		bSettingsWasChaged = true;
		WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "LogURLs", bWebSites);
	}

	// log Window captions?
	bOldSetting = ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "UsbMonitor", (DWORD)0); 
	if ( bOldSetting != bFiles)
	{
		bSettingsWasChaged = true;
		WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "UsbMonitor", bFiles );
	}


	bOldInt = ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "display-mode", (DWORD)0); 
	if ( bOldInt != nOpenMonitoring)
	{
		bSettingsWasChaged = true;
	    WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "display-mode", (DWORD)nOpenMonitoring );
	}

	bOldInt = ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "allow-app-stop", (DWORD)0); 
	if ( bOldInt != nAllowAppStop)
	{
		bSettingsWasChaged = true;
		WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "allow-app-stop", (DWORD)nAllowAppStop );
	}

	 if (bSettingsWasChaged == true)
	 {		 
		 WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter") , "new-settings", (DWORD)1);
	 }
	
}

//*********************************************************************

int report::send_report(LPCTSTR strCommand, LPCTSTR strDevid, LPCTSTR strName, LPCTSTR strFilename)
{	

	char subject[200] = {"StaffCounter Report. "};
	char email_text[100] = {"Sent automatically - to stop it configure StaffCounter. "};
	BOOL bError;

	
	int attempts =0;
	HANDLE hLogWrite_Mutex; 

	
	char comp_name[50]= {"PC"};
		 DWORD dwsz = 40;

		

send_again:

	if (attempts++ > 3)
	{
		printf_log("exit on 4rd attempt");
		return 1;
	}

	SYSTEMTIME	time; 
	char date[190];
			
	GetLocalTime( &time);	
	sprintf(date, "%02d - %02d:%02d:%02d", time.wDay, time.wHour, time.wMinute, time.wSecond);
	
	printf_log("input params (%s %s %s %s). attempt =%d ", strCommand, strDevid, strName, strFilename, attempts);
	

	printf("\n\n\n");

		// *****************************************************************************************************

		if ( strstr(strCommand, "web_create_id") )
		{
			
				char szUploadURL[200] = {""};
				char szEmail[200] = {""};

				char szDevName[200] = {""};				
				char szDevNameUtf8hex[250];
				WCHAR _utf8_buff[200];

				ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "UploadURL", szUploadURL, 190);

				if ( strlen(szUploadURL) ==0 )
					strcpy(szUploadURL, "http://data.StaffCounter.net/upload-v2/device/" );
				else
					strcat(szUploadURL, "/device/");

				strcpy(szEmail, strDevid); ///argv[1]
				strcpy(szDevName, strName); // argv[2] // UTF8 string 

				MultiByteToWideChar(CP_ACP , 0, szDevName, -1, _utf8_buff, 190);
				WideCharToMultiByte(CP_UTF8, 0, _utf8_buff, -1, szDevName, 190, NULL, NULL);
				LPCTSTR hexret = make_hex((LPBYTE)szDevName, strlen(szDevName) * sizeof(char) );
				strncpy(szDevNameUtf8hex, hexret, 220);
				
				GenericHTTPClient *pClient=new GenericHTTPClient();

				pClient->InitilizePostArguments();

				pClient->AddPostArguments(_T("device"), "");				
				pClient->AddPostArguments(_T("content"), "add-dev"); 
				pClient->AddPostArguments(_T("email"), szEmail); 

				pClient->AddPostArguments(_T("DeviceName"), szDevNameUtf8hex); 

				LPCTSTR platform = get_windows_platform_info();
				if ( strlen(platform ))
					pClient->AddPostArguments(_T("client-ver"), platform );
				else
					pClient->AddPostArguments(_T("client-ver"), "Windows" );


				pClient->AddPostArguments(_T("app-ver"), _T("5.8.0"));			

				GetLocalTime( &time);	
				sprintf(date, "%02d/%02d/%d %02d:%02d:%02d", time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
				pClient->AddPostArguments(_T("client-date-time"), date);


					

				printf_log("Create Dev Request... %s (%s)", date, szUploadURL);

				char post_info[1700] = {""};

				_tcscpy(pClient->szAgentName, "StaffCounter.5.7.5. WIN.");				

			

				if(pClient->Request(szUploadURL,  GenericHTTPClient::RequestPostMethod) )
				{  			
					printf_log("11");

					LPCTSTR szResult = pClient->QueryHTTPResponse();
					char resp[3000] = {""};

					strncpy(resp, szResult, 290);
					printf_log(resp);

					if ( strstr(resp, "Ok") || strstr(szResult, "OK"))
					{
						printf_log("Sent successfully - ");
						printf_log(resp );

						LPCTSTR dsn = strstr(resp, "DSN:");
						TCHAR dsn_id[100]= {""};
						if (dsn)
							strcpy(dsn_id, dsn + 4);

						WriteReg(HKEY_CURRENT_USER, "Software\\StaffCounter", "smtp_to", dsn_id);
						WriteReg(HKEY_LOCAL_MACHINE, "Software\\StaffCounter", "smtp_to", dsn_id);

						TCHAR cmdline[500];
						strcpy(cmdline, dsn_id);
						strcat(cmdline, " Report web_post ");						
						WriteReg(HKEY_CURRENT_USER, "Software\\StaffCounter", "report_cmdline", cmdline);
						WriteReg(HKEY_LOCAL_MACHINE, "Software\\StaffCounter", "report_cmdline", cmdline);

						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", szResult);

					}

					if (strstr(resp, "ERR")   ) // device not found!
					{	
						printf_log(resp);
						strcat(resp, "\n");
						strcat(resp, szUploadURL);
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", resp);
						return 0;
					}
					
				} else {
					
					printf_log("Connection error ");									
					WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", "Connection Error");				
				}
				printf_log("done\n\n");
				return 0;


		}

		// *****************************************************************************************************


		if ( strstr(strCommand, "web_get_settings") )
		{
			int dev_not_found_ret = ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "dev_not_found", 0);
			if (dev_not_found_ret >10)
			{
				printf_log("device not found error many times.");
				WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", "Device_id not found. Or software is outdated. stop sending logs");
				return 0;
			}

				char szUploadURL[200] = {""};
				char szDeviceID[200] = {""};

				char szDevName[200] = {""};				
				char szDevNameUtf8hex[250];
				WCHAR _utf8_buff[200];

				ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "UploadURL", szUploadURL, 190);

				if ( strlen(szUploadURL) ==0 )
					strcpy(szUploadURL, "http://data.StaffCounter.net/upload-v2/config/" );
				else
					strcat(szUploadURL, "/config/");

				strcpy(szDeviceID, strDevid);				
				
				GenericHTTPClient *pClient=new GenericHTTPClient();

				pClient->InitilizePostArguments();

				pClient->AddPostArguments(_T("device"), szDeviceID);				
				pClient->AddPostArguments(_T("content"), "get-dev-config"); 
				

				LPCTSTR platform = get_windows_platform_info();
				if ( strlen(platform ))
					pClient->AddPostArguments(_T("client-ver"), platform );
				else
					pClient->AddPostArguments(_T("client-ver"), "Windows" );


				pClient->AddPostArguments(_T("app-ver"), _T("5.8.0"));			

				GetLocalTime( &time);	
				sprintf(date, "%02d/%02d/%d %02d:%02d:%02d", time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
				pClient->AddPostArguments(_T("client-date-time"), date);

				printf_log("Get Config Request... %s (%s)", date, szUploadURL);

				char post_info[1700] = {""};
				_tcscpy(pClient->szAgentName, "StaffCounter.5.7.5. WIN.");							

				if(pClient->Request(szUploadURL,  GenericHTTPClient::RequestPostMethod) )
				{  			
					LPCTSTR szResult = pClient->QueryHTTPResponse();
					char resp[3000] = {""};

					strncpy(resp, szResult, 290);
					printf_log(resp);

					if ( strstr(resp, "Ok.") || strstr(szResult, "OK."))
					{						
						printf_log("Sent successfully - ");
						printf_log(resp );
						ScanAndUpdateSettings(resp);	
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", szResult);						
					}

					if (strstr(resp, "ERR")   ) // device not found!
					{	
						printf_log(resp );
						strcat(resp, "\n");
						strcat(resp, szUploadURL);
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", resp);						
						return 0;
					}
					
				} else {
					
					printf_log("Connection error ");									
					WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", "Connection Error");			
				}
				printf_log("done\n\n");
				return 0;

		}

		// *****************************************************************************************************


		if ( strstr(strCommand, "web_post") )
		{
			

			try {

				int dev_not_found_ret = ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "dev_not_found", 0);
				if (dev_not_found_ret >10)
				{
					printf_log("device not found many times error.");
					WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", "Device_id not found. Or software is outdated. stop sending logs");
					return 0;
				}


				char file_name[500] = {""};
				char szUploadURL[200] = {""};

				ReadReg(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\StaffCounter"), "UploadURL", szUploadURL, 190);

				if ( strlen(szUploadURL) ==0 )
					strcpy(szUploadURL, "http://data.staffcounter.net/upload-v2/device/" );

				
				strcpy(file_name, strFilename);


				bool send_file_block_from_last_position = true;

				/*if ( argc > 6 && strstr(strCommand, "web_post_clear") )
				{
					send_file_block_from_last_position = false;
				}*/

				GenericHTTPClient *pClient=new GenericHTTPClient();

				pClient->InitilizePostArguments();

				pClient->AddPostArguments(_T("device"), strDevid);

				char file_type[50] = {""};
		
				if ( strstr(file_name, ".htm") )
					strcpy(file_type, "HTML");

				if ( strstr(file_name, "test_upload") )
				{
					// this is test report 
					pClient->AddPostArguments(_T("content"), "HTML"); 
					strcpy(file_type, "txt");

				} else			
					pClient->AddPostArguments(_T("content"), file_type);


				// Check if the file_type is rejected for today?
				char check_reject_type[70] = {"rejected_"};
				char reject_date[70] = {""};
				strcat(check_reject_type, file_type);
				ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), check_reject_type, reject_date, 60);

				if (strlen(reject_date))
				{
					GetLocalTime( &time);	
					sprintf(date, "%02d-%02d-%d", time.wDay, time.wMonth, time.wYear);

					if ( stricmp(date, reject_date) ==0) 
					{
						// file is rejected!
						printf_log("skip file. rejected");
						return 0;
						
					} else 
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), check_reject_type, (LPCSTR)NULL);
					
				}

				LPCTSTR platform = get_windows_platform_info();
				if ( strlen(platform ))
					pClient->AddPostArguments(_T("client-ver"), platform );
				else
					pClient->AddPostArguments(_T("client-ver"), "Windows" );


				pClient->AddPostArguments(_T("app-ver"), _T("5.9.0"));			

				GetLocalTime( &time);	
				sprintf(date, "%02d/%02d/%d %02d:%02d:%02d", time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
				pClient->AddPostArguments(_T("client-date-time"), date);


				DWORD last_pos = getLastFilePos(file_name, file_type);

				if ( _taccess(file_name, 0) != 0 )
				{
					printf_log("wait for %s", file_name);
					Sleep(4000); 

					if ( _taccess(file_name, 0) != 0 )											
						Sleep(4000); 

					if ( _taccess(file_name, 0) != 0 )											
						Sleep(4000); 
					
				}

				HANDLE hFile=::CreateFile(file_name, 
					FILE_READ_ATTRIBUTES, // desired access
					FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
					NULL, // security attribute
					OPEN_EXISTING, // create disposition
					FILE_ATTRIBUTE_NORMAL, // flag and attributes
					NULL); // template file handle

				if (hFile == INVALID_HANDLE_VALUE)
				{
					// file does not exist for some reason.										
					printf_log("error Open: %s (GLE:%d) ", file_name, GetLastError() );

					WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", "Error while opening Log file");					
					return 0;
				}

				DWORD	dwFileSize= ::GetFileSize(hFile, NULL);
				CloseHandle(hFile);

				printf_log("file size %d", dwFileSize);

				if (last_pos == dwFileSize) // if file size didn't changed - there is no activity - so no need to send it. 
					return 0;

				pClient->AddPostArguments(_T("file"), file_name, TRUE, last_pos);			

				printf_log("Begin Request... %s last_pos:%d size:%d (%s)", date, last_pos, dwFileSize, szUploadURL);

				char post_info[1700] = {""};

				_tcscpy(pClient->szAgentName, "StaffCounter.6.1 WIN. ID:");
				_tcscat(pClient->szAgentName, strDevid);

				if(pClient->Request(szUploadURL,  GenericHTTPClient::RequestPostMethodMultiPartsFormData) )
				{        
					LPCTSTR szResult = pClient->QueryHTTPResponse();
					char resp[3000] = {""};

					strncpy(resp, szResult, 290);
					printf_log(resp);

					if ( strstr(szResult, "Ok") || strstr(szResult, "OK"))
					{
						setLastFilePos("", file_type, pClient->fileOffsetStopAt);					
						printf_log("Sent successfully - up to %d", pClient->fileOffsetStopAt);
						printf_log(szResult );

						
						if ( strstr(szResult, "LOG-CONFIG:") )
						{
							ScanAndUpdateSettings(resp);	
						}
					}

					if (strstr(szResult, "BAD_DEV.") || strstr(szResult, "not found")  ) // device not found!
					{
						dev_not_found_ret = ReadReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "dev_not_found", 0);
						dev_not_found_ret++;
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "dev_not_found", dev_not_found_ret);						

						sprintf(post_info, "Connection Error: %s ", resp);
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", post_info);

						printf_log( resp);
						return 0;

					}

					if (strstr(szResult, "UPDATE.")  ) // server needs new version of software.
					{						
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "dev_not_found", 20);												
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", "Please update your client software. Files logging refused.");
						printf_log( pClient->_error);
						return 0;

					}

					if ( strstr(szResult, "REJECT.") )
					{						

						GetLocalTime( &time);	
						sprintf(reject_date, "%02d-%02d-%d", time.wDay, time.wMonth, time.wYear);
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), check_reject_type, reject_date);

						printf_log("file rejected");

						sprintf(post_info, "file: %s.\nPlease update your Account on StaffCounter.net ", file_name);
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_file", post_info);

						return 0;

					}

					WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_resp", resp);

					sprintf(post_info, "file: %s  offset: %d  len: %d", file_name, last_pos, pClient->fileOffsetStopAt-last_pos);
					WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_file", post_info);

					if ( strstr(szResult, "Ok") == NULL && strstr(szResult, "OK") == NULL)
					{
						printf_log("An error occurred: \n");
						LPCTSTR just_name = strrchr(file_name, '\\');
						if (!just_name)
							just_name = file_name;
						sprintf(post_info, "Web upload status: %s (file: %s  offset: %d  len: %d)", resp, just_name, last_pos, pClient->fileOffsetStopAt);
						WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", post_info);
										
						printf_log(post_info);
					}

				} else {
					printf_log("Connection error ");

					// error
					LPCTSTR just_name = strrchr(file_name, '\\');
					if (!just_name)
						just_name = file_name;

					sprintf(post_info, "Connection Error: %s (file: %s)", pClient->_error, just_name);
					WriteReg(HKEY_CURRENT_USER, TEXT("SOFTWARE\\StaffCounter"), "web_post_error", post_info);

					printf_log( pClient->_error);
				}
				printf_log("done\n\n");
				return 0;

			}

			catch (...)
			{

				printf_log("exception web");

			}
		} // end of web post 

		bool rohos_report = false;


	goto exit_0; // success
	
exit:
        return 1;   // Error
exit_0:
        return 0; // Success

}

