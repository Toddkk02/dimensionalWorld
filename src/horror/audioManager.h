#pragma once

#include "raylib.h"
#include <map>
#include <string>

class AudioManager {
public:
    static AudioManager& Get();
    
    void Init();
    void Update(float tension, float deltaTime);
    void Cleanup();
    
    void PlayOneShotEffect(const std::string& name);
    void SetMasterVolume(float volume);
    
    bool IsInitialized() const { return m_initialized; }

private:
    AudioManager();
    ~AudioManager();
    
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    
    struct AudioLayer {
        Music music;
        float targetVolume;
        float currentVolume;
        float fadeSpeed;
        bool loaded;
        std::string name;
    };
    
    std::map<std::string, AudioLayer> m_layers;
    std::map<std::string, Sound> m_effects;
    
    float m_masterVolume;
    float m_lastTensionLevel;
    bool m_initialized;
    
    void LoadAudioAssets();
    void UpdateLayer(const std::string& name, float targetVol, float deltaTime);
    void CreatePlaceholderAudio();
};