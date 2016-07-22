#include "MessageQueue.h"
#include "Logger.h"

#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>

using namespace boost::interprocess;

MessageQueue::MessageQueue(const char* queue_name, int max_messages, int buffer_size) {
    this->queue_name = queue_name;
    this->buffer_size = buffer_size;
    buffer = (char*) std::malloc(buffer_size);
    
    boost::filesystem::path tmp = boost::filesystem::temp_directory_path();
    flock.file << tmp.native() << "/" << queue_name << ".lock";
    
    flock.handle = ipcdetail::create_or_open_file(flock.file.str().c_str(), read_write);
    ipcdetail::try_acquire_file_lock(flock.handle, flock.acquired);
    
    try {
        if(flock.acquired) {
            message_queue::remove(queue_name);
            queue = new message_queue
                    (
                        create_only,
                        queue_name,
                        max_messages,
                        buffer_size
                    );
        } else {
            queue = new message_queue
                    (
                        open_only,
                        queue_name
                    );
        }
    } catch(interprocess_exception &ex) {
        message_queue::remove(queue_name);
        Logger::PrintError(ex, "Failed to create MessageQueue!");
    }
}

MessageQueue::~MessageQueue() {
    std::free(buffer);
    ipcdetail::close_file(flock.handle);
    if(flock.acquired) {
        message_queue::remove(queue_name);
        ipcdetail::delete_file(flock.file.str().c_str());
    }
    delete(queue);
}

bool MessageQueue::is_only_instance() {
    return flock.acquired;
}

bool MessageQueue::GetMessage(std::string& msg, int timeout_ms) {
    unsigned int             _dummy_priority;
    message_queue::size_type _dummy_recvd_size;
    boost::posix_time::ptime timeout = boost::posix_time::microsec_clock::universal_time()
                                     + boost::posix_time::milliseconds(timeout_ms);
    try {
        bool received = queue->timed_receive(buffer, buffer_size,
                        _dummy_recvd_size, _dummy_priority, timeout);
        if(received) {
            msg = std::string(buffer);
            return true;
        } else {
            return false;
        }
    }
    catch(interprocess_exception &ex) {
        message_queue::remove(queue_name);
        Logger::PrintError(ex, "Could not receive message!");
        return false;
    }
}

bool MessageQueue::GetMessage(std::string& msg, message_queue::size_type recvd_size, unsigned int priority, int timeout_ms) {
    boost::posix_time::ptime timeout = boost::posix_time::microsec_clock::universal_time()
                                     + boost::posix_time::milliseconds(timeout_ms);
    try {
        bool received = queue->timed_receive(buffer, buffer_size,
                        recvd_size, priority, timeout);
        if(received) {
            msg = std::string(buffer);
            return true;
        } else {
            return false;
        }
    }
    catch(interprocess_exception &ex) {
        message_queue::remove(queue_name);
        Logger::PrintError(ex, "Could not receive message!");
        return false;
    }
}

bool MessageQueue::SendMessage(const char* msg) {
    try {
        queue->send(msg, std::strlen(msg)+1, 0);
    }
    catch(interprocess_exception &ex) {
        message_queue::remove(queue_name);
        Logger::PrintError(ex, "Could not send message!");
    }
    return true;
}
