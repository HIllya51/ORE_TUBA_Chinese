 
#define _CRT_SECURE_NO_WARNINGS
 
#include<fstream>
#include<Windows.h> 
#include <unordered_map>
#include<map>
#include<thread>
#include<detours.h>  
#include<mutex>
#include<regex>
#include <filesystem> 
#include<shellapi.h>
#include"json.hpp"
#include "lyric/lyric.h"
#ifndef ISCHT
constexpr bool g_ischs=true;
#else 
constexpr bool g_ischs=false;
#endif
template<class T>
constexpr const T selectstring(const T s1,const T s2){
    if(g_ischs)return s1;
    return s2;
}
HMODULE g_hm;
std::string LoadResImage(LPCWSTR pszResID) 
{ 
	HMODULE hModule=g_hm;
	HRSRC hRsrc = ::FindResource (hModule, pszResID,L"TXT"); // type   
	if (!hRsrc)  
		return 0;  
	// load resource into memory   
	DWORD len = SizeofResource(hModule, hRsrc);  
	BYTE* lpRsrc = (BYTE*)LoadResource(hModule, hRsrc);  
	if (!lpRsrc)  
		return 0;  
	// Allocate global memory on which to create stream   
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);  
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);  
	memcpy(pmem,lpRsrc,len);  
	 
	GlobalUnlock(m_hMem);  
	 
	FreeResource(lpRsrc); 
	return std::string((char*)pmem,len);
}  
namespace hooks{
    std::wstring StringToWideString(const std::string& text, UINT encoding)
    {
        std::vector<wchar_t> buffer(text.size() + 1);
        int length = MultiByteToWideChar(encoding, 0, text.c_str(), text.size() + 1, buffer.data(), buffer.size());
        return std::wstring(buffer.data(), length - 1);
      
    }
    HWND gamehwnd=0;
    HLRC showtipswindow;
    HLRC button;

    char savelast[4096] = { 0 };
    HWND galwindow = 0;
    auto CreateFile_save = CreateFileA;
    auto CreateFontA_SAVE = CreateFontA;
    HFONT(WINAPI  CreateFontA_HOOK)(_In_ int cHeight, _In_ int cWidth, _In_ int cEscapement, _In_ int cOrientation, _In_ int cWeight, _In_ DWORD bItalic,
        _In_ DWORD bUnderline, _In_ DWORD bStrikeOut, _In_ DWORD iCharSet, _In_ DWORD iOutPrecision, _In_ DWORD iClipPrecision,
        _In_ DWORD iQuality, _In_ DWORD iPitchAndFamily, _In_opt_ LPCSTR pszFaceName) {
        iCharSet = GB2312_CHARSET;
        return CreateFontA_SAVE(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);// "微软雅黑");

    }
    
    auto GetGlyphOutlineAs =  GetGlyphOutlineA;
    DWORD WINAPI GetGlyphOutlineAh(__in HDC hdc,
        __in UINT uChar,
        __in UINT fuFormat,
        __out LPGLYPHMETRICS lpgm,
        __in DWORD cjBuffer,
        __out_bcount_opt(cjBuffer) LPVOID pvBuffer,
        __in CONST MAT2* lpmat2
    ) {
        
        std::map<UINT, UINT>sjisgbk = {
            {0x8148,0xa3bf},//?
            {0x89b4,0xb0b3},//俺
            {0x82bd,0xa4bf},//た
            {0x82bf,0xa4c1},//ち
            {0x82c9,0xa4cb},//に
            {0x9783,0xd2ed},//翼
            {0x82cd,0xa4cf},//は
            {0x82c8,0xa4ca},//な
            {0x82a2,0xa4a4},//い
        };
        if (sjisgbk.find(uChar) != sjisgbk.end()) {
            uChar = sjisgbk[uChar];
        }
        return GetGlyphOutlineAs(hdc, uChar, fuFormat,lpgm, cjBuffer, pvBuffer, lpmat2);
    }
    HANDLE  WINAPI   CreateFileAh(
        __in     LPCSTR lpFileName,
        __in     DWORD dwDesiredAccess,
        __in     DWORD dwShareMode,
        __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        __in     DWORD dwCreationDisposition,
        __in     DWORD dwFlagsAndAttributes,
        __in_opt HANDLE hTemplateFile
    ) {
        std::filesystem::path path(lpFileName);
        auto parent = path.parent_path();
        
        if (parent.filename() == "SCRIPT") { 
             
            auto new_parent = parent.parent_path() / selectstring("SCRIPT_CHS", "SCRIPT_CHT");
            auto p = new_parent.string() + '\\' + path.filename().string();
              
            return CreateFile_save(p.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
        } 

        return CreateFile_save(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    auto CreateFontIndirectA_SAVE = CreateFontIndirectA;
    HFONT(WINAPI  CreateFontIndirectA_HOOK) (LOGFONTA* lplf) {
        lplf->lfCharSet = GB2312_CHARSET;
       // *(char*)(lplf->lfFaceName) = *(char*)"微软雅黑";
        return CreateFontIndirectA_SAVE(lplf );

    }
    auto CreateWindowExA_save = CreateWindowExA;

    HLRC g_lrc;
    void createtipswindow(HWND gamehwnd) {
        
        std::thread([=] {
                    g_lrc = CreateLyric(3,gamehwnd,0);
                    //SetLyricParent(g_lrc, g_hwnd);
                    LyricShow(g_lrc);
                    RECT rect;
                    GetClientRect(gamehwnd,&rect);
                    SetLyricPosition(g_lrc,0, 0, rect.right, rect.bottom);
                    LyricUpdate(g_lrc);
                    MSG msg;
                    while (GetMessage(&msg, nullptr, 0, 0))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    }).detach();
        
        showtipswindow = CreateLyric(0,gamehwnd,0);
        button = CreateLyric(1,gamehwnd, showtipswindow); 

        SetLyricFontA(showtipswindow, "黑体", 18);
         
        RECT rect;
        GetWindowRect(gamehwnd, &rect);

        SetLyricPosition(showtipswindow, 0, 0, rect.right-rect.left-10, 400);
         
        SetLyricPosition(button, 692, 427, 104, 37);
        auto t = std::thread([] {
            MSG msg;
            while (GetMessage(&msg, nullptr, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            });

        t.detach();
    }
    HWND  WINAPI  CreateWindowExAhook(
        _In_ DWORD dwExStyle,
        _In_opt_ LPCSTR lpClassName,
        _In_opt_ LPCSTR lpWindowName,
        _In_ DWORD dwStyle,
        _In_ int X,
        _In_ int Y,
        _In_ int nWidth, 
        _In_ int nHeight,
        _In_opt_ HWND hWndParent,
        _In_opt_ HMENU hMenu,
        _In_opt_ HINSTANCE hInstance,
        _In_opt_ LPVOID lpParam) {
        HWND hwnd = CreateWindowExA_save(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
        if (gamehwnd == 0) {

            createtipswindow(hwnd);
            gamehwnd = hwnd;
            SetWindowTextW(gamehwnd,(StringToWideString(lpWindowName,CP_ACP)+L" 三号布丁ver3.0  The Strongest Hyper Master-Homo × 新米").c_str());
        }
        return hwnd;

    }
    
 
    

    std::string WideStringToString(const std::wstring& text, UINT encoding=CP_UTF8)
    {
        std::vector<char> buffer((text.size() + 1) * 4);
        WideCharToMultiByte(encoding, 0, text.c_str(), -1, buffer.data(), buffer.size(), nullptr, nullptr);
        return buffer.data();
    }
    auto sub_455ab0=0x455ab0;
    nlohmann::json tips;
    void __stdcall sub_455ab0H_1(char* text)
    {  
        if (strcmp(savelast, text) == 0) return ;
        strcpy(savelast, text);

        std::string t = text;
        t = WideStringToString(StringToWideString(t, 936));
        
        if (tips.find(t)!= tips.end()) {
            std::string tipstext = tips.at(t);


            SetLyricTextW(showtipswindow, StringToWideString(tipstext, 65001).c_str());
            //LyricUpdate(showtipswindow);
            LyricShow(button);
            LyricUpdate(button);
        }
        else {

            SetLyricTextW(showtipswindow,L"");
            LyricUpdate(showtipswindow);
            LyricHide(button);
            LyricHide(showtipswindow);
        }
         
    }
     __declspec(naked) void sub_455ab0H(char *text)
    {
        __asm{
            
            pushad
            pushfd
            mov eax,[esp+0x4]
            push eax 
            call sub_455ab0H_1
            popfd
            popad
            jmp sub_455ab0
        }
    }
    HRESULT __cdecl sub_477390H(HWND a1, const CHAR* a2, const CHAR* lpString2, LPSTR pszPath) {
        char _s[10000] = { 0 };
        GetCurrentDirectoryA(10000, _s);

        strcpy(pszPath, _s);
        return 0;
    }
    typedef  HRESULT  (__cdecl *sub_477390_t)(HWND a1, const CHAR* a2, const CHAR* lpString2, LPSTR pszPath) ;
    auto sub_477390 = 0x477390;

    auto EnumFontFamiliesExAs = EnumFontFamiliesExA;
    int  WINAPI EnumFontFamiliesExAh(_In_ HDC hdc, _In_ LPLOGFONTA lpLogfont, _In_ FONTENUMPROCA lpProc, _In_ LPARAM lParam, _In_ DWORD dwFlags) {
        lpLogfont->lfCharSet = GB2312_CHARSET;
        return EnumFontFamiliesExAs(hdc, lpLogfont, lpProc, lParam, dwFlags);
    }
    auto DialogBoxParamAS = DialogBoxParamA;
    INT_PTR  WINAPI DialogBoxParamAH(_In_opt_ HINSTANCE hInstance,
    _In_ LPCSTR lpTemplateName,
    _In_opt_ HWND hWndParent,
    _In_opt_ DLGPROC lpDialogFunc,
    _In_ LPARAM dwInitParam) {
        return DialogBoxParamAS(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
    }
    auto showwindow=ShowWindow;
    bool showindowonce=true;
    BOOL
    WINAPI
    ShowWindow(
        _In_ HWND hWnd,
        _In_ int nCmdShow){
            auto res=showwindow(hWnd,nCmdShow);
            if(hWnd==gamehwnd){
                if(showindowonce)
                {
                    showindowonce=false;
                    auto event=CreateEventW(nullptr,0,0, L"LIANYUYUEKUANGBING_SHOW_THANKS");
                        MSG msg;
                    while(GetMessage(&msg, nullptr, 0, 0) && WaitForSingleObject(event,5)!=WAIT_OBJECT_0){
                     
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                     
                    }
                    CloseHandle(event);
                    LyricHide(g_lrc);
                }
            }
            return res;
        }
    void hookall(HINSTANCE hInstance) {
        
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)EnumFontFamiliesExAs, EnumFontFamiliesExAh);
        DetourAttach(&(PVOID&)CreateFontA_SAVE, CreateFontA_HOOK);
        DetourAttach(&(PVOID&)CreateFontIndirectA_SAVE, CreateFontIndirectA_HOOK);
        DetourAttach(&(PVOID&)CreateWindowExA_save, CreateWindowExAhook);
        DetourAttach(&(PVOID&)CreateFile_save, CreateFileAh);
        DetourAttach(&(PVOID&)GetGlyphOutlineAs, GetGlyphOutlineAh);
        DetourAttach(&(PVOID&)sub_477390, sub_477390H);
        DetourAttach(&(PVOID&)sub_455ab0, sub_455ab0H);
        DetourAttach(&(PVOID&)showwindow, ShowWindow);
        DetourAttach(&(PVOID&)DialogBoxParamAS, DialogBoxParamAH);
        DetourTransactionCommit(); 
    }
    
}
 
std::string WideStringToString(const std::wstring& text, UINT encoding=CP_UTF8)
    {
        std::vector<char> buffer((text.size() + 1) * 4);
        WideCharToMultiByte(encoding, 0, text.c_str(), -1, buffer.data(), buffer.size(), nullptr, nullptr);
        return buffer.data();
    }
__declspec(dllexport) void dumy() {} 
void patchsjisstrings() {
    
    std::vector<std::pair<UINT, std::string >>pairs = {
        {0x4FC1D8,selectstring("要退出「%s」吗？","要退出「%s」嗎？")},
        {0x4FB908,selectstring("-画面设置-%s","-畫面設置-%s")},
        {0x4FB364,selectstring("恢复默认设置","恢復默認設置")},
        {0x4FB348,selectstring("输入·其他设置","輸入·其他設置")},
        {0x4FDFCC,selectstring("全屏模式","全屏模式")}, //不知道为什么，不管用
        //combo
        {0x4FB898,selectstring("快进直到下一个选择支","快進直到下一個選擇支")},
        {0x4FB890,selectstring("存档","存檔")},
        {0x4FB888,selectstring("读档","讀檔")},
        {0x4FB87C,selectstring("按钮%d","按鈕%d")},
        {0x4FB8B0,selectstring("自动播放","自動播放")},
        {0x4FB8C0,selectstring("关闭窗口","關閉窗口")},
        {0x4FB8D4,selectstring("系统菜单","系統菜單")},
        {0x4FB8E8,selectstring("未分配","未分配")},
 
        {0x4FC2A4,selectstring("%s【存档时间 %s】","%s【存檔時間 %s】")},
        {0x4FC294,selectstring("】","】")},
        {0x4FC298,selectstring("【","【")},
        {0x4FC2DC,selectstring("『","『")},
        {0x4FC2B8,selectstring("』","』")},
        {0x4FC1f4,selectstring("？？？","？？？")},


    };
    
         DWORD old,_;
    for (auto pair : pairs) { 
        VirtualProtect((void*)pair.first, 100, PAGE_EXECUTE_READWRITE, &old);
        strcpy((char*)pair.first, pair.second.c_str());
    }
        VirtualProtect((void*)0x4EDE80, 100, PAGE_EXECUTE_READWRITE, &old);
    memcpy((void*)0x4EDE80, "日\0一\0二\0三\0四\0五\0六\0", 7 * 3);
    
    
}

std::vector<std::pair<int, int>> patch81s={
{0x411019,1},
{0x411021,1},
{0x41df4c,1},
{0x41df54,1},
{0x41e992,1},
{0x41e99a,1},
{0x41eb7d,2},
{0x41eb87,2},
{0x432d9e,1},
{0x432da6,1},
{0x432dcb,1},
{0x432dd7,1},
{0x43307b,2},
{0x433085,2},
{0x4330c2,2},
{0x4330cc,2},
{0x43ddfb,1},
{0x43de03,1},
{0x43e73e,1},
{0x43e746,1},
{0x44ae38,2},
{0x44ae42,2},
{0x44c5ee,1},
{0x44c5f6,1},
{0x44c809,2},
{0x44c813,2},
{0x44d71d,1},
{0x44d725,1},
{0x44e1cc,1},
{0x44e1d4,1},
{0x44e8e3,1},
{0x44e8eb,1},
{0x450e49,2},
{0x450e53,2},
{0x45215a,2},
{0x452164,2},
{0x455b31,1},
{0x455b39,1},
{0x455be3,1},
{0x455beb,1},
{0x455ccb,1},
{0x455cd3,1},
{0x455d8f,1},
{0x455d97,1},
{0x456012,1},
{0x45601a,1},
{0x456092,1},
{0x45609a,1},
{0x456256,1},
{0x45625e,1},
{0x456288,1},
{0x456290,1},
{0x456636,1},
{0x45663e,1},
{0x456770,2},
{0x45677a,2},
{0x456869,1},
{0x456871,1},
{0x4568f9,1},
{0x456901,1},
{0x456b04,1},
{0x456b0c,1},
{0x456b48,1},
{0x456b50,1},
{0x456bb5,1},
{0x456bbd,1},
{0x45a9ac,2},
{0x45a9b6,2},
{0x474a18,1},
{0x474a20,1},
{0x474ae3,2},
{0x474aed,2},
{0x474b7b,1},
{0x474b83,1},
{0x474c45,1},
{0x474c4d,1},
{0x474d54,1},
{0x474d5c,1},
{0x474ded,1},
{0x474df5,1},
{0x474e35,1},
{0x474e3d,1},
{0x47719c,1},
{0x4771a4,1},
{0x47727b,1},
{0x477283,1},
{0x4898b1,1},
{0x4898b9,1},
{0x48a3f4,2},
{0x48a3fe,2},
{0x48a856,2},
{0x48a860,2},
{0x48a9f6,2},
{0x48aa00,2},
{0x48ac06,2},
{0x48ac10,2},
{0x48af66,2},
{0x48af70,2},
{0x48b24f,2},
{0x48b259,2},
{0x48b654,1},
{0x48b660,1},
{0x48b934,2},
{0x48b942,2},
{0x48baa7,2},
{0x48bab5,2},
{0x4960ec,1},
{0x4960f4,1},
{0x496ec8,1},
{0x496ed0,1},
{0x4979a8,1},
{0x4979b0,1},
{0x4bee2e,2},
{0x4bee38,2},
{0x4c645d,1},
{0x4c6465,1},
{0x4c654c,1},
{0x4c6554,1},
{0x4c668f,1},
{0x4c6697,1},
{0x4c66f8,1},
{0x4c6700,1},
{0x4c6aa0,1},
{0x4c6aa8,1},
{0x4c6bd1,1},
{0x4c6bd9,1},
{0x4d858e,1},
{0x4d8596,1},
{0x4d87d3,1},
{0x4d87db,1},
{0x4d8892,2},
{0x4d889c,2},
{0x4d88d4,1},
{0x4d88dc,1},
{0x4d89b6,1},
{0x4d89be,1},
{0x4d8a67,2},
{0x4d8a71,2},
{0x4d8aa4,2},
{0x4d8aae,2},
{0x4d8aca,2},
{0x4d8ad4,2},
{0x4d8b37,2},
{0x4d8b41,2},
{0x4d92f3,1},
{0x4d92fb,1},
{0x4d9394,2},
{0x4d939e,2}};

void nopmessagebox() {
    void* addr = (void*)0x412B6D;
    int sz = 5 + 2 + 6;
    DWORD old,_;
    VirtualProtect(  addr, sz, PAGE_EXECUTE_READWRITE, &old);
    
    memset(addr, 0x90, sz); 

    addr = (void*)0x49E07F;
    VirtualProtect(addr, 6, PAGE_EXECUTE_READWRITE, &old);
    memset(addr, 0x90, 6);

    addr=(void*)0x412C74;
    VirtualProtect(addr, 5, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, "\xb0\x01\x90\x90\x90",5);

}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: 
    
    for(auto &&[addr, offset]:patch81s)
    {
        DWORD old,_;
    VirtualProtect((void*)addr, 5, PAGE_EXECUTE_READWRITE, &old);
    ((BYTE*)addr)[offset]=0xfe;

    }
    g_hm=hModule;hooks::tips = nlohmann::json::parse(LoadResImage(L"TIPSJSON"));
        hooks::hookall(hModule);
        nopmessagebox();
       patchsjisstrings();
    case DLL_THREAD_ATTACH:

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

