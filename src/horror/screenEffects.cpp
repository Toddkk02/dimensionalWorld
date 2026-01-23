#include "screenEffects.h"
#include <cmath>
#include <stdlib.h>

ScreenEffects::ScreenEffects() : 
    m_shakeIntensity(0), 
    m_shakeTime(0),
    m_initialized(false),
    m_screenWidth(0),
    m_screenHeight(0) {
}

ScreenEffects::~ScreenEffects() {
    if (m_initialized) {
        Cleanup();
    }
}

ScreenEffects& ScreenEffects::Get() {
    static ScreenEffects instance;
    return instance;
}

void ScreenEffects::Init(int screenWidth, int screenHeight) {
    if (m_initialized) {
        TraceLog(LOG_WARNING, "ScreenEffects already initialized");
        return;
    }
    
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    
    m_screenTarget = LoadRenderTexture(screenWidth, screenHeight);
    
    #if defined(PLATFORM_DESKTOP)
        #define GLSL_VERSION 330
    #else
        #define GLSL_VERSION 100
    #endif
    
    if (FileExists(TextFormat("assets/shaders/glsl%i/chromatic.fs", GLSL_VERSION))) {
        m_chromaticShader = LoadShader(
            TextFormat("assets/shaders/glsl%i/base.vs", GLSL_VERSION),
            TextFormat("assets/shaders/glsl%i/chromatic.fs", GLSL_VERSION)
        );
        TraceLog(LOG_INFO, "  ✓ Loaded chromatic aberration shader");
    } else {
        m_chromaticShader.id = 0;
        TraceLog(LOG_WARNING, "  ✗ Chromatic shader not found - using default");
    }
    
    m_initialized = true;
    TraceLog(LOG_INFO, "✓ Screen Effects initialized (%dx%d)", screenWidth, screenHeight);
}

void ScreenEffects::BeginEffects() {
    if (!m_initialized) return;
    BeginTextureMode(m_screenTarget);
}

void ScreenEffects::EndEffects(float tension) {
    if (!m_initialized) {
        return;
    }
    
    EndTextureMode();
    
    BeginDrawing();
    
    float chromaticAmount = 0.0f;
    float vignetteStrength = 0.0f;
    
    if (tension > 30.0f) {
        chromaticAmount = (tension - 30.0f) / 70.0f * 0.003f;
    }
    
    if (tension > 40.0f) {
        vignetteStrength = (tension - 40.0f) / 60.0f * 0.6f;
    }
    
    Vector2 shakeOffset = {0, 0};
    if (m_shakeIntensity > 0) {
        m_shakeTime += GetFrameTime() * 50.0f;
        shakeOffset.x = sinf(m_shakeTime) * m_shakeIntensity;
        shakeOffset.y = cosf(m_shakeTime * 1.3f) * m_shakeIntensity;
        m_shakeIntensity *= 0.95f;
    }
    
    if (chromaticAmount > 0.001f && m_chromaticShader.id > 0) {
        BeginShaderMode(m_chromaticShader);
        int loc = GetShaderLocation(m_chromaticShader, "aberration");
        SetShaderValue(m_chromaticShader, loc, &chromaticAmount, SHADER_UNIFORM_FLOAT);
    }
    
    Rectangle source = {
        0, 0,
        (float)m_screenTarget.texture.width,
        -(float)m_screenTarget.texture.height
    };
    
    Rectangle dest = {
        shakeOffset.x,
        shakeOffset.y,
        (float)m_screenWidth,
        (float)m_screenHeight
    };
    
    DrawTexturePro(
        m_screenTarget.texture,
        source,
        dest,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
    
    if (chromaticAmount > 0.001f && m_chromaticShader.id > 0) {
        EndShaderMode();
    }
    
    if (vignetteStrength > 0.05f) {
        DrawVignette(vignetteStrength);
    }
    
    EndDrawing();
}

void ScreenEffects::DrawVignette(float strength) {
    int steps = 20;
    for (int i = 0; i < steps; i++) {
        float t = (float)i / steps;
        float alpha = strength * (1.0f - t);
        Color c = Fade(BLACK, alpha);
        
        float inset = t * m_screenWidth / 2;
        
        DrawRectangleLinesEx(
            (Rectangle){
                inset,
                inset * (m_screenHeight / (float)m_screenWidth),
                m_screenWidth - 2 * inset,
                m_screenHeight - 2 * inset * (m_screenHeight / (float)m_screenWidth)
            },
            m_screenWidth * 0.05f,
            c
        );
    }
}

void ScreenEffects::SetScreenShake(float intensity) {
    m_shakeIntensity = intensity;
}

void ScreenEffects::Cleanup() {
    if (!m_initialized) return;
    
    UnloadRenderTexture(m_screenTarget);
    
    if (m_chromaticShader.id > 0) {
        UnloadShader(m_chromaticShader);
    }
    
    m_initialized = false;
    TraceLog(LOG_INFO, "✓ Screen Effects cleaned up");
}