#include "Logger.h"

#include <boost/interprocess/exceptions.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>

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

namespace Logger {
    bool error_set = false;
    Logger::Error last_error(None, "No Error");

    void SetError(Logger::Error& error) {
        error.msg << "\n\t" << LastErrorToString();
        //Set error type based on last system error
        error_set = true;
        last_error = error;
    }
    
    void PrintError(Logger::Error error = Logger::Error()) {
        if(error.type == Unknown) {
            SetError(error);
        }
        else if(error_set == false) {
            error_set = true;
            last_error = error;
        }

        std::cerr << error.msg.str() << std::endl;
    }
    
    void PrintError(boost::interprocess::interprocess_exception &ex, Logger::Error error) {
        if(error_set == false) {
            error_set = true;
            last_error = error;
        }

        std::cerr << error.msg.str() << "\n" << "\tError: " << ex.what() << std::endl;
    }

    void PrintError(boost::filesystem::filesystem_error &ex, Logger::Error error) {
        if(error_set == false) {
            error_set = true;
            last_error = error;
        }

        std::cerr << error.msg.str() << "\n" << "\tError: " << ex.what() << std::endl;
    }

    void SetLog(bool logging) {
        if(logging) {
            freopen("error.txt", "w", stderr);
            freopen("log.txt", "w", stdout);
        }
        else {
            fclose(stderr);
            fclose(stdout);
        }
    }
}