#include "Sound.h"
#include "Pipe.h"
#include "Logger.h"

#include <fmod.hpp>
#include "fmod_errors.h"

#include <string>
#include <iostream>

//standard unix headers, need this to get present working directory
#ifdef __unix
  #include <stdlib.h>
  #include <unistd.h>
  #include <pwd.h>
  #include <sys/types.h>
  #include <errno.h>
#elif _WIN32
  #include <windows.h>
  #include <Knownfolders.h>
  #include <Shlobj.h>
  #include <cwchar>
#else
  #ERROR "Incompatible OS"
#endif

int main( int argc, char *argv[]) {

    #ifdef __unix
      errno = 0;
    #elif _WIN32
      SetLastError(0);
    #endif

    #ifdef RELEASE
      SysError::Log();
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

        if(argc < 1) {
            std::cerr << "Error, missing arguments." << std::endl;
            return -2;
        }

        std::string music_dir;
        std::string track_dir;
        std::string track_name;
        std::string subdirectory;

		//Check command line arguments
        for(int i = 1; i < argc; ++i)
        {
          if(strcmp(argv[i],"-subdir") == 0)
          {
            ++i;
            if(i < argc)
            { 
              subdirectory = argv[i]; 
              subdirectory += "/";
            }
          }
          else
          { track_name = argv[i]; }
        }

        //Get home directory of user
        #ifdef __unix
          const char* home_dir = getenv("HOME");
          if(home_dir == NULL) {
              //Get home directory if it is not defined in the environment variable
              home_dir = getpwuid(getuid())->pw_dir;
              if(home_dir == NULL) {
                  Logger::PrintError("Could not find home directory!");
                  return -1;
              }
          }

          //It is assumed the music folder is in "$HOME/Music"
          //The default music folder can be set via "$HOME/.config/user-dirs.dirs"
          music_dir = home_dir;
          music_dir += "/Music/";
        #elif _WIN32
          //On Windows all default folders can be found via a function
          PWSTR* music_dir_ptr =
            static_cast<PWSTR*>(CoTaskMemAlloc(sizeof(wchar_t)*MAX_PATH));
          wchar_t music_dir_buf[MAX_PATH];
          char char_buf[MAX_PATH];
          if(SHGetKnownFolderPath(FOLDERID_Music,0,NULL,music_dir_ptr) != S_OK) {
              Logger::PrintError("Could not find music directory!");
              return -1;
          }

          wcscpy(music_dir_buf,*music_dir_ptr);
          CoTaskMemFree(music_dir_ptr);
          wcstombs(char_buf,music_dir_buf,MAX_PATH);

          music_dir = char_buf;
          music_dir += "/";
        #endif

        track_dir += music_dir;
        track_dir += subdirectory;
        track_dir += track_name;

        Sound song(subdirectory == "" ? music_dir.c_str() : track_dir.c_str());

        song.init();
        std::cout << "Playing file: " << track_name << std::endl;
        song.play(track_dir.c_str());

        std::string msg;
        bool running = true;

        while(running) {

            msg = pipe.GetMessage();

            if(!song.isPlaying()) {
                song.play_next();
            }

            //Events
            if(msg == "none") {
                continue;
            } else if(msg == "kill") {
                std::cout << "Killed!" << std::endl;
                running = false;
            } else if(msg == "play") {
                std::cout << "Play" << std::endl;
                song.play();
            } else if(msg == "pause") {
                std::cout << "Pause" << std::endl;
                song.pause();
            } else if(msg == "next") {
                std::cout << "Next" << std::endl;
                song.play_next();
            } else if(msg == "prev") {
                std::cout << "Previous" << std::endl;
                song.play_prev();
            } else if(msg == "shuffle") {
                std::cout << "Toggle Shuffle" << std::endl;
                song.setMode(SHUFFLE);
            } else if(msg == "loop-file") {
                std::cout << "Toggle File Loop" << std::endl;
                song.setMode(LOOP_FILE);
            }

        }

        std::cout << track_name << " stopped, closing." << std::endl;
        return 0;
    }
}
