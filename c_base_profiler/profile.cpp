#include "profile.h"

#include <stdio.h>

#pragma region Extern Variables

LARGE_INTEGER _freq;

PROFILE_SAMPLE _samples[PF_SPS_MAX_SIZE];
int _curSampleSize = 0;

int _indexStack[PF_SP_MAX_INDEX_STACK_SIZE];
int _curTop = -1;

#pragma endregion

int FindSampleIndex(const WCHAR* tag);

int IsIndexEmpty();
int IsIndexFull();

void ProfileBegin(const WCHAR* tag)
{
	wprintf(L"[PROFILE] Begin profiling %ls\n", tag);

	PROFILE_SAMPLE* sam;

	int index = FindSampleIndex(tag);
	// 존재하지 않는 tag일 경우 새롭게 sample을 생성
	if (index == -1)
	{
		index = _curSampleSize++;

		if (lstrcpyn(_samples[index].name, tag, PF_SP_MAX_NAME_SIZE) == NULL)
		{
			DebugBreak(); // Tag 복사 시 문제 발생
		}
	}

	if (IsIndexFull())
	{
		DebugBreak(); // _indexStack overflow
	}

	_indexStack[++_curTop] = index;

	// sample에 정보를 등록 및 갱신
	sam = &_samples[index];

	sam->callCount++;

	static int freqInit = 0;
	// QueryPerformanceFrequency는 처음 시작 시 한번만 호출되어야 함
	if (freqInit == 0)
	{
		freqInit = 1;
		QueryPerformanceFrequency(&_freq);
	}

	QueryPerformanceCounter(&sam->startTime);
}

void ProfileEnd()
{
	if (IsIndexEmpty())
	{
		DebugBreak(); // _indexStack already empty
	}

	int index = _curTop--;

	PROFILE_SAMPLE* sam = &_samples[index];

	wprintf(L"[PROFILE] End profiling %ls\n", sam->name);

	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	LONGLONG sec = (endTime.QuadPart - sam->startTime.QuadPart) / _freq.QuadPart;
}

int FindSampleIndex(const WCHAR* tag)
{
	for (size_t i = 0; i < _curSampleSize; i++)
	{
		WCHAR* name = _samples[i].name;
		if (lstrcmp(tag, name) == 0)
		{
			return i;
		}
	}

	return -1;
}

int IsIndexEmpty()
{
	if (_curTop < 0)
		return 1;
	else
		return 0;
}

int IsIndexFull()
{
	if (_curTop >= PF_SP_MAX_INDEX_STACK_SIZE - 1)
		return 1;
	else
		return 0;
}

void Profile::Begin(const WCHAR* tag)
{
	ProfileBegin(tag);
}

void Profile::End()
{
	ProfileEnd();
}
