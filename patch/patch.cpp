 
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
    auto CreateFontA_SAVE = CreateFontA;
    auto CreateFile_save = CreateFileA;
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
            SetWindowTextW(gamehwnd,(StringToWideString(lpWindowName,CP_ACP)+L" The Strongest Hyper MASTER-HOMO × BRANDNEW THE-RICE").c_str());
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
    void __stdcall sub_455ab0H_1(char* text)
    {  
        if (strcmp(savelast, text) == 0) return ;
        strcpy(savelast, text);

        std::string t = text;
        t = WideStringToString(StringToWideString(t, 936));
        nlohmann::json tips = nlohmann::json::parse(LoadResImage(L"TIPSJSON"));
        
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
                    SECURITY_DESCRIPTOR sd;
                    InitializeSecurityDescriptor(&(sd),1); 
                    SetSecurityDescriptorDacl(&(sd), 1, 0, 0);
                    SECURITY_ATTRIBUTES allacc; 
                    allacc.nLength=sizeof(allacc);
                    allacc.bInheritHandle=0;
                    allacc.lpSecurityDescriptor=&(sd);
                    auto event=CreateEventW(&allacc,0,0, L"LIANYUYUEKUANGBING_SHOW_THANKS");
                    while(WaitForSingleObject(event,5)!=WAIT_OBJECT_0){
                        MSG msg;
                        GetMessage(&msg, nullptr, 0, 0);
                     
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
        {0x4FC2DC,selectstring("『","『")},
        {0x4FC2B8,selectstring("』","』")},


    };
    
    for (auto pair : pairs) { 
        strcpy((char*)pair.first, pair.second.c_str());
    }
    memcpy((void*)0x4EDE80, "日\0一\0二\0三\0四\0五\0六\0", 7 * 3);
     
    std::vector<std::pair<std::pair<UINT,UINT>,   std::wstring>>wpairs = {
        {{0x5382ac,11},selectstring(L"   全屏模式   ",L"   全屏模式   ")},
        {{0x5383a4,10},selectstring(L"窗口模式",L"窗口模式")},
        {{0x5383fc,5},selectstring(L" 取 消 ",L" 取 消 ")},
        {{0x538ea6,9},selectstring(L"强制快进键设置",L"強制快進鍵設置")},
        {{0x538e6e,5},selectstring(L"键盘",L"鍵盤")},
        {{0x539596,12},selectstring(L"   鼠标·操纵杆   ",L"   滑鼠·操縱杆   ")},
        {{0x539c1e,3},selectstring(L"其他",L"其他")},
        {{0x5390e6,17},selectstring(L"键盘快捷键设置",L"鍵盤快捷鍵設置")},
        {{0x539122,2},selectstring(L"确定",L"確定")},
        {{0x53962a,2},selectstring(L"确定",L"確定")},
        {{0x539142,5},selectstring(L"   取消",L"   取消")},
        {{0x53964a,5},selectstring(L"   取消",L"   取消")},
        {{0x5391ce,7},selectstring(L"   快速存档",L"   快速存檔")},
        {{0x5391f6,7},selectstring(L"   快速读档",L"   快速讀檔")},
        {{0x5396d6,7},selectstring(L"   快速存档",L"   快速存檔")},
        {{0x5396fe,7},selectstring(L"   快速读档",L"   快速讀檔")},
        {{0x539166,6},selectstring(L"  自动播放",L"  自動播放")},
        {{0x53966e,6},selectstring(L"  自动播放",L"  自動播放")},
        {{0x53918e,3},selectstring(L" 存档",L" 存檔")},
        {{0x5391ae,3},selectstring(L" 读档",L" 讀檔")},
        {{0x539696,3},selectstring(L" 存档",L" 存檔")},
        {{0x5396b6,3},selectstring(L" 读档",L" 讀檔")},
        {{0x5392e2,8},selectstring(L"    关闭窗口",L"    關閉窗口")},
        {{0x5397ea,8},selectstring(L"    关闭窗口",L"    關閉窗口")},
        {{0x53930e,8},selectstring(L"    系统菜单",L"    系統菜單")},
        {{0x539816,8},selectstring(L"    系统菜单",L"    系統菜單")},
        {{0x53933a,4},selectstring(L"系统设置",L"系統設置")},
        {{0x539842,4},selectstring(L"系统设置",L"系統設置")},

        {{0x53935e,6},selectstring(L"文本历史记录",L"文本歷史記錄")},
        {{0x539866,6},selectstring(L"文本历史记录",L"文本歷史記錄")},
        {{0x539386,11},selectstring(L" 快进直到下一个选择支",L" 快進直到下一個選擇支")},
        {{0x53988e,11},selectstring(L" 快进直到下一个选择支",L" 快進直到下一個選擇支")},
        {{0x5393b6,8},selectstring(L"返回上一个选择支",L"返回上一個選擇支")},
        {{0x5398be,8},selectstring(L"返回上一个选择支",L"返回上一個選擇支")},
        {{0x5393e2,7},selectstring(L"   返回标题",L"   返回標題")},
        {{0x5398ea,7},selectstring(L"   返回标题",L"   返回标题")},

        {{0x5394ce,87},selectstring(LR"(※无法设置在系统端保留的键，例如 [回车]、[Esc]、[F1]～[F5]、[Shift]、[Ctrl]、[Tab]、[取消键]、[Alt] 等。)",LR"(※無法設置在系統端保留的鍵，例如 [回車]、[Esc]、[F1]～[F5]、[Shift]、[Ctrl]、[Tab]、[取消鍵]、[Alt] 等。)")},
        {{0x538f1a,13},selectstring(L"不要快进未读文本",L"不要快進未讀文本")},
        {{0x5399d6,33},selectstring(L"※按下[Delete]键可将其设置为未分配状态。",L"※按下[Delete]鍵可將其設置為未分配狀態。")},
        {{0x539aea,17},selectstring(L"右键单击以停用自动播放",L"右鍵單擊以停用自動播放")},
        {{0x5395f6,13},selectstring(L"使用操纵杆",L"使用操縱杆")},
        {{0x538f4e,11},selectstring(L"快速存档·读档",L"快速存檔·讀檔")},
        {{0x538f7e,35},selectstring(L"F1～F5           … 从槽位1～5 处快速加载",L"F1～F5           … 從槽位1～5 處快速加載")},
        {{0x538ffa,25},selectstring(L"＋F1～F5 … 快速存档至存槽位1～5 处",L"＋F1～F5 … 快速存檔至存槽位1～5 處")},
        {{0x539046,67},selectstring(LR"(※槽位6～10 会在按下键盘快捷键或快速保存（加载）按钮时顺序使用。)",LR"(※槽位6～10 會在按下鍵盤快捷鍵或快速保存（加載）按鈕時順序使用。)")},
        {{0x539a32,8},selectstring(L"鼠标按键设置",L"滑鼠按鍵設置")},
        {{0x539a5e,6},selectstring(L"＜右键＞",L"＜右鍵＞")},
        {{0x539aa2,9},selectstring(L"＜滚轮按钮＞",L"＜滾輪按鈕＞")},
        {{0x539b26,9},selectstring(L"鼠标滚轮设置",L"滑鼠滾輪設置")},
        {{0x539b52,7},selectstring(L"向上滚动↑",L"向上滾動↑")},
        {{0x539bd2,25},selectstring(L"向下滚动↓时显示下一页（继续阅读文本）",L"向下滾動↓時顯示下一頁（繼續閱讀文本）")},
        {{0x539ba6,8},selectstring(L"反向浏览文本",L"反向瀏覽文本")},
        {{0x539b7a,8},selectstring(L"显示文本历史记录",L"顯示文本歷史記錄")},
        {{0x539c52,7},selectstring(L"其他设置",L"其他設置")},
        {{0x539c7a,24},selectstring(L"存档时自动将当前文本设置为注释",L"存檔時自動將當前文本設置為注釋")},
        {{0x539d0e,23},selectstring(L"通过将鼠标光标移动到窗口下方来隐藏光标",L"通過將滑鼠光標移動到窗口下方來隱藏光標")},
        {{0x539d56,17},selectstring(L"退出时保存窗口位置",L"退出時保存窗口位置")},
        {{0x539d92,22},selectstring(L"打开网页时显示确认对话框",L"打開網頁時顯示確認對話框")},
        {{0x539df6,9},selectstring(L"播放视频",L"播放視頻")},
        {{0x539e22,20},selectstring(L"使用DirectShow绘制窗口",L"使用DirectShow繪製窗口")},
        {{0x539f2e,14},selectstring(L"显示提示弹窗",L"顯示提示彈窗")},
        {{0x539f66,22},selectstring(L"显示提示弹窗之前的等待时间",L"顯示提示彈窗之前的等待時間")},
        {{0x53a012,3},selectstring(L"毫秒",L"毫秒")},
        {{0x53a032,13},selectstring(L"文本历史记录保存页数",L"文本歷史記錄保存頁數")},
        {{0x53a0ea,30},selectstring(L"※若更改保存页数则将丢弃当前文本历史记录。",L"※若更改保存頁數則將丟棄當前文本歷史記錄。")},
        {{0x53a142,20},selectstring(L"保存并加载文本历史记录",L"保存並加載文本歷史記錄")},
        {{0x539cc6,23},selectstring(L"在文本历史记录的字体上添加抗锯齿",L"在文本歷史記錄的字體上添加抗鋸齒")},
        {{0x53a0ca,3},selectstring(L"页",L"頁")},
        {{0x538772,2},selectstring(L"是",L"是")},
        {{0x538792,3},selectstring(L"否",L"否")}
    };
    
    for (auto pair : wpairs) {
         
            std::wstring use =  pair.second;
             
            if (use.size() < pair.first.second) {
                int sz = pair.first.second - use.size();
                for (int i = 0; i < sz; i++) {
                    use += L" ";
                }
            }
            wcscpy((wchar_t*) pair.first.first, use.c_str());
        
    }
    
}
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
    g_hm=hModule;
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

