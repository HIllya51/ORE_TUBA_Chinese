#define EXPORT_API  //__declspec(dllexport)
#include<Windows.h>
typedef void* HLRC;
//
extern "C" {
	EXPORT_API HLRC CreateLyric(int ,HWND parent, HLRC connecthlrc);
	EXPORT_API void SetLyricTextA(HLRC lrc, const char* text);
	EXPORT_API void SetLyricTextW(HLRC lrc, const wchar_t* text);
	EXPORT_API void SetLyricScale(HLRC lrc, float scale);
	EXPORT_API void SetLyricFontA(HLRC lrc, const char* fontName, float fontSize);
	EXPORT_API void SetLyricFontW(HLRC lrc, const wchar_t* fontName, float fontSize);
	EXPORT_API void LyricShow(HLRC lrc);
	EXPORT_API void LyricHide(HLRC lrc);
	EXPORT_API bool LyricUpdate(HLRC lrc);
	EXPORT_API void FreeLyric(HLRC lrc);
	EXPORT_API void SetLyricColorStyle(HLRC lrc, int style);
	EXPORT_API void SetLyricPosition(HLRC lrc, int x, int y, int width, int height);
	EXPORT_API void SetLyricLock(HLRC lrc, bool status);
}