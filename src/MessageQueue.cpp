#include "MessageQueue.h"

#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace boost::interprocess;

MessageQueue::MessageQueue(const char* queue_name, int max_messages, int buffer_size) {
    this->queue_name = queue_name;
    this->buffer_size = buffer_size;
    flock.name = std::string(queue_name);
    flock.name += std::string(".lock");
    
    flock.handle = ipcdetail::create_or_open_file(flock.name.c_str(), read_write);
    ipcdetail::try_acquire_file_lock(flock.handle, flock.acquired);
    
    buffer = (char*) std::malloc(buffer_size);
    
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
        std::cout << ex.what() << std::endl;
    }
}

MessageQueue::~MessageQueue() {
    std::free(buffer);
    ipcdetail::close_file(flock.handle);
    if(flock.acquired) {
        message_queue::remove(queue_name);
        ipcdetail::delete_file(flock.name.c_str());
    }
}

bool MessageQueue::is_only_instance() {
    return flock.acquired;
}

bool MessageQueue::GetMessage(std::string& msg, int timeout_ms) {
    unsigned int             _dummy_priority;
    message_queue::size_type _dummy_recvd_size;
    boost::posix_time::ptime timeout = boost::posix_time::microsec_clock::local_time()
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
        std::cout << ex.what() << std::endl;
        return false;
    }
}

bool MessageQueue::GetMessage(std::string& msg, message_queue::size_type recvd_size, unsigned int priority, int timeout_ms) {
    boost::posix_time::ptime timeout = boost::posix_time::microsec_clock::local_time()
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
        std::cout << ex.what() << std::endl;
        return false;
    }
}

bool MessageQueue::SendMessage(const char* msg) {
    try {
        queue->send(msg, std::strlen(msg)+1, 0);
    }
    catch(interprocess_exception &ex) {
        message_queue::remove(queue_name);
        std::cout << ex.what() << std::endl;
    }
    return true;
}
