#include "profile.h"

#include <stdio.h>

#pragma region Extern Variables

LARGE_INTEGER _freq;

PROFILE_SAMPLE _samples[PF_SPS_MAX_SIZE];
int _curSampleSize = 0;

int _indexStack[PF_SP_MAX_INDEX_STACK_SIZE];
int _curTop = -1;

#pragma endregion

void SetProfileTime(PROFILE_SAMPLE* sam, ULONGLONG ns);

void ProfileReset();
void ProfileReset(PROFILE_SAMPLE* sam);

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

	if (sam->flag <= 0)
	{
		sam->flag = 1;

		for (size_t i = 0; i < PF_SP_MIN_MAX_SAVE_SIZE; i++)
		{
			sam->minTime[i] = 0;
			sam->maxTime[i] = 0;
		}
	}

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

	int index = _indexStack[_curTop--];

	PROFILE_SAMPLE* sam = &_samples[index];

	wprintf(L"[PROFILE] End profiling %ls\n", sam->name);

	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	ULONGLONG mcs = ((double)(endTime.QuadPart - sam->startTime.QuadPart) / _freq.QuadPart) * 10000000;
	SetProfileTime(sam, mcs);
}

void SetProfileTime(PROFILE_SAMPLE* sam, ULONGLONG mcs)
{
	ULONGLONG tempTime;
	ULONGLONG prevTime = 0;

	if (mcs < sam->minTime[PF_SP_MIN_MAX_SAVE_SIZE - 1] || sam->minTime[PF_SP_MIN_MAX_SAVE_SIZE - 1] == 0)
	{
		tempTime = mcs;

		// 최소값과 비교
		for (size_t i = 0; i < PF_SP_MIN_MAX_SAVE_SIZE; i++)
		{
			if (tempTime <= sam->minTime[i] || sam->minTime[i] == 0)
			{
				prevTime = sam->minTime[i];
				sam->minTime[i] = tempTime;
				tempTime = prevTime;
			}
		}
	}

	if (mcs > sam->maxTime[PF_SP_MIN_MAX_SAVE_SIZE - 1])
	{
		tempTime = mcs;

		// 최대값과 비교
		for (size_t i = 0; i < PF_SP_MIN_MAX_SAVE_SIZE; i++)
		{
			if (tempTime >= sam->maxTime[i])
			{
				prevTime = sam->maxTime[i];
				sam->maxTime[i] = tempTime;
				tempTime = prevTime;
			}
		}
	}

	sam->totalTime += mcs;
}

void ProfileReset()
{
	for (size_t i = 0; i < PF_SPS_MAX_SIZE; i++)
	{
		if (_samples[i].flag <= 0)
		{
			break;
		}

		ProfileReset(&_samples[i]);
	}
}

void ProfileReset(PROFILE_SAMPLE* sam)
{
	sam->startTime.QuadPart = 0;

	sam->totalTime = 0;
	for (size_t i = 0; i < PF_SP_MIN_MAX_SAVE_SIZE; i++)
	{
		sam->minTime[i] = 0;
	}
	for (size_t i = 0; i < PF_SP_MIN_MAX_SAVE_SIZE; i++)
	{
		sam->maxTime[i] = 0;
	}

	sam->callCount = 0;
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

void SaveProfileData(const char* fileName)
{
	// 프로파일링 한 범위 개수
	size_t count;
	for (count = 0; count < PF_SPS_MAX_SIZE; count++)
	{
		if (_samples[count].flag <= 0)
		{
			break;
		}
	}

	FILE* fs;
	errno_t err;
	err = fopen_s(&fs, fileName, "w");
	if (err != 0)
	{
		DebugBreak();
	}

	WCHAR** buf = (WCHAR**)malloc(count);
	size_t lineSize = 100 + PF_SP_MAX_NAME_SIZE;

	WCHAR* label = (WCHAR*)L"Name | Average | Min | Max | Call\n";
	fwrite(L"Name | Average | Min | Max | Call\n", sizeof(WCHAR), lstrlen(label), fs);

	for (size_t iProfileCount = 0; iProfileCount < count; iProfileCount++)
	{
		buf[iProfileCount] = (WCHAR*)malloc(lineSize);

		// 평균 값 계산
		ULONGLONG avg = _samples[iProfileCount].totalTime;
		if (_samples[iProfileCount].callCount <= PF_SP_MIN_MAX_SAVE_SIZE * 2)
		{
			avg /= _samples[iProfileCount].callCount;
		}
		else
		{
			for (size_t iSaveSize = 0; iSaveSize < PF_SP_MIN_MAX_SAVE_SIZE; iSaveSize++)
			{
				avg -= _samples[iProfileCount].minTime[iSaveSize];
				avg -= _samples[iProfileCount].maxTime[iSaveSize];
			}

			avg /= _samples[iProfileCount].callCount - PF_SP_MIN_MAX_SAVE_SIZE * 2;
		}

		// Formating
		wsprintf(buf[iProfileCount], L"%s | %I64d micro sec | %I64d micro sec | %I64dmicro sec | %I64d micro sec\n", 
			_samples[iProfileCount].name, 
			avg, _samples[iProfileCount].minTime[0], _samples[iProfileCount].maxTime[0], 
			_samples[iProfileCount].callCount);

		int len = lstrlen(buf[iProfileCount]);
		size_t writeLen = fwrite(buf[iProfileCount], sizeof(WCHAR), len, fs);
		if (writeLen < len)
		{
			DebugBreak();
		}
	}

	fclose(fs);

	// 메모리 해제
	for (size_t i = 0; i < count; i++)
	{
		free(buf[i]);
	}
	free(buf);
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
