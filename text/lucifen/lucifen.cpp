#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
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
#include <set>
#include <map>
#include <vector>
#include<Windows.h>

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
int main(int argc, char** argv)
{
    
    int strings = 0;
    int totalspace = 0;
    for (int i = 0; i < argc; i++)
    {
        maps.clear();
        seen.clear();

        auto f = fopen(argv[i], "rb");
        if (!f) continue;
        char magic[4];
        int n = fread(magic, 1, 4, f);
        if (n != 4 || strncmp(magic, "SOB0", 4) != 0) continue;
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
                fread(&value, 4, 1, f);
                newmap[key] = value;
                //printf("%08X: %08X\n", key, value);
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
        fread(&strings_end, 4, 1, f);
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
            
            
            if (((command & 0xFFF00000) == 0x01800000)||((command & 0xFFF00000) == 0x01700000 ) && maps[4].count(address) && maps[4][address] >= strings_start && maps[4][address] < strings_end)
            {
                //printf("Found command that uses string: ");
                //printf("%08X: %08X (%08X) / ", address, command, maps[4][address], argv[i]);
                //printf("|%08X|:(|%08X|) [%s]\n", address+header_end, maps[4][address]+header_end, argv[i]);
                strings++;
                fseek(f, maps[4][address] + header_end, SEEK_SET);
                int c = fgetc(f);
                //printf("%08X  %08X\n", command,ftell(f));
                if (c == 0) // 00 XX XX <string
                    fseek(f, 2, SEEK_CUR);
                else // <string>
                    fseek(f, -1, SEEK_CUR);
                
                
                auto _ = fgetc(f); 
                if (_ == 2||_==3) {//STX,选择支
                    fseek(f, 3, SEEK_CUR);
                    isselect = _-1;
                }
                else {
                    fseek(f, -1, SEEK_CUR);
                }
                
                auto start = ftell(f);
                while ((c = fgetc(f)) != 0);
                auto end = ftell(f);
                char* text = (char*)malloc(end - start);
                fseek(f, start, SEEK_SET);
                fread(text, 1, end - start, f);
                

                std::string str = std::string(text);
                free(text);
                if (!examples.count(command))
                    examples[command] = str;

                if (str != "" && str.length()>1)
                    //    printf("%08X text %s\n", command, text);
                {
                    //printf("%08X text %s\n", command, text);
                    if (checkeng(str)) {
                       // printf("is eng %s\n", str.c_str());
                        //continue;
                    }
                    else {
                        printf(WStrToStr(L"○%08X○%s\n", CP_UTF8).c_str(), maps[4][address], WStrToStr(StrToWStr(str, 932), CP_UTF8).c_str());
                        printf(WStrToStr(L"●%08X●%s\n\n", CP_UTF8).c_str(), maps[4][address], WStrToStr(StrToWStr(str, 932), CP_UTF8).c_str());
                    }
                    
                }
                
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
                    if (!examples.count(command))
                        examples[command] = str;

                    if (str != "" && str.length() > 1)
                        //    printf("%08X text %s\n", command, text);
                    {
                       
                        printf(WStrToStr(L"○%08X○%s\n", CP_UTF8).c_str(), maps[4][address], WStrToStr(StrToWStr(text, 932), CP_UTF8).c_str());
                        printf(WStrToStr(L"●%08X●%s\n\n", CP_UTF8).c_str(), maps[4][address], WStrToStr(StrToWStr(text, 932), CP_UTF8).c_str());
                    }

                }

                    //printf("%08X: %08X %s\n", address+header_end, command, text);

                //text_memory = str;
                //text_memory_addr = maps[4][address];

                
                justdidtext = true;
            }
            else
            {
                //// I have NO CERTAINTY that this is correct
                //if ((command == 0x000001C8 || command == 0x000001CF) && text_memory != "" && (!deduplciate || !seen.count(text_memory_addr)))
                //{
                //   // puts(text_memory.data());
                //    if (deduplciate) seen.insert(text_memory_addr);
                //}

                //justdidtext = false;
            }
            fseek(f, address + header_end + 4, SEEK_SET);
        }

        fclose(f);
    }
    //printf("Total space for strings: %d\n", totalspace);
    //printf("Total strings: %d\n", strings);
    //for(auto &[k, v] : examples)
    //{
    //    printf("%08X: %s\n", k, v.data());
    //}
}