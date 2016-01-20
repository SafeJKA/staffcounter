// GenericHTTPClient.cpp: implementation of the GenericHTTPClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GenericHTTPClient.h"
#include <winreg.h>

void printf_log(LPCTSTR szData, ...);

GenericHTTPClient::GenericHTTPClient(){
	_hHTTPOpen=_hHTTPConnection=_hHTTPRequest=NULL;
	_tcscpy(szAgentName, "StaffCounter.6.1. WIN. ID:0)");
	_error[0]=0;
	_bHTTPS = false; 
}

GenericHTTPClient::~GenericHTTPClient(){
	_hHTTPOpen=_hHTTPConnection=_hHTTPRequest=NULL;
}

GenericHTTPClient::RequestMethod GenericHTTPClient::GetMethod(int nMethod){	
	return nMethod<=0 ? GenericHTTPClient::RequestGetMethod : static_cast<GenericHTTPClient::RequestMethod>(nMethod);
}

GenericHTTPClient::TypePostArgument GenericHTTPClient::GetPostArgumentType(int nType){
	return nType<=0 ? GenericHTTPClient::TypeNormal : static_cast<GenericHTTPClient::TypePostArgument>(nType);
}

BOOL GenericHTTPClient::Connect(LPCTSTR szAddress, LPCTSTR szAgent, unsigned short nPort, LPCTSTR szUserAccount, LPCTSTR szPassword){

	_hHTTPOpen=::InternetOpen(szAgentName,												// agent name
												INTERNET_OPEN_TYPE_PRECONFIG,	// proxy option
												"",														// proxy
												"",												// proxy bypass
												0);					// flags

	if(!_hHTTPOpen){
		_dwError=::GetLastError();
		sprintf(_error, "InternetOpen %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}	

	_hHTTPConnection=::InternetConnect(	_hHTTPOpen,	// internet opened handle
															szAddress, // server name
															nPort, // ports
															szUserAccount, // user name
															szPassword, // password 
															INTERNET_SERVICE_HTTP, // service type
															INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE,	// service option																														
															0);	// context call-back option

	if(!_hHTTPConnection){		
		_dwError=::GetLastError();
		sprintf(_error, "InternetConnect %d", _dwError);
		::CloseHandle(_hHTTPOpen);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	if(::InternetAttemptConnect(NULL)!=ERROR_SUCCESS){		
		_dwError=::GetLastError();

		sprintf(_error, "InternetAttemptConnect %d", _dwError);
		::CloseHandle(_hHTTPConnection);
		::CloseHandle(_hHTTPOpen);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	return TRUE;															
}

BOOL GenericHTTPClient::Close(){

	if(_hHTTPRequest)
		::InternetCloseHandle(_hHTTPRequest);

	if(_hHTTPConnection)
		::InternetCloseHandle(_hHTTPConnection);

	if(_hHTTPOpen)
		::InternetCloseHandle(_hHTTPOpen);




	return TRUE;
}

BOOL GenericHTTPClient::Request(LPCTSTR szURL, int nMethod, LPCTSTR szAgent){

	BOOL bReturn=TRUE;
	DWORD dwPort=0;
	TCHAR szProtocol[__SIZE_BUFFER]="";
	TCHAR szAddress[__SIZE_BUFFER]="";	
	TCHAR szURI[__SIZE_BUFFER]="";
	DWORD dwSize=0;

	if ( _tcsstr(szURL, "https:") ) 
	{
		_bHTTPS  = true;		
	}

	ParseURL(szURL, szProtocol, szAddress, dwPort, szURI);

	if(Connect(szAddress, szAgentName, dwPort)){

		if(!RequestOfURI(szURI, nMethod)){
			bReturn=FALSE;
		}else{
			

			if(!Response((PBYTE)_szHTTPResponseHeader, __SIZE_HTTP_BUFFER, (PBYTE)_szHTTPResponseHTML, __SIZE_HTTP_BUFFER, dwSize)){
				//printf(_szHTTPResponseHeader);
				//printf(_szHTTPResponseHTML);
				bReturn=FALSE;		
			}
		}
		
		Close();
	}else{
		bReturn=FALSE;
	}

	return bReturn;
}

BOOL GenericHTTPClient::RequestOfURI(LPCTSTR szURI, int nMethod){

	BOOL bReturn=TRUE;
	

	try{
		switch(nMethod){
		case	GenericHTTPClient::RequestGetMethod:
		default:
			bReturn=RequestGet(szURI);
			break;
		case	GenericHTTPClient::RequestPostMethod:		
			bReturn=RequestPost(szURI);
			
			break;
		case	GenericHTTPClient::RequestPostMethodMultiPartsFormData:
			bReturn=RequestPostMultiPartsFormData(szURI);
			break;
		}
	} catch(...){



#ifdef	_DEBUG
		//TRACE("\nEXCEPTION\n");
		//TCHAR szERROR[1024];
		//e->GetErrorMessage(szERROR, 1024);
		//TRACE(szERROR);
		
#endif
		
	}
	


	return bReturn;
}

BOOL GenericHTTPClient::RequestGet(LPCTSTR szURI){

	CONST TCHAR *szAcceptType=__HTTP_ACCEPT_TYPE;

	_hHTTPRequest=::HttpOpenRequest(	_hHTTPConnection,
															__HTTP_VERB_GET, // HTTP Verb
															szURI, // Object Name
															HTTP_VERSION, // Version
															"", // Reference
															&szAcceptType, // Accept Type
															INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE,
															0); // context call-back point

	if(!_hHTTPRequest){
		_dwError=::GetLastError();
		sprintf(_error, "HttpOpenRequest %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	// REPLACE HEADER
	if(!::HttpAddRequestHeaders( _hHTTPRequest, __HTTP_ACCEPT, _tcslen(__HTTP_ACCEPT), HTTP_ADDREQ_FLAG_REPLACE)){
		_dwError=::GetLastError();
		sprintf(_error, "HttpAddRequestHeaders %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	// SEND REQUEST
	if(!::HttpSendRequest( _hHTTPRequest,	// handle by returned HttpOpenRequest
									NULL, // additional HTTP header
									0, // additional HTTP header length
									NULL, // additional data in HTTP Post or HTTP Put
									0) // additional data length
									){
		_dwError=::GetLastError();
		sprintf(_error, "HttpSendRequest %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	return TRUE;
}

BOOL GenericHTTPClient::RequestPost(LPCTSTR szURI){

	CONST TCHAR *szAcceptType=__HTTP_ACCEPT_TYPE;
	TCHAR			szPostArguments[__SIZE_BUFFER+5]="";
	LPCTSTR szContentType=TEXT("Content-Type: application/x-www-form-urlencoded\r\n");

	DWORD dwPostArgumentsLegnth=GetPostArguments(szPostArguments, __SIZE_BUFFER);

	//if (_bHTTPS)
	//	MessageBox(0, "11", "22", 0);
	
	_hHTTPRequest=::HttpOpenRequest(	_hHTTPConnection,
															__HTTP_VERB_POST, // HTTP Verb
															szURI, // Object Name
															HTTP_VERSION, // Version
															"", // Reference
															&szAcceptType, // Accept Type
															INTERNET_FLAG_KEEP_CONNECTION | 
															INTERNET_FLAG_NO_CACHE_WRITE | 
															INTERNET_FLAG_FORMS_SUBMIT |
															INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | // Disables the ability of the Win32 Internet functions to detect this special type of redirect
                                                            INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |   // Disables the ability of the Win32 Internet functions to detect this special type of redirect
                                                            INTERNET_FLAG_IGNORE_CERT_CN_INVALID |   // Ignore invalid certificate
                                                            INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | // Ignore invalid certificate date
															(  _bHTTPS ? INTERNET_FLAG_SECURE : 0),
															0); // context call-back point

	

	if(!_hHTTPRequest){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE("\n%d : %s\n", _dwError, reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	

	// REPLACE HEADER
	if(!::HttpAddRequestHeaders( _hHTTPRequest, __HTTP_ACCEPT, _tcslen(__HTTP_ACCEPT), HTTP_ADDREQ_FLAG_REPLACE)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE("\n%d : %s\n", _dwError, reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}


	

	// SEND REQUEST
	if(!::HttpSendRequest( _hHTTPRequest,	// handle by returned HttpOpenRequest
									szContentType, // additional HTTP header
									_tcslen(szContentType), // additional HTTP header length
									(LPVOID)szPostArguments, // additional data in HTTP Post or HTTP Put
									_tcslen(szPostArguments)) // additional data length
									){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE("\n%d : %s\n", _dwError, reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	

	return TRUE;
}

BOOL GenericHTTPClient::RequestPostMultiPartsFormData(LPCTSTR szURI){

	CONST TCHAR *szAcceptType=__HTTP_ACCEPT_TYPE;	
	LPCTSTR szContentType=TEXT("Content-Type: multipart/form-data; boundary=--MULTI-PARTS-FORM-DATA-BOUNDARY\r\n");		
	
	// ALLOCATE POST MULTI-PARTS FORM DATA ARGUMENTS
	PBYTE pPostBuffer=NULL;
	DWORD dwPostBufferLength=AllocMultiPartsFormData(pPostBuffer, "--MULTI-PARTS-FORM-DATA-BOUNDARY");

	if (dwPostBufferLength==0) // there is no file to sent.
	{
		return TRUE;
	}
	//HttpOpenRequestEx

	
	_hHTTPRequest=::HttpOpenRequest(	_hHTTPConnection,
															__HTTP_VERB_POST, // HTTP Verb
															szURI, // Object Name
															HTTP_VERSION, // Version
															"", // Reference
															&szAcceptType, // Accept Type
															INTERNET_FLAG_KEEP_CONNECTION | 
															INTERNET_FLAG_NO_CACHE_WRITE | 
															INTERNET_FLAG_FORMS_SUBMIT|
															INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | // Disables the ability of the Win32 Internet functions to detect this special type of redirect
                                                            INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |   // Disables the ability of the Win32 Internet functions to detect this special type of redirect
                                                            INTERNET_FLAG_IGNORE_CERT_CN_INVALID |   // Ignore invalid certificate
                                                            INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | // Ignore invalid certificate date
															//INTERNET_FLAG_SECURE ,
															(  _bHTTPS ? INTERNET_FLAG_SECURE : 0) , // flags
															0); // context call-back point
	if(!_hHTTPRequest){
		_dwError=::GetLastError();
		sprintf(_error, "HttpOpenRequest %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	DWORD dwTimout =0;
	DWORD dwLen = sizeof(DWORD);
	//InternetQueryOption(_hHTTPRequest, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &dwTimout, &dwLen );

	//sprintf(_error, "INTERNET_OPTION_CONNECT_TIMEOUT %X", dwTimout);
	

	dwTimout =  1000 * 1 * 60; // 2 minutes;
	InternetSetOption(_hHTTPRequest, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &dwTimout, sizeof(DWORD) );

	// REPLACE HEADER
	if(!::HttpAddRequestHeaders( _hHTTPRequest, __HTTP_ACCEPT, _tcslen(__HTTP_ACCEPT), HTTP_ADDREQ_FLAG_REPLACE)){
		_dwError=::GetLastError();
		sprintf(_error, "HttpAddRequestHeaders %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	if(!::HttpAddRequestHeaders( _hHTTPRequest, szContentType, _tcslen(szContentType), HTTP_ADDREQ_FLAG_ADD_IF_NEW)){
		_dwError=::GetLastError();
		sprintf(_error, "HttpAddRequestHeaders %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	TCHAR	szContentLength[__SIZE_BUFFER]="";
	::ZeroMemory(szContentLength, __SIZE_BUFFER);

	_stprintf( szContentLength, "Content-Length: %d\r\n", dwPostBufferLength);

	if(!::HttpAddRequestHeaders( _hHTTPRequest, szContentLength, _tcslen(szContentLength), HTTP_ADDREQ_FLAG_ADD_IF_NEW)){
		_dwError=::GetLastError();
		sprintf(_error, "HttpAddRequestHeaders %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

#ifdef	_DEBUG
	TCHAR	szHTTPRequest[__SIZE_HTTP_BUFFER]="";
	DWORD	dwHTTPRequestLength=__SIZE_HTTP_BUFFER;

	::ZeroMemory(szHTTPRequest, __SIZE_HTTP_BUFFER);
	if(!::HttpQueryInfo(_hHTTPRequest, HTTP_QUERY_RAW_HEADERS_CRLF, szHTTPRequest, &dwHTTPRequestLength, NULL)){
		_dwError=::GetLastError();
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE("\n%d : %s\n", ::GetLastError(), reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
		//return FALSE;
	}

	//TRACE(szHTTPRequest);
#endif

	// SEND REQUEST WITH HttpSendRequestEx and InternetWriteFile
	INTERNET_BUFFERS InternetBufferIn={0};
	InternetBufferIn.dwStructSize=sizeof(INTERNET_BUFFERS);
	InternetBufferIn.Next=NULL;	
	
	if(!::HttpSendRequestEx(_hHTTPRequest, &InternetBufferIn, NULL, HSR_INITIATE, 0)){
		_dwError=::GetLastError();
		sprintf(_error, "HttpSendRequestEx %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		//TRACE(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	DWORD dwOutPostBufferLength=0;
	if(!::InternetWriteFile(_hHTTPRequest, pPostBuffer, dwPostBufferLength, &dwOutPostBufferLength)){
		_dwError=::GetLastError();
		sprintf(_error, "InternetWriteFile %d", _dwError);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	if(!::HttpEndRequest(_hHTTPRequest, NULL, HSR_INITIATE, 0)){
		// 2F88 (12168)- ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION 

		_dwError=::GetLastError();
		sprintf(_error, "HttpEndRequest %d", _dwError);

#ifdef	_DEBUG
		TCHAR     *lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  800,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	// FREE POST MULTI-PARTS FORM DATA ARGUMENTS
	FreeMultiPartsFormData(pPostBuffer);

	return TRUE;
}

DWORD GenericHTTPClient::ResponseOfBytes(PBYTE pBuffer, DWORD dwSize){

	static DWORD dwBytes=0;

	if(!_hHTTPRequest){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return 0;
	}

	::ZeroMemory(pBuffer, dwSize);
	if(!::InternetReadFile(	_hHTTPRequest,
									pBuffer,
									dwSize,
									&dwBytes)){
		_dwError=::GetLastError();
		sprintf(_error, "HttpEndRequest %d", _dwError);

#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return 0;
	}

	return dwBytes;
}

BOOL GenericHTTPClient::Response(PBYTE pHeaderBuffer, DWORD dwHeaderBufferLength, PBYTE pBuffer, DWORD dwBufferLength, DWORD &dwResultSize){

	BYTE pResponseBuffer[__SIZE_BUFFER]="";	
	DWORD dwNumOfBytesToRead=0;

	if(!_hHTTPRequest){
		_dwError=::GetLastError();
#ifdef	_DEBUG		
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	::ZeroMemory(pBuffer, dwBufferLength);
	dwResultSize=0;
	while((dwNumOfBytesToRead=ResponseOfBytes(pResponseBuffer, __SIZE_BUFFER))!=NULL && dwResultSize<dwBufferLength){
		::CopyMemory( (pBuffer+dwResultSize), pResponseBuffer, dwNumOfBytesToRead);		
		dwResultSize+=dwNumOfBytesToRead;
	}

	::ZeroMemory(pHeaderBuffer, dwHeaderBufferLength);
	if(!::HttpQueryInfo(_hHTTPRequest, HTTP_QUERY_RAW_HEADERS_CRLF, pHeaderBuffer, &dwHeaderBufferLength, NULL)){
		_dwError=::GetLastError();
		sprintf(_error, "HttpQueryInfo %d", _dwError);

#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPTSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	return (dwResultSize? TRUE: FALSE);
}

VOID GenericHTTPClient::AddPostArguments(LPCTSTR szName, LPCTSTR szValue, BOOL bBinary, DWORD offset)
{
	GenericHTTPArgument	arg;
	::ZeroMemory(&arg, sizeof(arg));

	_tcscpy(arg.szName, szName);
	_tcscpy(arg.szValue, szValue);

	if(!bBinary)
		arg.dwType = GenericHTTPClient::TypeNormal;
	else
	{
		arg.dwType = GenericHTTPClient::TypeBinary;			
	}	arg.dwOffset = offset;

	_vArguments.push_back(arg);

	return;
}

VOID GenericHTTPClient::AddPostArguments(LPCTSTR szName, DWORD nValue){

	GenericHTTPArgument	arg;
	::FillMemory(&arg, sizeof(arg), 0x00);

	_tcscpy(arg.szName, szName);
	_stprintf(arg.szValue, "%d", nValue);
	arg.dwType = GenericHTTPClient::TypeNormal;

	_vArguments.push_back(arg);

	return;
}

DWORD GenericHTTPClient::GetPostArguments(LPTSTR szArguments, DWORD dwLength){

	std::vector<GenericHTTPArgument>::iterator itArg;

	::ZeroMemory(szArguments, dwLength);
	for(itArg=_vArguments.begin(); itArg<_vArguments.end(); ){
		_tcscat(szArguments, itArg->szName);
		_tcscat(szArguments, "=");
		_tcscat(szArguments, itArg->szValue);

		if((++itArg)<_vArguments.end()){
			_tcscat(szArguments, "&");
		}
	}

	*(szArguments+dwLength)='\0';

	return _tcslen(szArguments);
}


VOID GenericHTTPClient::InitilizePostArguments(){
	_vArguments.clear();
	return;
}

VOID GenericHTTPClient::FreeMultiPartsFormData(PBYTE &pBuffer){

	if(pBuffer){
		::LocalFree(pBuffer);
		pBuffer=NULL;
	}

	return;
}

DWORD GenericHTTPClient::AllocMultiPartsFormData(PBYTE &pInBuffer, LPCTSTR szBoundary){

	if(pInBuffer){
		::LocalFree(pInBuffer);
		pInBuffer=NULL;
	}

	DWORD  data_len = GetMultiPartsFormDataLength();
	//printf_log("dlen %d", data_len);
	pInBuffer=(PBYTE)::LocalAlloc(LPTR, data_len);	
	std::vector<GenericHTTPArgument>::iterator itArgv;

	DWORD dwPosition=0;
	DWORD dwBufferSize=0;

	for(itArgv=_vArguments.begin(); itArgv<_vArguments.end(); ++itArgv){

		PBYTE pBuffer=NULL;

		// SET MULTI_PRATS FORM DATA BUFFER
		switch(itArgv->dwType){
		case	GenericHTTPClient::TypeNormal:
			pBuffer=(PBYTE)::LocalAlloc(LPTR, __SIZE_HTTP_HEAD_LINE*4);

			_stprintf(	(TCHAR*)pBuffer,							
							"--%s\r\n"
							"Content-Disposition: form-data; name=\"%s\"\r\n"
							"\r\n"
							"%s\r\n",
							szBoundary,
							itArgv->szName,
							itArgv->szValue);

			dwBufferSize=_tcslen((TCHAR*)pBuffer);

			break;
		case	GenericHTTPClient::TypeBinary:
			DWORD	dwNumOfBytesToRead=0;
			DWORD	dwTotalBytes=0;

			HANDLE hFile=::CreateFile(itArgv->szValue, 
									GENERIC_READ, // desired access
									FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
									NULL, // security attribute
									OPEN_EXISTING, // create disposition
									FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // flag and attributes
									NULL); // template file handle

			DWORD	dwSize =0;

			if (hFile == INVALID_HANDLE_VALUE)
			{

			} else
			dwSize=::GetFileSize(hFile, NULL);

			fileOffsetStopAt = dwSize; // remember the offset that we stop at (the client will remember this position and put it into itArgv->dwOffset next time)		

			if (dwSize >= itArgv->dwOffset && itArgv->dwOffset >0) // begin read file from some offset (this is the last time we stop at)
			{			

				SetFilePointer(hFile, itArgv->dwOffset, NULL, FILE_BEGIN);
				dwSize -= itArgv->dwOffset;
			}

			DWORD actual_size = dwSize;
			if (dwSize > 600 * 1024) // more than 800 kb
				dwSize = 600 * 1024 ;			

			pBuffer=(PBYTE)::LocalAlloc(LPTR, __SIZE_HTTP_HEAD_LINE*3+dwSize+3000);
			BYTE	pBytes[__SIZE_BUFFER+1]="";

// Content-Disposition: inline; filename*0*=utf-8''%D0%BF%D0%BE%D0%BA%D0%BE%D0%B2%D0; filename*1*=%B1%D0%BE%D0%B9%D1%81%D0%BA %D0%B8.png
// Content-Type: image/png; name*0*=utf-8''%D0%BF%D0%BE%D0%BA%D0%BE%D0%B2%D0; name*1*=%B1%D0%BE%D0%B9%D1%81%D0%BA%D 0%B8.png
			//
			
			_stprintf( (TCHAR*)pBuffer,
							"--%s\r\n"
							"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
							"Content-Type: %s\r\n"
							"\r\n",
							szBoundary,
							itArgv->szName, itArgv->szValue,
							GetContentType(itArgv->szValue)
							);

			DWORD	dwBufPosition=_tcslen((TCHAR*)pBuffer);	
	
			while(::ReadFile(hFile, pBytes, __SIZE_BUFFER, &dwNumOfBytesToRead, NULL) && 
				dwNumOfBytesToRead>0 && 
				dwTotalBytes<=dwSize)
			{
				::CopyMemory((pBuffer+dwBufPosition+dwTotalBytes), pBytes, dwNumOfBytesToRead);
				::ZeroMemory(pBytes, __SIZE_BUFFER+1);
				dwTotalBytes+=dwNumOfBytesToRead;				
			}

			printf_log("file chunk %d, for sent %d. dwTotalBytes %d (%s) %X", actual_size, dwSize, dwTotalBytes, GetContentType(itArgv->szValue), GetLastError() );					


			::CopyMemory((pBuffer+dwBufPosition+dwTotalBytes), "\r\n", _tcslen("\r\n"));
			::CloseHandle(hFile);

			dwBufferSize=dwBufPosition+dwTotalBytes+_tcslen("\r\n");

			break;			
		}

		::CopyMemory((pInBuffer+dwPosition), pBuffer, dwBufferSize);
		dwPosition+=dwBufferSize;

		if(pBuffer){
			::LocalFree(pBuffer);
			pBuffer=NULL;
		}
	}

	::CopyMemory((pInBuffer+dwPosition), "--", 2);
	::CopyMemory((pInBuffer+dwPosition+2), szBoundary, _tcslen(szBoundary));
	::CopyMemory((pInBuffer+dwPosition+2+_tcslen(szBoundary)), "--\r\n", 3);

	return dwPosition+5+_tcslen(szBoundary);
}

DWORD GenericHTTPClient::GetMultiPartsFormDataLength(){
	std::vector<GenericHTTPArgument>::iterator itArgv;

	DWORD	dwLength=0;

	for(itArgv=_vArguments.begin(); itArgv<_vArguments.end(); ++itArgv){

		switch(itArgv->dwType){
		case	GenericHTTPClient::TypeNormal:
			dwLength+=__SIZE_HTTP_HEAD_LINE*4;
			break;
		case	GenericHTTPClient::TypeBinary:
			//printf_log("dlen1 %s", itArgv->szValue);
			HANDLE hFile=::CreateFile(itArgv->szValue, 
									GENERIC_READ, // desired access
									FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
									NULL, // security attribute
									OPEN_EXISTING, // create disposition
									FILE_ATTRIBUTE_NORMAL, // flag and attributes
									NULL); // template file handle

			if (hFile != INVALID_HANDLE_VALUE)
				dwLength+=__SIZE_HTTP_HEAD_LINE*3+::GetFileSize(hFile, NULL);
			else 
				dwLength+=__SIZE_HTTP_HEAD_LINE*3+ (500*1024); // 500 kb file
			printf_log("file len %d", GetFileSize(hFile, NULL));
			::CloseHandle(hFile);			
			break;
		}

	}

	return dwLength;
}

LPCTSTR GenericHTTPClient::GetContentType(LPCTSTR szName){

	static TCHAR	szReturn[1024]="";
	
	if (strstr(szName, ".htm") || strstr(szName, ".HTM"))
		_tcscpy(szReturn, "text/html");	

	if (strlen(szReturn) == 0)
		_tcscpy(szReturn, "application/octet-stream");

	return szReturn;
}

DWORD GenericHTTPClient::GetLastError(){

	return _dwError;
}

VOID GenericHTTPClient::ParseURL(LPCTSTR szURL, LPTSTR szProtocol, LPTSTR szAddress, DWORD &dwPort, LPTSTR szURI){

	TCHAR szPort[__SIZE_BUFFER]="";
	DWORD dwPosition=0;
	BOOL bFlag=FALSE;

	while(_tcslen(szURL)>0 && dwPosition<_tcslen(szURL) && _tcsncmp((szURL+dwPosition), ":", 1))
		++dwPosition;

	if(!_tcsncmp((szURL+dwPosition+1), "/", 1)){	// is PROTOCOL
		if(szProtocol){
			_tcsncpy(szProtocol, szURL, dwPosition);
			szProtocol[dwPosition]=0;
		}
		bFlag=TRUE;
	}else{	// is HOST 
		if(szProtocol){
			_tcsncpy(szProtocol, "http", 4);
			szProtocol[5]=0;
		}
	}

	DWORD dwStartPosition=0;
	
	if(bFlag){
		dwStartPosition=dwPosition+=3;				
	}else{
		dwStartPosition=dwPosition=0;
	}

	bFlag=FALSE;
	while(_tcslen(szURL)>0 && dwPosition<_tcslen(szURL) && _tcsncmp((szURL+dwPosition), "/", 1))
			++dwPosition;

	DWORD dwFind=dwStartPosition;

	for(;dwFind<=dwPosition;dwFind++){
		if(!_tcsncmp((szURL+dwFind), ":", 1)){ // find PORT
			bFlag=TRUE;
			break;
		}
	}

	if(bFlag){
		TCHAR sztmp[__SIZE_BUFFER]="";
		_tcsncpy(sztmp, (szURL+dwFind+1), dwPosition-dwFind);
		dwPort=_ttol(sztmp);
		_tcsncpy(szAddress, (szURL+dwStartPosition), dwFind-dwStartPosition);
	}else if(!_tcscmp(szProtocol,"https")){
		dwPort=INTERNET_DEFAULT_HTTPS_PORT;
		_tcsncpy(szAddress, (szURL+dwStartPosition), dwPosition-dwStartPosition);
	}else {
		dwPort=INTERNET_DEFAULT_HTTP_PORT;
		_tcsncpy(szAddress, (szURL+dwStartPosition), dwPosition-dwStartPosition);
	}

	if(dwPosition<_tcslen(szURL)){ // find URI
		_tcsncpy(szURI, (szURL+dwPosition), _tcslen(szURL)-dwPosition);
	}else{
		szURI[0]=0;
	}

	return;
}

LPCTSTR GenericHTTPClient::QueryHTTPResponseHeader(){
	return _szHTTPResponseHeader;
}

LPCTSTR GenericHTTPClient::QueryHTTPResponse(){
	return _szHTTPResponseHTML;
}


