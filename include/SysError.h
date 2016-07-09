#pragma once

#include <string>
#include <sstream>

namespace SysError {
    void Set(std::string msg);
    void Print(std::string msg = "Unknown"); //Can also set errors
    void Log(std::string msg = "Unknown"); //Can also set errors
    
    extern std::stringstream error_msg;
    extern bool error_set;
}