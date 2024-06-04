#include <Windows.h>

#include <stdio.h>

#include "profile.h"

int main(void)
{
	Profile first(L"First");
	
	{
		Profile second(L"Second");

		Sleep(2000);
	}

	Sleep(1000);

	{
		Profile second(L"Second");

		Sleep(2000);
	}
}