#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <vector>
#include <iostream>
using namespace std;

#define BUF_SIZE 10240
LPCTSTR szName = TEXT("myFile");
struct ProcessInfo {
	char * pid = new char[20];
	char * ppid = new char[20];
	char * filename = new char[40];
};

BOOL SetPrivilege(
	HANDLE hToken,               // access token handle
	LPCTSTR lpszPrivilege,    // name of privilege to enable/disable
	BOOL bEnablePrivilege    // to enable (or disable privilege)
)
{
	// Token privilege structure
	TOKEN_PRIVILEGES tp;
	// Used by local system to identify the privilege
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,                // lookup privilege on local system
		lpszPrivilege,    // privilege to lookup
		&luid))               // receives LUID of privilege
	{
		printf("LookupPrivilegeValue() error: %u\n", GetLastError());
		return FALSE;
	}
	else
		printf("LookupPrivilegeValue() is OK\n");

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;

	// Don't forget to disable the privileges after you enabled them,
	// or have already completed your task. Don't mess up your system :o)
	if (bEnablePrivilege)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		printf("tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED\n");
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
		printf("tp.Privileges[0].Attributes = 0\n");
	}

	// Enable the privilege (or disable all privileges).
	if (!AdjustTokenPrivileges(
		hToken,
		FALSE, // If TRUE, function disables all privileges, if FALSE the function modifies privilege based on the tp
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		printf("AdjustTokenPrivileges() error: %u\n", GetLastError());
		return FALSE;
	}
	else
	{
		printf("AdjustTokenPrivileges() is OK, last error if any: %u\n", GetLastError());
		printf("Should be 0, means the operation completed successfully = ERROR_SUCCESS\n");
	}
	return TRUE;
}
ProcessInfo extract_pid(char * line, int contor) {
	ProcessInfo myProcess;
	char* pid = new char[20];
	char* ppid = new char[20];
	char* filename = new char[40];
	int i = 0;
	if (contor == 1) {
		while (line[i + 1] != '>') {
			pid[i] = line[i + 1];
			i++;
		}
		pid[i] = NULL;
		strcpy(line, line + i + 3);
	}
	else {
		while (line[i + 2] != '>') {
			pid[i] = line[i + 2];
			i++;
		}
		pid[i] = NULL;
		strcpy(line, line + i + 4);
	}
	i = 0;
	while (line[i] != '>') {
		ppid[i] = line[i];
		i++;
	}
	ppid[i] = NULL;
	strcpy(line, line + i + 2);

	i = 0;
	while (line[i] != '>') {
		filename[i] = line[i];
		i++;
	}
	filename[i] = NULL;
	//printf("%s %s %s\n", pid, ppid, filename);
	strcpy(myProcess.pid, pid);
	strcpy(myProcess.ppid, ppid);
	strcpy(myProcess.filename, filename);
	//printf("%s %s %s\n", myProcess.pid, myProcess.ppid, myProcess.filename);
	return myProcess;
}

vector<ProcessInfo>split(wchar_t* arr)
{
	int contor = 0;
	wstring ws(arr);
	string str(ws.begin(), ws.end());
	string buffer = str;
	char * line = new char[50];
	vector<string>result;
	vector<ProcessInfo> infos;
	int beg = 0, end = 0;//begining and end of each line in the array
	while (end = buffer.find('\n', beg + 1))
	{
		if (end == -1)
		{
			result.push_back(buffer.substr(beg));
			break;
		}
		contor++;
		sprintf(line, "%s", buffer.substr(beg, end - beg).c_str());
		ProcessInfo myProcess;
		myProcess = extract_pid(line, contor);
		//printf("%s %s %s \n", myProcess.pid, myProcess.ppid, myProcess.filename);
		result.push_back(line);
		infos.push_back(myProcess);
		beg = end;
	}
	return infos;
}

int tip_nod(char* pid, vector<ProcessInfo> v) {
	int st = 0, dr = 0;
	for (int i = 0; i < v.size(); i++) {
		if (strcmp(pid, v[i].pid) == 0)
			st++;
		if (strcmp(pid, v[i].ppid) == 0)
			dr++;
	}
	if (st == 0 && dr == 0)
		return -1; // nu exista nod
	if ((st == 0 && dr != 0) || strcmp(pid, "0") == 0)
		return 0; // radacina sau system process
	if (st != 0 && dr == 0)
		return 1; // frunza
	if (st != 0 && dr != 0)
		return 2; //intermediar

}
bool contains(char * pid, vector<char*> v) {
	for (int i = 0; i < v.size(); i++) {
		if (strcmp(v[i], pid) == 0)
			return TRUE;
	}
	return FALSE;
}
vector<char*> gaseste_radacini(vector<ProcessInfo> v) {

	vector<char*> radacini;
	for (int i = 0; i < v.size(); i++)
	{
		if (tip_nod(v[i].ppid, v) == 0 && contains(v[i].ppid, radacini) == FALSE)
			radacini.push_back(v[i].ppid);
	}
	return radacini;
}

void parcurgere_radacina(char* ppid, vector<ProcessInfo> result, char* pad) {
	if (tip_nod(ppid, result) != 1) { //nu este frunza
		strcat(pad, "  ");
		for (int i = 0; i < result.size(); i++)
			if (strcmp(ppid, result[i].ppid) == 0) {
				//printf("%s %s %s \n", pad, result[i].filename, result[i].pid);
				printf("%s%s %s %s \n", pad, result[i].pid, result[i].ppid, result[i].filename);
				if (strcmp(result[i].pid, "0") != 0) {
					parcurgere_radacina(result[i].pid, result, pad);
				}
			}
		strcpy(pad, pad + 2);
	}
}

void killProcessByID(char *pid)
{
	DWORD mypid = atoi(pid);
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (pEntry.th32ProcessID == mypid)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				//printf("Proces terminat %s \n", pid);
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
			else
				printf("proces neterminat\n");
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

void kill_tree(char * ppid, vector<ProcessInfo> &result) {
	if (tip_nod(ppid, result) != 1) {
		for (int i = 0; i < result.size(); i++)
			if (strcmp(ppid, result[i].ppid) == 0) {
				if (strcmp(result[i].pid, "0") != 0) {
					kill_tree(result[i].pid, result);
					killProcessByID(ppid);
				}
			}
	}
	else {
		killProcessByID(ppid);
	}
	// este frunza

}

void remove_rad(int nr_arbore, vector<char*> &radacini, vector<ProcessInfo> &result) {
	nr_arbore--;
	if (nr_arbore <radacini.size()) // 
	{
		kill_tree(radacini[nr_arbore], result);
		radacini.erase(radacini.begin() + nr_arbore);
	}
}
void display(vector<ProcessInfo> &v)
{
	for (int i = 0; i < v.size(); i++)
	{
		printf("%s %s %s\n", v[i].pid, v[i].ppid, v[i].filename);
	}
	cout << "\n" << endl;
}
void display_rad(vector<char*> &v)
{
	printf("\nRadacini:\n");
	for (int i = 0; i < v.size(); i++)
	{
		printf("%s\n", v[i]);
	}
	cout << "\n" << endl;
}
void display_arbori(vector<char*> &radacini, vector<ProcessInfo> &result) {
	char* pad = new char[10000000];
	for (int i = 0; i < radacini.size(); i++) {
		strcpy(pad, "");
		printf("[arbore nr. %d]\n", i + 1);
		parcurgere_radacina(radacini[i], result, pad);

	}
	delete[] pad;
}
int main()
{
	LPCTSTR lpszPrivilege = SE_DEBUG_NAME;
	// Change this BOOL value to set/unset the SE_PRIVILEGE_ENABLED attribute
	BOOL bEnablePrivilege = TRUE;
	HANDLE hToken;

	// Open a handle to the access token for the calling process. That is this running program
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		printf("OpenProcessToken() error %u\n", GetLastError());
		return FALSE;
	}
	else
		printf("OpenProcessToken() is OK\n");

	// Call the user defined SetPrivilege() function to enable and set the needed privilege
	BOOL test = SetPrivilege(hToken, lpszPrivilege, bEnablePrivilege);
	printf("The SetPrivilege() return value: %d\n\n", test);

	//************************************************
	HANDLE hMapFile;
	LPCTSTR pBuf;

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		szName);               // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMapFile, // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0,
		0,
		BUF_SIZE);

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"),
			GetLastError());

		CloseHandle(hMapFile);

		return 1;
	}

	//wprintf(L"%ls", pBuf);
	vector<ProcessInfo >result = split((wchar_t *)pBuf);
	vector<char*> radacini = gaseste_radacini(result);

	display_rad(radacini);
	display_arbori(radacini, result);
	int nr;
	cout << "Dati nr arborelui pe care doriti sa il eliminati:\n";
	cin >> nr;
	remove_rad(nr, radacini, result);
	//display_rad(radacini);
	//display_arbori(radacini, result);

	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	//***********************************************

	// After we have completed our task, don't forget to disable the privilege
	bEnablePrivilege = FALSE;
	BOOL test1 = SetPrivilege(hToken, lpszPrivilege, bEnablePrivilege);
	printf("The SetPrivilage() return value: %d\n", test1);

	return 0;
}