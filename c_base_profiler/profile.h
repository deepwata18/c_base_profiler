#pragma once

#include <Windows.h>

#define PF_SPS_MAX_SIZE 1000
#define PF_SP_MAX_NAME_SIZE 64
#define PF_SP_MAX_INDEX_STACK_SIZE 30
#define PF_SP_MIN_MAX_SAVE_SIZE 2

#pragma region Types

struct PROFILE_SAMPLE
{
	long			flag;								// 프로파일의 사용 여부. (배열시에만)
	WCHAR			name[PF_SP_MAX_NAME_SIZE];			// 프로파일 샘플 이름.

	LARGE_INTEGER	startTime;							// 프로파일 샘플 실행 시간.

	ULONGLONG		totalTime;							// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
	ULONGLONG		minTime[PF_SP_MIN_MAX_SAVE_SIZE];	// 최소 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최소 [1] 다음 최소 [2])
	ULONGLONG		maxTime[PF_SP_MIN_MAX_SAVE_SIZE];	// 최대 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최대 [1] 다음 최대 [2])

	ULONGLONG		callCount;							// 누적 호출 횟수.
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

void SaveProfileData(const char* fileName);

#pragma region Extern Variables

extern LARGE_INTEGER _freq;

extern PROFILE_SAMPLE _samples[PF_SPS_MAX_SIZE];
extern int _curSampleSize;

extern int _indexStack[PF_SP_MAX_INDEX_STACK_SIZE];
extern int _curTop;

#pragma endregion
