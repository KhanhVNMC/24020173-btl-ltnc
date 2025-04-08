//
// Created by GiaKhanhVN on 4/8/2025.
//
#include "jsystemstd.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <algorithm>

int bgmAudioVolume = 50;
int sfxAudioVolume = 100;

namespace SysAudio {
    namespace {
        inline std::unordered_map<std::string, Mix_Chunk*> soundCache;
        inline std::unordered_map<int, Mix_Chunk*> activeChannels;
    }

    int getBGMVolume() {
        return bgmAudioVolume;
    }

    int getSFXVolume() {
        return sfxAudioVolume;
    }

    bool initSoundSystem() {
        if (SDL_Init(SDL_INIT_AUDIO) < 0) return false;
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return false;

        Mix_AllocateChannels(64);
        return true;
    }

    void precache(const std::string& path, int volume, bool repeat) {
        static const std::string basePath = "../assets/sfxbgm/";
        const std::string fullPath = basePath + path;

        if (soundCache.count(fullPath) == 0) {
            Mix_Chunk* chunk = Mix_LoadWAV(fullPath.c_str());
            if (!chunk) return;
            soundCache[fullPath] = chunk;
        }
    }

    void playSoundAsync(const std::string& path, int volume, bool repeat) {
        static const std::string basePath = "../assets/sfxbgm/";
        const std::string fullPath = basePath + path;

        if (soundCache.count(fullPath) == 0) {
            Mix_Chunk* chunk = Mix_LoadWAV(fullPath.c_str());
            if (!chunk) return;
            soundCache[fullPath] = chunk;
        }

        Mix_Chunk* chunk = soundCache[fullPath];
        volume = std::clamp(volume, 0, 100);
        Mix_VolumeChunk(chunk, (volume * MIX_MAX_VOLUME) / 100);

        int channel = Mix_PlayChannel(-1, chunk, repeat ? -1 : 0);
        if (channel != -1) {
            activeChannels[channel] = chunk;
        }
    }

    void stopAudio() {
        for (auto& [channel, _] : activeChannels) {
            Mix_HaltChannel(channel);
        }
        activeChannels.clear();
    }

    void shutdownSoundSystem() {
        stopAudio();
        for (auto& [_, chunk] : soundCache) {
            Mix_FreeChunk(chunk);
        }
        soundCache.clear();
        Mix_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
}
