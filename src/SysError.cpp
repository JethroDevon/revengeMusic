#include "SysError.h"

#include <iostream>
#include <string>
#include <sstream>

#ifdef __unix
    #include <cerrno>
    #include <cstring>
#elif _WIN32
    #include <windows.h>
#endif

static std::string LastErrorToString() {
    #ifdef __unix
        if(errno != 0) {
            std::stringstream buffer;
            buffer << "Error " << errno << " - " << std::strerror(errno);
            return buffer.str();
        }
        else
            return "errno not set!";
    #elif _WIN32
        DWORD error = GetLastError();
        if(error == 0)
            return "no error found!";
        
        LPSTR messageBuffer = nullptr;
        
        size_t size = FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        error,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) &messageBuffer,
                        0, NULL
                        );
                        
        std::string error_str(messageBuffer, size);
        
        LocalFree(messageBuffer);
        
        std::stringstream buffer;
        buffer << "Error " << GetLastError() << " - " << error_str;
        return buffer.str();
    #endif
        
}

namespace SysError {
    std::stringstream error_msg;
    bool error_set = false;

    void Set(std::string msg) {
        error_msg << msg << "\n"
                << "\t" << LastErrorToString();
        error_set = true;
    }
    
    void Print(std::string msg) {
        if(!error_set)
            Set(msg);
        std::cerr << error_msg.str() << std::endl;
    }
    
    void Log(std::string msg) {
        if(!error_set)
            Set(msg);
        //TODO
    }
}