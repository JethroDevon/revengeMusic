#pragma once

#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <sstream>

using namespace boost::interprocess;
namespace filesystem = boost::filesystem;


class MessageQueue {
    public:
    
        MessageQueue(const char* queue_name, int max_messages, int buffer_size);
        ~MessageQueue();
        
        bool SendMessage(const char* msg);
        bool GetMessage(std::string& msg, int timeout_ms = 16);
        bool GetMessage(std::string& msg, message_queue::size_type recvd_size,
                        unsigned int priority, int timeout_ms = 16);
        
        bool is_only_instance();
        
    private:
        
        char*               buffer;
        int                 buffer_size;
        message_queue*      queue;
        const char*         queue_name;
        
        struct {
            filesystem::path    file;
            file_handle_t       handle;
            bool                acquired;
        } flock;
};