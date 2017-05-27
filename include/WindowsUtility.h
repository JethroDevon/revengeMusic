#pragma once

#include <Windows.h>

//Takes a null terminated wchar_t string and returns a std::string with UTF8 encoding
//Returns an empty string if the conversion fails
std::string UTF16toUTF8string(wchar_t* UTF16str)
{
    //Check for invalid pointer
    if(UTF16str == nullptr || UTF16str == NULL) { 
        return std::string();
    }

    //Calculate how much space is needed for UTF8 string
    int num_bytes = WideCharToMultiByte(CP_UTF8, 0, UTF16str, -1, NULL, 0, NULL, NULL );
    
    //Allocate memory for multibyte string
    char* UTF8tempstr = new char[num_bytes];

    int result = WideCharToMultiByte(
        CP_UTF8, //Code Page
        0, //Options flags
        UTF16str, //Input string
        -1, //Specifies to convert up until null character and appends null character
        UTF8tempstr, //Output string
        num_bytes, //Size in bytes of buffer for output string
        NULL, //Must be NULL when converting to UTF8
        NULL  //Must be NULL when converting to UTF8
    );
    
    std::string UTF8str;
    
    if(result != 0) {
        UTF8str = UTF8tempstr;
    }
        
    delete[] UTF8tempstr;
    
    return UTF8str;
}

bool GetCommandLineToArgs(std::vector<wchar_t*> & output)
{
    int nArgs;
    LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    
    if(szArglist == NULL) {
      return false;
    }
    
    for(int count = 0; count < nArgs; ++count) {
        output.emplace_back(szArglist[count]);
    }
    
    return true;
}
    