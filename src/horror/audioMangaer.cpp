#include "audioManager.h"
#include <cmath>

AudioManager::AudioManager() : 
    m_masterVolume(0.7f), 
    m_lastTensionLevel(0),
    m_initialized(false) {
}

AudioManager::~AudioManager() {
    if (m_initialized) {
        Cleanup();
    }
}

AudioManager& AudioManager::Get() {
    static AudioManager instance;
    return instance;
}

void AudioManager::Init() {
    if (m_initialized) {
        TraceLog(LOG_WARNING, "AudioManager already initialized");
        return;
    }
    
    InitAudioDevice();
    
    if (!IsAudioDeviceReady()) {
        TraceLog(LOG_ERROR, "Failed to initialize audio device");
        return;
    }
    
    LoadAudioAssets();
    
    m_initialized = true;
    TraceLog(LOG_INFO, "✓ Audio Manager initialized");
}

void AudioManager::CreatePlaceholderAudio() {
    TraceLog(LOG_INFO, "Creating placeholder audio (silent tracks)");
    
    const char* layerNames[] = {"base", "whispers", "drone", "distortion", "void"};
    
    for (int i = 0; i < 5; i++) {
        AudioLayer layer;
        layer.targetVolume = 0.0f;
        layer.currentVolume = 0.0f;
        layer.fadeSpeed = 0.1f;
        layer.loaded = false;
        layer.name = layerNames[i];
        
        m_layers[layerNames[i]] = layer;
        
        TraceLog(LOG_INFO, "  Created placeholder layer: %s", layerNames[i]);
    }
}

void AudioManager::LoadAudioAssets() {
    TraceLog(LOG_INFO, "Loading audio assets...");
    
    const char* paths[] = {
        "assets/audio/ambient_base.ogg",
        "assets/audio/whispers_low.ogg",
        "assets/audio/drone_medium.ogg",
        "assets/audio/reality_distortion.ogg",
        "assets/audio/void_screams.ogg"
    };
    
    const char* names[] = {
        "base",
        "whispers",
        "drone",
        "distortion",
        "void"
    };
    
    int loadedCount = 0;
    
    for (int i = 0; i < 5; i++) {
        AudioLayer layer;
        layer.targetVolume = 0.0f;
        layer.currentVolume = 0.0f;
        layer.fadeSpeed = 0.1f;
        layer.loaded = false;
        layer.name = names[i];
        
        if (FileExists(paths[i])) {
            layer.music = LoadMusicStream(paths[i]);
            
            if (layer.music.stream.buffer != NULL) {
                layer.loaded = true;
                PlayMusicStream(layer.music);
                SetMusicVolume(layer.music, 0.0f);
                loadedCount++;
                TraceLog(LOG_INFO, "  ✓ Loaded audio layer: %s", names[i]);
            } else {
                TraceLog(LOG_WARNING, "  ✗ Failed to load: %s", paths[i]);
            }
        } else {
            TraceLog(LOG_WARNING, "  ✗ File not found: %s", paths[i]);
        }
        
        m_layers[names[i]] = layer;
    }
    
    TraceLog(LOG_INFO, "Audio layers loaded: %d/5", loadedCount);
    
    if (loadedCount == 0) {
        TraceLog(LOG_WARNING, "No audio files found - running in silent mode");
        CreatePlaceholderAudio();
    }
    
    const char* effectPaths[] = {
        "assets/audio/watcher_seen.ogg",
        "assets/audio/portal_whisper.ogg",
        "assets/audio/monument_activate.ogg"
    };
    
    const char* effectNames[] = {
        "watcher_seen",
        "portal_whisper",
        "monument_activate"
    };
    
    for (int i = 0; i < 3; i++) {
        if (FileExists(effectPaths[i])) {
            Sound effect = LoadSound(effectPaths[i]);
            if (effect.frameCount > 0) {
                m_effects[effectNames[i]] = effect;
                TraceLog(LOG_INFO, "  ✓ Loaded sound effect: %s", effectNames[i]);
            }
        }
    }
}

void AudioManager::Update(float tension, float deltaTime) {
    if (!m_initialized) return;
    
    for (auto& pair : m_layers) {
        if (pair.second.loaded) {
            UpdateMusicStream(pair.second.music);
        }
    }
    
    if (tension < 15.0f) {
        UpdateLayer("base", 0.6f, deltaTime);
        UpdateLayer("whispers", 0.0f, deltaTime);
        UpdateLayer("drone", 0.0f, deltaTime);
        UpdateLayer("distortion", 0.0f, deltaTime);
        UpdateLayer("void", 0.0f, deltaTime);
    }
    else if (tension < 30.0f) {
        float t = (tension - 15.0f) / 15.0f;
        UpdateLayer("base", 0.6f * (1.0f - t), deltaTime);
        UpdateLayer("whispers", 0.5f * t, deltaTime);
        UpdateLayer("drone", 0.0f, deltaTime);
        UpdateLayer("distortion", 0.0f, deltaTime);
        UpdateLayer("void", 0.0f, deltaTime);
    }
    else if (tension < 50.0f) {
        float t = (tension - 30.0f) / 20.0f;
        UpdateLayer("base", 0.0f, deltaTime);
        UpdateLayer("whispers", 0.5f * (1.0f - t), deltaTime);
        UpdateLayer("drone", 0.6f * t, deltaTime);
        UpdateLayer("distortion", 0.0f, deltaTime);
        UpdateLayer("void", 0.0f, deltaTime);
    }
    else if (tension < 70.0f) {
        float t = (tension - 50.0f) / 20.0f;
        UpdateLayer("base", 0.0f, deltaTime);
        UpdateLayer("whispers", 0.0f, deltaTime);
        UpdateLayer("drone", 0.6f * (1.0f - t), deltaTime);
        UpdateLayer("distortion", 0.7f * t, deltaTime);
        UpdateLayer("void", 0.0f, deltaTime);
    }
    else {
        float t = (tension - 70.0f) / 30.0f;
        if (t > 1.0f) t = 1.0f;
        UpdateLayer("base", 0.0f, deltaTime);
        UpdateLayer("whispers", 0.0f, deltaTime);
        UpdateLayer("drone", 0.0f, deltaTime);
        UpdateLayer("distortion", 0.7f * (1.0f - t), deltaTime);
        UpdateLayer("void", 0.8f * t, deltaTime);
    }
    
    m_lastTensionLevel = tension;
}

void AudioManager::UpdateLayer(const std::string& name, float targetVol, float deltaTime) {
    if (m_layers.find(name) == m_layers.end()) return;
    
    AudioLayer& layer = m_layers[name];
    if (!layer.loaded) return;
    
    layer.targetVolume = targetVol;
    
    if (layer.currentVolume < layer.targetVolume) {
        layer.currentVolume += layer.fadeSpeed * deltaTime;
        if (layer.currentVolume > layer.targetVolume)
            layer.currentVolume = layer.targetVolume;
    }
    else if (layer.currentVolume > layer.targetVolume) {
        layer.currentVolume -= layer.fadeSpeed * deltaTime;
        if (layer.currentVolume < layer.targetVolume)
            layer.currentVolume = layer.targetVolume;
    }
    
    SetMusicVolume(layer.music, layer.currentVolume * m_masterVolume);
}

void AudioManager::PlayOneShotEffect(const std::string& name) {
    if (!m_initialized) return;
    
    if (m_effects.find(name) != m_effects.end()) {
        PlaySound(m_effects[name]);
        TraceLog(LOG_INFO, "Playing sound effect: %s", name.c_str());
    }
}

void AudioManager::SetMasterVolume(float volume) {
    m_masterVolume = volume;
    if (m_masterVolume < 0.0f) m_masterVolume = 0.0f;
    if (m_masterVolume > 1.0f) m_masterVolume = 1.0f;
}

void AudioManager::Cleanup() {
    if (!m_initialized) return;
    
    TraceLog(LOG_INFO, "Cleaning up Audio Manager...");
    
    for (auto& pair : m_layers) {
        if (pair.second.loaded) {
            StopMusicStream(pair.second.music);
            UnloadMusicStream(pair.second.music);
        }
    }
    
    for (auto& pair : m_effects) {
        UnloadSound(pair.second);
    }
    
    m_layers.clear();
    m_effects.clear();
    
    CloseAudioDevice();
    m_initialized = false;
    
    TraceLog(LOG_INFO, "✓ Audio Manager cleaned up");
}