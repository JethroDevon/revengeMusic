#include "Sound.h"
#include "MessageQueue.h"
#include "Logger.h"

#include <fmod.hpp>
#include "fmod_errors.h"

#include <thread>
#include <chrono>
#include <string>
#include <iostream>
#include <cstring>

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
  #undef GetMessage
  #undef SendMessage
#else
  #ERROR "Incompatible OS"
#endif

#define MAX_MESSAGES 2
#define MAX_MESSAGE_BYTES 128


int main( int argc, char *argv[]) {

    #ifdef __unix
      errno = 0;
    #elif _WIN32
      SetLastError(0);
    #endif

    #ifdef RELEASE
      SysError::Log();
    #endif

    MessageQueue mq("revengeMusic", MAX_MESSAGES, MAX_MESSAGE_BYTES);

    if(!mq.is_only_instance()) {
        std::string msg;
        if(strcmp(argv[1],"") == 0) {
            mq.SendMessage("kill");
        } else if(static_cast<std::string>(argv[1]) == "help") {
            msg =
                "\nUsage: revengeMusic (--commands | <path>)\n"
                "\t-h, --help\tShows this message\n"
                "\t-subdir\t\tSpecify a specific folder within the Music directory\n"
                "\tkill\t\tExits revengeMusic\n"
                "\tplay\t\tUnpause song\n"
                "\tpause\t\tPause song\n"
                "\tnext\t\tPlay next song (based on shuffle)\n"
                "\tprev\t\tPlay previous song\n"
                "\tshuffle\t\tToggles shuffle on/off\n"
                "\tloop-file\tLoops the current song\n";
        } else {
            mq.SendMessage(argv[1]);
            //Get message from player
            int timeout_ms = 16;
            mq.GetMessage(msg, timeout_ms);
        }

        std::cout << msg << std::endl;
        return 0;

    } else if(mq.is_only_instance()) {

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

            if(!song.isPlaying()) {
                song.play_next();
            }
            
            if(mq.GetMessage(msg)) {

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
                } else {
                    mq.SendMessage("Invalid");
                }
                //Delay to prevent sending message to self
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        std::cout << track_name << " stopped, closing." << std::endl;
        return 0;
    }
}
