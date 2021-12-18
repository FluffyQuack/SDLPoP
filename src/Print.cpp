#include <stdio.h>
#include "Libs/slikenet/Include/WindowsIncludes.h" //Include this instead of "windows.h" as explained in Raknet FAQ
#include "Print.h"
static HANDLE mConsolehandle = 0;

void CreateConsole()
{
#if _DEBUG
	if(mConsolehandle == 0) 
	{
		AllocConsole();	
		mConsolehandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if(mConsolehandle == INVALID_HANDLE_VALUE)
		{
			printf("Warning: Failed to create console");
			mConsolehandle = 0;
		}
	}
#endif
}

void DeleteConsole()
{
	if(mConsolehandle)
		FreeConsole();
	mConsolehandle = 0;
}

void PrintToConsole(const char *text, ...)
{
	if(mConsolehandle == 0)
		return;

	int bufferSize = 256, formattedLength = -1;
	char *formattedString = 0;
	va_list args;
	for(int i = 0; i < 10; i++)
	{
		if(i > 0) //If previous attempt failed then increase buffer size
		{
			delete[]formattedString;
			if(formattedLength != -1)
				bufferSize = formattedLength + 1;
			else
				bufferSize *= 2;
		}
		formattedString = new char[bufferSize];
		va_start(args, text);
		formattedLength = vsnprintf_s(formattedString, bufferSize, _TRUNCATE, text, args);
		va_end(args);
		if(formattedLength < bufferSize && formattedLength != -1)
			break;
	}

	DWORD charsWritten = 0;
	WriteConsole(mConsolehandle, formattedString, formattedLength + 1, &charsWritten, 0);
	delete[]formattedString;
}