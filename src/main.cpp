#include <string>
#include <stdio.h>
#include <errno.h>
#include <iostream>

#include <fmod.hpp>
#include "fmod_errors.h"

#include "Sound.h"
#include "Pipe.h"

//standard unix headers, need this to get present working directory
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

int main( int argc, char *argv[]) {

    Pipe pipe("/tmp/fifo");

    if(!pipe.isOnlyInstance()) {
        if(argv[1] == NULL) {
            pipe.SendMessage("kill");
            return 0;
        } else {
            pipe.SendMessage(argv[1]);
            return 0; 
        }
    } else if(pipe.isOnlyInstance()) {
        
        if(argv[1] == NULL) {
            std::cerr << "Error track name invalied" << std::endl;
            return -2;
        }
        
        std::string music_dir;
        const char* home_dir;
        std::string track = argv[1];
        
        //Get home directory of user
        home_dir = getenv("HOME");
        if(home_dir == NULL) {
            //Get home directory if it is not defined in the environment variable
            home_dir = getpwuid(getuid())->pw_dir;
            if(home_dir == NULL) {
                std::cer << "Could not get home directory:\n\t" << std::strerror(errno) << std::endl;
                return -1;
            }
        }
        
        music_dir = home_dir;
        music_dir += "/Music/";
        music_dir += track;
        
        Sound song;
        std::cout << "Playing file: " << track << std::endl;
        song.createSound(music_dir.c_str());
        song.playSound(false);
        
        std::string msg;
        bool running = true;
        
        while(song.isPlaying() && running) {
            //GetMessage() is blocking which might need to be changed
            msg = pipe.GetMessage();
            
            //Events
            if(msg == "kill") {
                std::cout << "Killed!" << std::endl;
                running = false;
            }
        }
        
        std::cout << track << " stopped, closing." << std::endl;
        return 0;
    }
}
