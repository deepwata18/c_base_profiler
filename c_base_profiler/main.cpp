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
void DummyRandWork()
{
	int randTime = (rand() % 100 + 1) * 10;
	printf("[DEBUG] Do dummy work during %d sec\n", randTime / 1000);
	Sleep(randTime);
}

void Init()
{
	timeBeginPeriod(1);
	srand(GetTickCount64());
}

int main()
{
	Init();

	int len = rand() % 4 + 1;
	for (size_t i = 0; i < len; i++)
	{
		{
			Profile test1(L"test1");

			{
				Profile test1_1(L"test1_1");

				DummyWork(200);
			}

			{
				Profile test1_2(L"test1_2");

				DummyWork(500);
			}

			DummyRandWork();
		}

		{
			Profile test1(L"test2");

			DummyRandWork();
		}
	}

	SaveProfileData("test.txt");
}