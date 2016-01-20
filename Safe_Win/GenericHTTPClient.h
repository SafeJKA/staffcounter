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

#ifndef __GENERIC_HTTP_CLIENT
#define __GENERIC_HTTP_CLIENT

//#include <afxwin.h>
#include <windows.h>
#include <tchar.h>
#include <wininet.h>

// use stl
#include <vector>

// PRE-DEFINED CONSTANTS
#define __DEFAULT_AGENT_NAME	_T("KIDLOGGER.WIN(5.6/;p)")

// PRE-DEFINED BUFFER SIZE
#define	__SIZE_HTTP_ARGUMENT_NAME	256
#define __SIZE_HTTP_ARGUMENT_VALUE	1024

#define __HTTP_VERB_GET	"GET"
#define __HTTP_VERB_POST "POST"
#define __HTTP_ACCEPT_TYPE "*/*"
#define __HTTP_ACCEPT "Accept: */*\r\n"
#define __SIZE_HTTP_BUFFER	1900000
#define __SIZE_HTTP_RESPONSE_BUFFER	100000
#define __SIZE_HTTP_HEAD_LINE	2048

#define __SIZE_BUFFER	1024
#define __SIZE_SMALL_BUFFER	256

class GenericHTTPClient {
public:					

	DWORD fileOffsetStopAt; //< the len of the file data that were successfully sent.

	typedef struct __GENERIC_HTTP_ARGUMENT{							// ARGUMENTS STRUCTURE
		TCHAR	szName[__SIZE_HTTP_ARGUMENT_NAME];
		TCHAR	szValue[__SIZE_HTTP_ARGUMENT_VALUE];
		DWORD	dwType;
		BOOL	bContinueSendingExisting;
		DWORD	dwOffset;
		
	} GenericHTTPArgument;

	enum RequestMethod{															// REQUEST METHOD
		RequestUnknown=0,
		RequestGetMethod=1,
		RequestPostMethod=2,
		RequestPostMethodMultiPartsFormData=3
	};

	enum TypePostArgument{													// POST TYPE 
		TypeUnknown=0,
		TypeNormal=1,
		TypeBinary=2
	};

	// CONSTRUCTOR & DESTRUCTOR
	GenericHTTPClient();
	virtual ~GenericHTTPClient();

	TCHAR szAgentName[100];

	static GenericHTTPClient::RequestMethod GetMethod(int nMethod);
	static GenericHTTPClient::TypePostArgument GetPostArgumentType(int nType);

	// Connection handler	
	BOOL Connect(LPCTSTR szAddress, LPCTSTR szAgent = __DEFAULT_AGENT_NAME, unsigned short nPort = INTERNET_DEFAULT_HTTP_PORT, LPCTSTR szUserAccount = NULL, LPCTSTR szPassword = NULL);
	BOOL Close();
	VOID InitilizePostArguments();

	// HTTP Arguments handler	
	VOID AddPostArguments(LPCTSTR szName, DWORD nValue);
	VOID AddPostArguments(LPCTSTR szName, LPCTSTR szValue, BOOL bBinary = FALSE, DWORD offset =0);

	// HTTP Method handler 
	BOOL Request(LPCTSTR szURL, int nMethod = GenericHTTPClient::RequestGetMethod, LPCTSTR szAgent = __DEFAULT_AGENT_NAME);
	BOOL RequestOfURI(LPCTSTR szURI, int nMethod = GenericHTTPClient::RequestGetMethod);
	BOOL Response(PBYTE pHeaderBuffer, DWORD dwHeaderBufferLength, PBYTE pBuffer, DWORD dwBufferLength, DWORD &dwResultSize);	
	LPCTSTR QueryHTTPResponse();
	LPCTSTR QueryHTTPResponseHeader();	

	// General Handler
	DWORD GetLastError();
	LPCTSTR GetContentType(LPCTSTR szName);
	VOID ParseURL(LPCTSTR szURL, LPTSTR szProtocol, LPTSTR szAddress, DWORD &dwPort, LPTSTR szURI);

	
	TCHAR		_error[350]; // last error string 

protected:				
	std::vector<GenericHTTPArgument> _vArguments;				// POST ARGUMENTS VECTOR

	TCHAR		_szHTTPResponseHTML[__SIZE_HTTP_BUFFER+100];		// RECEIVE HTTP BODY
	TCHAR		_szHTTPResponseHeader[__SIZE_HTTP_BUFFER];	// RECEIVE HTTP HEADR

	HINTERNET _hHTTPOpen;				// internet open handle
	HINTERNET _hHTTPConnection;		// internet connection hadle
	HINTERNET _hHTTPRequest;		// internet request hadle

	
	DWORD		_dwError;					// LAST ERROR CODE
	LPCTSTR		_szHost;					 //	 HOST NAME
	DWORD		_dwPort;					//  PORT
	BOOL		_bHTTPS; 

	// HTTP Method handler
	DWORD ResponseOfBytes(PBYTE pBuffer, DWORD dwSize);
	DWORD GetPostArguments(LPTSTR szArguments, DWORD dwLength);
	BOOL RequestPost(LPCTSTR szURI);
	BOOL RequestPostMultiPartsFormData(LPCTSTR szURI);
	BOOL RequestGet(LPCTSTR szURI);
	DWORD AllocMultiPartsFormData(PBYTE &pInBuffer, LPCTSTR szBoundary = "--MULTI-PARTS-FORM-DATA-BOUNDARY-");
	VOID FreeMultiPartsFormData(PBYTE &pBuffer);
	DWORD GetMultiPartsFormDataLength();
};

#endif	// #ifndef __GENERIC_HTTP_CLIENT
