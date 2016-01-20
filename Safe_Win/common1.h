#ifndef _COMMON_H_1234
#define _COMMON_H_1234

void GetMyPath(LPTSTR path, int include_name, HMODULE handle = NULL);

LPCTSTR ReadReg(HKEY key, LPCTSTR path, LPCTSTR val_name, LPTSTR ret_value, DWORD buff_len);
DWORD ReadReg(HKEY key, LPCTSTR path, LPCTSTR val_name, DWORD i);
bool WriteReg(HKEY key, LPCTSTR path, LPCTSTR val_name, LPCTSTR value) ;
bool WriteReg(HKEY key, LPCTSTR path, LPCTSTR val_name, DWORD value) ;
DWORD ReadReg(HKEY key, LPCTSTR path, LPCTSTR val_name, LPBYTE value, LPDWORD len);
void WriteReg(HKEY key, LPCTSTR path, LPCTSTR val_name, LPBYTE value, DWORD len);

LPCTSTR ReadRegAny(LPCTSTR path, LPCTSTR val_name, LPTSTR ret_value, DWORD buff_len);
DWORD ReadRegAny(LPCTSTR path, LPCTSTR val_name, DWORD i);


__int64 GetDiskSpaceMB(LPCTSTR root_path);
__int64 GetDiskFreeSpaceMB(LPCTSTR root_path);
void enableItems(HWND parent, bool en, ...);
void showItems(HWND parent, int show, ...);
HWND CreateToolTip(HWND hWnd, UINT idControl, LPCTSTR lpString, UINT iTime, bool baloon  );
BOOL SelectFolderDialog( HWND hDlg, LPTSTR Title, LPTSTR FolderPath, DWORD flags, DWORD clsid_root );
void prepare_file_name_str(LPTSTR file_name);
LPCTSTR get_windows_platform_info();

void LS_UI(HWND hwnd, int id, LPCTSTR string_name);
LPCTSTR LS(LPCSTR js_string_name);
LPCTSTR LS_ENG(LPCSTR js_string_name);
void GetSystemErrorMessage(DWORD GLE, LPTSTR szMessage);
LPCTSTR GetFileVersion_ProductName(DWORD processPID);
void printf_log(LPCTSTR szData, ...);
#endif




