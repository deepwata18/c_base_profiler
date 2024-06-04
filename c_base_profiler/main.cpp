#pragma comment(lib, "Winmm.lib")

#define PROFILE

#include <Windows.h>

#include <stdio.h>

#include "profile.h"

void DummyWork(DWORD ms)
{
	printf("[DEBUG] Do dummy work during %d sec\n", ms / 1000);
	Sleep(ms);
}

int main(void)
{
	timeBeginPeriod(1);

	Profile first(L"First");
	
	{
		Profile second(L"Second");
		DummyWork(2000);
	}

	{
		Profile second(L"Second");
		DummyWork(2000);
	}

	DummyWork(1000);
}