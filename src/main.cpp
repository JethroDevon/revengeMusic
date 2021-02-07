#include "Sound.h"
#include "MessageQueue.h"
#include "Logger.h"
#include "config.h"

#include <fmod.hpp>
#include "fmod_errors.h"

#include <thread>
#include <chrono>
#include <string>
#include <vector>
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
  #include "WindowsUtility.h"
  #undef GetMessage
  #undef SendMessage
#else
  #ERROR "Incompatible OS"
#endif

#define MAX_MESSAGES 2
#define MAX_MESSAGE_BYTES 128


int main( int argc, char *argv[]) {

    std::vector<std::string> argv_str;
    
    #ifdef __unix
      //Store arguments in vector of string
      for(int i = 0; i < argc; ++i) {
          argv_str.emplace_back(argv[i]);
      }
      
      errno = 0;
    #elif _WIN32
      std::vector<wchar_t*> windows_args;
      
      if(!GetCommandLineToArgs(windows_args)) {
          Logger::Error error(ErrorType::Fatal, "Could not interpret command line arguments!");
          Logger::PrintError(error);
          return -1;
      }
      
      for(wchar_t* UTF16str : windows_args) {
          argv_str.emplace_back(UTF16toUTF8string(UTF16str));
      }
      
      SetLastError(0);
    #endif

    #ifdef RELEASE
      SysError::SetLog(true);
    #endif

    if(argv_str.size() > 1) {
        //Check for simple commands like --help or --version
        if(argv_str[1] == "-h" || argv_str[1] == "--help") {
            std::string msg =
                "\nUsage: revengeMusic (--commands | <path>)\n"
                "\t-h, --help\tShows this message\n"
                "\t-v, --version\tShow version number\n"
                "\t-subdir\t\tSpecify a specific folder within the Music directory\n"
                "\tkill\t\tExits revengeMusic\n"
                "\tplay\t\tUnpause song\n"
                "\tpause\t\tPause song\n"
                "\tnext\t\tPlay next song (based on shuffle)\n"
                "\tprev\t\tPlay previous song\n"
                "\tshuffle\t\tToggles shuffle on/off\n"
                "\tloop-file\tLoops the current song\n";

            std::cout << msg << std::endl;
            return 0;
        } else if(argv_str[1] == "-v" || argv_str[1] == "--version") {
            std::cout << PROJECT_NAME << " v" << PROJECT_VERSION << std::endl;
            return 0;
        }
    }

    MessageQueue mq_to_client("revengeMusicToClient", MAX_MESSAGES, MAX_MESSAGE_BYTES);
    MessageQueue mq_to_player("revengeMusicToPlayer", MAX_MESSAGES, MAX_MESSAGE_BYTES);

    if(!mq_to_player.is_only_instance()) {
        std::string msg;
        std::string cmd;

        if(argc == 1) {
            cmd = "kill";
        } else {
            cmd = argv_str[1];
        }

        mq_to_player.SendMessage(cmd.c_str());
        //Get message from player
        int timeout_ms = 16;
        mq_to_client.GetMessage(msg, timeout_ms);

        std::cout << msg << std::endl;
        return 0;

    } else if(mq_to_player.is_only_instance()) {
        std::string music_dir;
        std::string track_dir;
        std::string track_name;
        std::string subdirectory;

        //Check command line arguments
        for(unsigned int i = 1; i < argv_str.size(); ++i) {
            if(argv_str[i] == "-subdir")
            {
                ++i;
                if(i < argv_str.size())
                {
                  subdirectory = argv_str[i];
                  subdirectory += "/";
                }
            }
            else {
                track_name = argv_str[i];
            }
        }

        //Get home directory of user
        #ifdef __unix
          const char* home_dir = getenv("HOME");
          if(home_dir == NULL) {
              //Get home directory if it is not defined in the environment variable
              home_dir = getpwuid(getuid())->pw_dir;
              if(home_dir == NULL) {
                  Logger::Error error(ErrorType::Fatal, "Could not find home directory!");
                  Logger::PrintError(error);
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
              Logger::Error error(ErrorType::Fatal, "Could not find music directory!");
              Logger::PrintError(error);
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

        if(Logger::error_set) {
                if(Logger::last_error.type == ErrorType::Fatal) {
                    std::cout << "A fatal error has occured,"
                              << "terminating program!" << std::endl;
                    return 1;
                } else {
                    Logger::error_set = false;
                }
        }

        std::cout << "Playing file: " << track_name << std::endl;
        if(track_name == "") {
            song.play();
        } else {
            song.play(track_dir.c_str());
        }

        std::string msg;
        bool running = true;

        while(running) {

            if(Logger::error_set) {
                if(Logger::last_error.type == ErrorType::Fatal) {
                    std::cout << "A fatal error has occured,"
                              << "terminating program!" << std::endl;
                    return 1;
                } else {
                    Logger::error_set = false;
                }
            }

            if(!song.isPlaying()) {
                song.play_next();
            }
            
            if(mq_to_player.GetMessage(msg)) {

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
                    mq_to_client.SendMessage("Invalid");
                }
            }
        }

        std::cout << track_name << " stopped, closing." << std::endl;
        return 0;
    }
}
