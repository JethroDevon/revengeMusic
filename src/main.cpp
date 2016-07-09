#include "Sound.h"
#include "Pipe.h"
#include "SysError.h"

#include <fmod.hpp>
#include "fmod_errors.h"

#include <string>
#include <cstring>
#include <stdio.h>
#include <errno.h>
#include <iostream>

//standard unix headers, need this to get present working directory
#ifdef __unix
  #include <stdlib.h>
  #include <unistd.h>
  #include <pwd.h>
  #include <sys/types.h>
#elif __WIN32
  #include <windows.h>
#else
  #ERROR "Incompatible OS"
#endif

int main( int argc, char *argv[]) {
    
    #ifdef __unix
        errno = 0;
    #endif

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
        #ifdef __unix
          home_dir = getenv("HOME");
        if(home_dir == NULL) {
            //Get home directory if it is not defined in the environment variable
            home_dir = getpwuid(getuid())->pw_dir;
            if(home_dir == NULL) {
                SysError::Print("Could not find home directory!");
                return -1;
            }
        }
        #elif __WIN32
          char home_dir_buf[100];
          if(GetEnvironmentVariable("HOMEPATH",home_dir_buf,100) == 0) {
              SysError::Print("Could not find home directory!");
              return -1;
          }
          home_dir = home_dir_buf;
        #endif

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
