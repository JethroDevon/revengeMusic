#pragma once

#include <boost/interprocess/exceptions.hpp>
#include <boost/filesystem/operations.hpp>

#include <string>
#include <sstream>


enum ErrorType {
    None,
    Unknown,
    Recoverable,
    Fatal
};

namespace Logger {
    struct Error;

    void SetError(Logger::Error& error);
    void PrintError(Logger::Error error);
    void PrintError(boost::interprocess::interprocess_exception &ex, Logger::Error error);
    void PrintError(boost::filesystem::filesystem_error &ex, Logger::Error error);

    void SetLog(bool logging); //Toggles redirecting error messages to log file
    
    extern bool error_set;
    extern Logger::Error last_error;

    struct Error {
        ErrorType type;
        std::stringstream msg;

        Error() {
            type = Unknown;
            msg.str();
        }

        Error(ErrorType type, std::string msg = "") {
            this->type = type;
            this->msg.str(msg);
        }

        //std::stringstream is not copyable
        //its contents must be manually swapped
        Error& operator=(const Error& other) {
            type = other.type;
            msg.str(other.msg.str());
            return *this;
        }

        Error(const Error& other) : msg(other.msg.str()) {
            type = other.type;
        }
    };
}
