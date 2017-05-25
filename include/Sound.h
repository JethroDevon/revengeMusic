#ifndef SOUND_H
#define SOUND_H

#include <fmod.hpp>

#include <vector>
#include <string>

enum {
    LOOP_FILE   = 1,
    SHUFFLE     = 2
};

class Sound {

    public:
        
        Sound(const char* music_dir, int flags = SHUFFLE);
        ~Sound();
        
        bool init();

        bool isPlaying();
        
        void setMode(int flags);
        void play();
        void play(const char* track_dir);
        void pause();
        void play_next();
        void play_prev();
        
    private:
        
        void getFileList(const char* music_dir);
        std::string getCurrentSong();
        
        struct Mode {
            bool loop_file;
            bool shuffle;
        } mode;
        
        const char* music_dir;
        std::vector<std::string> filelist;
        std::vector<std::string> playedFiles;
        FMOD::Channel *channel;
        FMOD::Sound *newSound;
        FMOD::System *m_pSystem;
};

#endif
