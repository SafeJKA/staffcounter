#pragma once
class report
{
public:
	report(void);
	~report(void);

	int send_report(LPCTSTR strCommand, LPCTSTR strDevid, LPCTSTR strName, LPCTSTR strFilename);

private:

	HANDLE hFile;
	TCHAR log_file_name[100];

	char *make_hex(BYTE *inps, int sz);
	DWORD getLastFilePos(LPCTSTR fileNamePath, LPCTSTR type);
	void setLastFilePos(LPCTSTR fileNamePath, LPCTSTR type, DWORD new_pos);	
	int GetInputIdleSeconds();
	void ScanAndUpdateSettings(LPCTSTR strSettingsResp);	
};

