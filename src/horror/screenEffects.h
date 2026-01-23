#pragma once

#include "raylib.h"

class ScreenEffects {
public:
    static ScreenEffects& Get();
    
    void Init(int screenWidth, int screenHeight);
    void BeginEffects();
    void EndEffects(float tension);
    void Cleanup();
    
    void SetScreenShake(float intensity);
    bool IsInitialized() const { return m_initialized; }

private:
    ScreenEffects();
    ~ScreenEffects();
    
    ScreenEffects(const ScreenEffects&) = delete;
    ScreenEffects& operator=(const ScreenEffects&) = delete;
    
    RenderTexture2D m_screenTarget;
    Shader m_chromaticShader;
    
    float m_shakeIntensity;
    float m_shakeTime;
    
    int m_screenWidth;
    int m_screenHeight;
    
    bool m_initialized;
    
    void DrawVignette(float strength);
};