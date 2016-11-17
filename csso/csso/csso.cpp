// csso.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <tchar.h> 
#include <tlhelp32.h>
#include <string.h>
#include <fcntl.h>
#include <conio.h>
#include <iostream>
using namespace std;

#define BUF_SIZE 10240
LPCTSTR szName = TEXT("myFile");

int main()
{
	HANDLE hProcessSnap;
	HANDLE hMapFile;
	PROCESSENTRY32 pe32;
	wchar_t* message = new wchar_t[10000000];
	wcsncpy(message, L"", 1);
	wchar_t *msg;
	///create mapping file using the new one
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		szName);
	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d).\n"),
			GetLastError());
		return 1;
	}

	//aducem in memoria virtuala
	LPTSTR pBuf = (LPTSTR)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
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

	//cer un snapshot la procese
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot failed.err = %d \n", GetLastError());
		return(-1);
	}
	//initializez dwSize cu dimensiunea structurii.
	pe32.dwSize = sizeof(PROCESSENTRY32);
	//obtin informatii despre primul proces

	if (!Process32First(hProcessSnap, &pe32))
	{
		printf("Process32First failed. err = %d \n", GetLastError());
		CloseHandle(hProcessSnap); //inchidem snapshot-ul
		return(-1);
	}
	do
	{
		msg = new wchar_t[50];
		swprintf(msg, L"<%d><%d><%s> \n", pe32.th32ProcessID, pe32.th32ParentProcessID, pe32.szExeFile);
		wcscat(message, msg);

	} while (Process32Next(hProcessSnap, &pe32)); //trec la urmatorul proces
	
												  //inched handle-ul catre snapshot
	wprintf(L"%ls", message);
	CopyMemory((PVOID)pBuf, message, BUF_SIZE * sizeof(char));
	_getch();

	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	CloseHandle(hProcessSnap);
	return 0;
}

