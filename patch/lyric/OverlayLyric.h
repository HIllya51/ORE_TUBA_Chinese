#pragma once
#include "Wnd.h"
#define GDIPVER 0x0110    //����߰汾��GDI+(1.1)
#include <gdiplus.h>
#include <gdipluseffects.h>
#include<utility>
#include"lyric.h"
using namespace Gdiplus;

typedef struct tagFontInfo {
	WCHAR FontName[64];
	REAL FontSize;
	DWORD ForeColor1;
	DWORD ForeColor2;
	DWORD LineColor;
	DWORD OkColor1;
	DWORD OkColor2;
}FontInfo, *PFontInfo;

enum class DefaultColorStyle{
	Default = 0,
	Blue = 1,
	Green = 2,
	Red = 3,
	White = 4,
	Yellow = 5
};

class OverlayLyric : private Wnd
{
public:
	OverlayLyric(int type,HINSTANCE hInst = NULL,HWND parent=0,HLRC connecthlrc=0);
	~OverlayLyric();
protected:
	BOOL InitializeGdiplus();
	void ReleaseGdiplus();
private:
	LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) override;
	BOOL DrawBackground(::Graphics* const graphics, const SIZE& size);
	BOOL DrawText(::Graphics* const graphics, const SIZE& size);
public:
	std::pair<RectF, Bitmap*> drawaline(::Graphics* const graphics, const SIZE& size,const wchar_t* text,int idx);
	void SetText(LPCWCHAR text);
	void SetScale(const REAL& scale);
	void SetFont(LPCWCHAR fontName, const REAL& fontSize);
	void SetDefaultStyle(DefaultColorStyle colorStyle);
	void SetWndPos(INT x, INT y, INT width, INT height);
	BOOL Update();
	void SetLock(BOOL lock);
	void ShowWnd();
	void HideWnd();
private:
	HLRC mconnecthlrc=0;int m_type;
	POINT mMouseXY = { 0 };
	BOOL mIsMousePrees = FALSE;
	ULONG_PTR mGdiplusToken = NULL;  // GDI+���
	FontInfo mFontInfo = { 0 };
	REAL mScale = 0.f;
	WCHAR mText[10000] = { 0 };
	BOOL mShowBackground = FALSE;
};
