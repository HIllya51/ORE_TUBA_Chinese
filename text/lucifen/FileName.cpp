#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include<io.h>
#include <set>
#include <map>
#include <vector>
#include<map>
#include <fcntl.h>
#include<Windows.h>
#include<regex> 
#include"json.hpp"
using json = nlohmann::json;
std::vector<std::map<uint32_t, uint32_t>> maps;

std::map<uint32_t, std::string> examples;

//deduplicate identical strings within each scene even if they have different addresses
bool deduplciate = true;
std::set<uint32_t> seen;

std::string WStrToStr(const std::wstring& wstrString, UINT uCodePage)
{
    int lenStr = 0;
    std::string result;

    lenStr = WideCharToMultiByte(uCodePage, NULL, wstrString.c_str(), wstrString.size(), NULL, NULL, NULL, NULL);
    char* buffer = new char[lenStr + 1];
    WideCharToMultiByte(uCodePage, NULL, wstrString.c_str(), wstrString.size(), buffer, lenStr, NULL, NULL);
    buffer[lenStr] = '\0';

    result.append(buffer);
    delete[] buffer;
    return result;
}
std::string WStrToStr(const wchar_t* wstrString, UINT uCodePage)
{
    std::wstring ss = wstrString;
    return WStrToStr(ss, uCodePage);

}
std::wstring StrToWStr(const std::string& strString, UINT uCodePage)
{
    int lenWStr = 0;
    std::wstring result;

    lenWStr = MultiByteToWideChar(uCodePage, NULL, strString.c_str(), strString.size(), NULL, NULL);
    wchar_t* buffer = new wchar_t[lenWStr + 1];
    MultiByteToWideChar(uCodePage, NULL, strString.c_str(), strString.size(), buffer, lenWStr);
    buffer[lenWStr] = '\0';

    result.append(buffer);
    delete[] buffer;
    return result;
}

std::vector<std::wstring> stringSplit(const std::wstring& str, const std::wstring& s) {

    std::wregex reg(s);
    std::vector<std::wstring> elems(std::wsregex_token_iterator(str.begin(), str.end(), reg, -1),
        std::wsregex_token_iterator());
    return elems;

}
#include<fstream>
json createmap(char* fname) {
    auto f = std::ifstream(fname);
    auto _js = json::parse(f);
     
    return _js;
}
int checkeng(std::string x) {
    int i = 1;
    for (int p = 0; p < x.length(); p += 1) {
        if ((x.c_str()[p] & 0x80) != 0) {
            i = 0;
            break;

        }
    }
    return i;
}
void packstring(FILE* f, std::string  my, std::string  jp, int savestringp, int saveallstringlen, int header_end) {
    char x[10] = { 0 };
    if(false){// (my.length() <= jp.length()) {
        int l = jp.length() - my.length();
        fwrite(my.c_str(), 1, my.length(), f);

        fwrite(&x, 1, l, f);

    }
    else {
        int off = 0;
        if (my[my.length() - 1] == '$')off = 3;

        int xxx = savestringp;
        fseek(f, savestringp, SEEK_SET);
        fread(&xxx, 4, 1, f);
        //   printf("%08X\n", xxx);
        fseek(f, 0, SEEK_END);

        fwrite(&x, 4, 1, f);
        unsigned int stringstart = ftell(f) - header_end - off;

        
        fwrite(my.c_str(), 1, my.length(), f);
        fwrite(&x, 4, 1, f);

        int end = ftell(f) - header_end;
        fseek(f, savestringp, SEEK_SET);

        fwrite(&stringstart, 4, 1, f);
        fseek(f, saveallstringlen, SEEK_SET);
        fwrite(&end, 4, 1, f);

    }



}
int main(int argc, char** argv)
{
    int strings = 0;
    int totalspace = 0;
    
    maps.clear();
    seen.clear();
    /*
    for (int i=0; i < argc; i += 1) {
        printf("%s\n", argv[i]);
    } */

    auto f = fopen(argv[1], "rb");//Դsob
    fseek(f, 0, SEEK_END);
    int end = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buff = (char*)malloc(end);
    fread(buff, 1, end, f);
    fseek(f, 0, SEEK_SET);

    std::map<int, int>reverse;

    auto f_pack = fopen(argv[3], "wb"); 
    fwrite(buff, 1, end, f_pack); 
    //std::map<std::wstring, std::wstring>trans;
    auto trans = createmap(argv[2]);
    // printf("create map ok\n");
    char magic[4];
    int n = fread(magic, 1, 4, f);
    if (n != 4 || strncmp(magic, "SOB0", 4) != 0) return 1;


    uint32_t table_size;
    fread(&table_size, 4, 1, f);
    uint32_t table_end = table_size + 0x8;
    int header_count = 0;
    //int j = 0;
    while (ftell(f) < table_end)
    {
        //printf("Header %d at %08X:\n", j++, ftell(f));
        uint32_t word_pairs;
        fread(&word_pairs, 4, 1, f);
        std::map<uint32_t, uint32_t> newmap;
        for (int i = 0; i < word_pairs; i++)
        {
            uint32_t key;
            uint32_t value;
            fread(&key, 4, 1, f);

            reverse.insert(std::pair<int, int>(key, ftell(f)));
            fread(&value, 4, 1, f);



            newmap[key] = value;
            //printf("%08X: %08X %08X\n", key, value, ftell(f));
        }
        maps.push_back(newmap);
        header_count += 1;
    }
    if (maps.size() < 5)
    {
        puts("not enough maps");
        exit(0);
    }
    fseek(f, table_end, SEEK_SET);
    uint32_t strings_start;
    uint32_t strings_end;
    fread(&strings_start, 4, 1, f);
    int stringendp = ftell(f);
    fread(&strings_end, 4, 1, f);
    //printf("%08X\n", strings_end);

    totalspace += strings_end - strings_start;
    uint32_t header_end = ftell(f);
    uint32_t code_end = strings_start + header_end;
    //printf("String range: %08X~%08X\n", strings_start, strings_end);

    // can't tell if text is content text until a command tries to use it? I think?
    std::string text_memory;
    uint32_t text_memory_addr;

    bool justdidtext = false;

    // no idea if commands have proper inline arguments in this bytecode lol
    while (ftell(f) < code_end)
    {
        uint32_t address = ftell(f) - header_end;
        uint32_t command;
        fread(&command, 4, 1, f);
        int isselect = 0;

        if (((command & 0xFFF00000) == 0x01800000) || ((command & 0xFFF00000) == 0x01700000) && maps[4].count(address) && maps[4][address] >= strings_start && maps[4][address] < strings_end)
        {

            strings++;


            fseek(f, maps[4][address] + header_end, SEEK_SET);
            
            int c = fgetc(f);

            auto savec = c;
            
            if (c == 0) // 00 XX XX <string
                fseek(f, 2, SEEK_CUR);
            else // <string>
                fseek(f, -1, SEEK_CUR);
            auto _ = fgetc(f);
            if (_ == 2 || _ == 3) { 
                fseek(f, 1, SEEK_CUR);
                isselect = _;
                 
            }
            else {
                fseek(f, -1, SEEK_CUR);
            }

            
            if (isselect == 0) {

            
                auto start = ftell(f);
                while ((c = fgetc(f)) != 0);
                auto end = ftell(f);
                char* text = (char*)malloc(end - start);
                fseek(f, start, SEEK_SET);
                fread(text, 1, end - start, f);
                std::string str = std::string(text);
                if (!examples.count(command))
                    examples[command] = str;

                if (str != "" && str.length() > 1) {
                    if (checkeng(str) == 0) {

                        fseek(f_pack, start, SEEK_SET);
                        std::wstring u16str = StrToWStr(str, 932);
                        std::string u8str = WStrToStr(u16str, 65001);
                        char xx[10000] = { 0 };
                        sprintf(xx, u8"○%08X○%s", maps[4][address], u8str.c_str());
                        u8str = xx;

                        if (trans.find(u8str) == trans.end()) {
                            system("chcp 65001");
                            printf("didn't trans %s\n", u8str.c_str());

                        }
                        else {

                            //    printf("%08X %08X %08X\n",address, maps[4][address], maps[4][address]+header_end);
                             //   printf("%s\n", str.c_str());
                            std::string gkbstr = WStrToStr(StrToWStr(trans.at(u8str),65001), 936);
                            fseek(f_pack, start, SEEK_SET);
                            packstring(f_pack, gkbstr, str, reverse.at(address), stringendp, header_end);
                        }
                    }
                    else {
                        //  printf("%08X %08X\n", maps[4][address], maps[4][address] + header_end);
                          //printf("%s\n", str.c_str());
                    }

                }
            }
            else {
                if (savec != 0)continue;
                std::vector<std::string>saveraw;
                std::vector<std::string>savetrans; 
                //MessageBoxW(0, std::to_wstring(isselect).c_str(),L"num", 0);
                short saveisselect = isselect;
                auto savep = ftell(f);
                while (isselect) {
                    isselect -= 1;
                    fseek(f, 2, SEEK_CUR);
                    auto start = ftell(f);
                    while ((c = fgetc(f)) != 0);
                    auto end = ftell(f);
                    char* text = (char*)malloc(end - start);
                    fseek(f, start, SEEK_SET);
                    fread(text, 1, end - start, f);


                    std::string str = std::string(text);

                    saveraw.push_back(str);


                    std::wstring u16str = StrToWStr(str, 932);
                    
                    std::string u8str = WStrToStr(u16str, 65001);
                    char xx[10000] = { 0 };
                    sprintf(xx, u8"○%08X○%s", maps[4][address], u8str.c_str());
                    u8str = xx;
                    if (trans.find(u8str) == trans.end()) {
                        savetrans.push_back(WStrToStr(StrToWStr(u8str, 65001), 936));
                        printf("didn't trans %s\n", str.c_str());
                    }
                    else {
                        std::string gkbstr = WStrToStr(StrToWStr(trans.at(u8str), 65001), 936);
                        savetrans.push_back(WStrToStr(StrToWStr(trans.at(u8str), 65001), 936));
                        //MessageBoxW(0, StrToWStr(trans.at(u8str), 65001).c_str(), L"1", 0);
                    }
                } 

                    char x[10] = { 0 };

                    fseek(f_pack, savep, SEEK_SET);
                    
                    for (auto& trans : savetrans) {
                        short leng = trans.length() + 1 + 2;
                        fwrite(&leng, 2, 1, f_pack);
                        fwrite(trans.c_str(), 1, trans.length(), f_pack);
                        fwrite("\0", 1, 1, f_pack); 
                    }  
                    

            }
            
        }
        else
        {
            // I have NO CERTAINTY that this is correct
            if ((command == 0x000001C8 || command == 0x000001CF) && text_memory != "" && (!deduplciate || !seen.count(text_memory_addr)))
            {
                //puts(text_memory.data());
                if (deduplciate) seen.insert(text_memory_addr);
            }

            justdidtext = false;
        }
        fseek(f, address + header_end + 4, SEEK_SET);
    }

    fclose(f);
    fclose(f_pack);

    //printf("Total space for strings: %d\n", totalspace);
    //printf("Total strings: %d\n", strings);
    //for(auto &[k, v] : examples)
    //{
    //    printf("%08X: %s\n", k, v.data());
    //}
}