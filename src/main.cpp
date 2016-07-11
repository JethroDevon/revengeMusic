#include "Sound.h"
#include "Pipe.h"
#include "SysError.h"

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

        if(argv[1] == NULL) {
            std::cerr << "Error track name invalid" << std::endl;
            return -2;
        }

        std::string music_dir;
        std::string track_dir;
        std::string track_name = argv[1];

        //Get home directory of user
        #ifdef __unix
          const char* home_dir = getenv("HOME");
          if(home_dir == NULL) {
              //Get home directory if it is not defined in the environment variable
              home_dir = getpwuid(getuid())->pw_dir;
              if(home_dir == NULL) {
                  SysError::Print("Could not find home directory!");
                  return -1;
              }
          }
          
          //It is assumed the music folder is in "$HOME/Music"
          //The default music folder can be set via "$HOME/.config/user-dirs.dirs"
          music_dir = home_dir;
          music_dir += "/Music/";
        #elif _WIN32
          //On Windows all default folders can be found via a function
          PWSTR* home_dir_ptr =
            static_cast<PWSTR*>(CoTaskMemAlloc(sizeof(wchar_t)*MAX_PATH));
          wchar_t music_dir_buf[MAX_PATH];
          char char_buf[MAX_PATH];
          if(SHGetKnownFolderPath(FOLDERID_Music,0,NULL,music_dir_ptr) != S_OK) {
              SysError::Print("Could not find music directory!");
              return -1;
          }
          
          wcscpy(music_dir_buf,*music_dir_ptr);
          CoTaskMemFree(music_dir_ptr);
          wcstombs(char_buf,music_dir_buf,MAX_PATH);
          
          music_dir = char_buf;
          music_dir += "/";
        #endif
        
        track_dir += music_dir;
        track_dir += track_name;

        Sound song;
        std::cout << "Playing file: " << track_name << std::endl;
        song.createSound(track_dir.c_str());
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

        std::cout << track_name << " stopped, closing." << std::endl;
        return 0;
    }
}
