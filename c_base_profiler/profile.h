#pragma once

#include <Windows.h>

#define PF_SPS_MAX_SIZE 1000
#define PF_SP_MAX_NAME_SIZE 64
#define PF_SP_MAX_INDEX_STACK_SIZE 30

#pragma region Types

struct PROFILE_SAMPLE
{
	long			flag;						// ���������� ��� ����. (�迭�ÿ���)
	WCHAR			name[PF_SP_MAX_NAME_SIZE];	// �������� ���� �̸�.

	LARGE_INTEGER	startTime;		// �������� ���� ���� �ð�.
	__int64			totalTime;		// ��ü ���ð� ī���� Time.	(��½� ȣ��ȸ���� ������ ��� ����)
	__int64			minTime[2];		// �ּ� ���ð� ī���� Time.	(�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1] ���� �ּ� [2])
	__int64			maxTime[2];		// �ִ� ���ð� ī���� Time.	(�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1] ���� �ִ� [2])

	__int64			callCount;		// ���� ȣ�� Ƚ��.
};

class Profile
{
public:
	Profile(const WCHAR* tag)
	{
#ifdef PROFILE
		Begin(tag);
#endif // PROFILE
	}
	~Profile()
	{
#ifdef PROFILE
		End();
#endif // PROFILE
	}

private:
	void Begin(const WCHAR* tag);
	void End();
};

#pragma endregion

#ifdef PROFILE

#define PROFILE_BEGIN(tag)	ProfileBegin(tag)
#define PROFILE_END()		ProfileEnd()

#else

#define PROFILE_BEGIN(tag)
#define PROFILE_END(tag)

#endif

void ProfileBegin(const WCHAR* tag);
void ProfileEnd();

#pragma region Extern Variables

extern LARGE_INTEGER _freq;

extern PROFILE_SAMPLE _samples[PF_SPS_MAX_SIZE];
extern int _curSampleSize;

extern int _indexStack[PF_SP_MAX_INDEX_STACK_SIZE];
extern int _curTop;

#pragma endregion
