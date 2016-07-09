#pragma once

#include <string>
#include <sstream>

namespace SysError {
    void Set(std::string msg);
    void Print(std::string msg = "Unknown"); //Can also set errors
    void Log(); //Toggles redirecting error messages to log file
    
    extern std::stringstream error_msg;
    extern bool error_set;
	extern bool logging;
}