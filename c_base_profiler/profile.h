#pragma once

#include <Windows.h>

#define PROFILE

#define PROFILE_SAMPLES_MAX_SIZE 1000
#define PROFILE_SAMPLE_MAX_NAME_SIZE 64
#define PROFILE_SAMPLE_MAX_INDEX_STACK_SIZE 30

//#ifdef PROFILE
//
//#define PRO_BEGIN(TagName)	ProfileBegin(TagName)
//#define PRO_END(TagName)	ProfileEnd(TagName)
//
//#elif
//
//#define PRO_BEGIN(TagName)
//#define PRO_END(TagName)
//
//#endif // PROFILE

struct PROFILE_SAMPLE
{
	long			flag;									// ���������� ��� ����. (�迭�ÿ���)
	WCHAR			name[PROFILE_SAMPLE_MAX_NAME_SIZE];		// �������� ���� �̸�.

	LARGE_INTEGER	startTime;		// �������� ���� ���� �ð�.
	__int64			totalTime;		// ��ü ���ð� ī���� Time.	(��½� ȣ��ȸ���� ������ ��� ����)
	__int64			minTime[2];		// �ּ� ���ð� ī���� Time.	(�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1] ���� �ּ� [2])
	__int64			maxTime[2];		// �ִ� ���ð� ī���� Time.	(�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1] ���� �ִ� [2])

	__int64			callCount;		// ���� ȣ�� Ƚ��.
};

LARGE_INTEGER freq;

PROFILE_SAMPLE samples[PROFILE_SAMPLES_MAX_SIZE];
int current_size = 0;

int _indexStack[PROFILE_SAMPLE_MAX_INDEX_STACK_SIZE];
int _curTop = -1;

class Profile
{
public:
	Profile(const WCHAR* tag)
	{
		Begin(tag);
	}
	~Profile()
	{
		End();
	}

private:
	void Begin(const WCHAR* tag);
	void End();

	int FindSampleIndex(const WCHAR* tag);

	int IsIndexEmpty();
	int IsIndexFull();
};

void Profile::Begin(const WCHAR* tag)
{
	wprintf(L"Begin profiling %ls\n", tag);
	
	PROFILE_SAMPLE* sam;

	int index = FindSampleIndex(tag);
	if (index == -1)
	{
		index = current_size++;

		if (lstrcpyn(samples[index].name, tag, PROFILE_SAMPLE_MAX_NAME_SIZE) == NULL)
		{
			DebugBreak(); // Tag ���� �� ���� �߻�
		}
	}

	if (IsIndexFull())
	{
		DebugBreak(); // _indexStack overflow
	}

	_indexStack[++_curTop] = index;

	sam = &samples[index];

	sam->callCount++;

	static int freqInit = 0;
	if (freqInit == 0)
	{
		freqInit = 1;
		QueryPerformanceFrequency(&freq);
	}

	QueryPerformanceCounter(&sam->startTime);
}

void Profile::End()
{
	if (IsIndexEmpty())
	{
		DebugBreak(); // _indexStack already empty
	}

	int index = _curTop--;

	PROFILE_SAMPLE* sam = &samples[index];

	wprintf(L"End profiling %ls\n", sam->name);

	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	LONGLONG sec = (endTime.QuadPart - sam->startTime.QuadPart) / freq.QuadPart;
}

int Profile::FindSampleIndex(const WCHAR* tag)
{
	for (size_t i = 0; i < current_size; i++)
	{
		WCHAR* name = samples[i].name;
		if (lstrcmp(tag, name) == 0)
		{
			return i;
		}
	}

	return -1;
}

int Profile::IsIndexEmpty()
{
	if (_curTop < 0)
		return 1;
	else
		return 0;
}

int Profile::IsIndexFull()
{
	if (_curTop >= PROFILE_SAMPLE_MAX_INDEX_STACK_SIZE - 1)
		return 1;
	else
		return 0;
}