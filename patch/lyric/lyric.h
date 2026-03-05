
#include<Windows.h>
typedef void* HLRC;
//
 
	 HLRC CreateLyric(int ,HWND parent, HLRC connecthlrc);
	 void SetLyricTextA(HLRC lrc, const char* text);
	 void SetLyricTextW(HLRC lrc, const wchar_t* text);
	 void SetLyricScale(HLRC lrc, float scale);
	 void SetLyricFontA(HLRC lrc, const char* fontName, float fontSize);
	 void SetLyricFontW(HLRC lrc, const wchar_t* fontName, float fontSize);
	 void LyricShow(HLRC lrc);
	 void LyricHide(HLRC lrc);
	 bool LyricUpdate(HLRC lrc);
	 void FreeLyric(HLRC lrc);
	 void SetLyricColorStyle(HLRC lrc, int style);
	 void SetLyricPosition(HLRC lrc, int x, int y, int width, int height);
	 void SetLyricLock(HLRC lrc, bool status);
 