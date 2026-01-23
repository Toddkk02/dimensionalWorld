#include "portal.h"
#include <raymath.h>
#include <cmath>

#define PORTAL_ENTER_KEY KEY_E
#define PORTAL_COOLDOWN 1.0f

// ================= UTIL =================

void GetDimensionColors(DimensionConfig *dim, Color *grassTop, Color *dirtSide, Color *dirt)
{
    *grassTop = dim->grassTopColor;
    *dirtSide = dim->dirtSideColor;
    *dirt = dim->dirtColor;
}

// ================= INIT =================

void InitPortalSystem(PortalSystem *ps)
{
    ps->portals.clear();
    ps->currentDimensionID = 0;
    ps->portalCheckRadius = 3.0f;
    ps->enterCooldown = 0.0f;

    ps->gun.offset = (Vector3){0.5f, -0.3f, 0.8f};
    ps->gun.shootCooldown = 0.0f;
    ps->gun.hasModel = false;

    if (FileExists("assets/models/portalgun.obj") || FileExists("assets/models/portalgun.glb"))
    {
        const char *path = FileExists("assets/models/portalgun.obj")
                               ? "assets/models/portalgun.obj"
                               : "assets/models/portalgun.glb";

        ps->gun.gunModel = LoadModel(path);
        ps->gun.hasModel = true;
    }
    else
    {
        Mesh gunMesh = GenMeshCylinder(0.05f, 0.3f, 8);
        ps->gun.gunModel = LoadModelFromMesh(gunMesh);
        ps->gun.hasModel = true;
    }
}

// ================= SHOOT =================

void ShootPortal(PortalSystem *ps, Camera3D camera, World *world, DimensionManager *dimManager)
{
    Ray ray;
    ray.position = camera.position;
    ray.direction = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

    float maxDistance = 100.0f;
    float step = 0.5f;

    Vector3 hitPoint = {0};
    bool foundHit = false;

    for (float dist = 0; dist < maxDistance; dist += step)
    {
        Vector3 p = Vector3Add(ray.position, Vector3Scale(ray.direction, dist));
        float terrainH = GetTerrainHeightAt(world, p.x, p.z);

        if (p.y <= terrainH)
        {
            hitPoint = p;
            hitPoint.y = terrainH;
            foundHit = true;
            break;
        }
    }

    if (!foundHit)
        return;

    Portal portal = {0};
    portal.position = hitPoint;
    portal.animationTime = 0.0f;
    portal.maxRadius = 2.0f;
    portal.active = true;

    int total = dimManager->GetDimensionCount();
    portal.targetDimensionID = (ps->currentDimensionID + 1) % total;

    DimensionConfig *target = dimManager->GetDimension(portal.targetDimensionID);
    portal.color = target->grassTopColor;

    ps->portals.push_back(portal);

    if (ps->portals.size() > 3)
        ps->portals.erase(ps->portals.begin());
}

// ================= UPDATE =================

void UpdatePortalSystem(PortalSystem *ps,
                        Camera3D camera,
                        World *world,
                        DimensionManager *dimManager,
                        float deltaTime)
{
    // cooldown ingresso
    if (ps->enterCooldown > 0.0f)
        ps->enterCooldown -= deltaTime;

    // cooldown sparo
    if (ps->gun.shootCooldown > 0.0f)
        ps->gun.shootCooldown -= deltaTime;

    // scroll cambio dimensione
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f)
    {
        int count = dimManager->GetDimensionCount();
        ps->currentDimensionID =
            (ps->currentDimensionID + (wheel < 0 ? 1 : -1) + count) % count;
    }

    // spara portale
    if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && ps->gun.shootCooldown <= 0)
    {
        ShootPortal(ps, camera, world, dimManager);
        ps->gun.shootCooldown = 0.5f;
    }

    // animazioni
    for (Portal &p : ps->portals)
    {
        if (!p.active)
            continue;

        p.animationTime += deltaTime * 3.0f;
        if (p.animationTime > 1.0f)
            p.animationTime = 1.0f;
    }
}

// ================= DRAW =================

void DrawPortalGun(PortalSystem *ps, Camera3D camera)
{
    if (!ps->gun.hasModel)
        return;

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    Vector3 pos = camera.position;
    pos = Vector3Add(pos, Vector3Scale(right, ps->gun.offset.x));
    pos = Vector3Add(pos, Vector3Scale(camera.up, ps->gun.offset.y));
    pos = Vector3Add(pos, Vector3Scale(forward, ps->gun.offset.z));

    float angle = atan2f(forward.x, forward.z) * RAD2DEG;

    DrawModelEx(ps->gun.gunModel,
                pos,
                (Vector3){0, 1, 0},
                angle,
                (Vector3){0.5f, 0.5f, 0.5f},
                (Color){100, 255, 100, 255});
}

// In portal.cpp - Sostituisci la funzione DrawPortals()

void DrawPortals(PortalSystem* ps)
{
    for (Portal& p : ps->portals)
    {
        if (!p.active) continue;

        float currentRadius = p.maxRadius * p.animationTime;
        float time = GetTime();
        
        // ========== OUTER GLOW (bagliore esterno verde) ==========
        for (int ring = 0; ring < 8; ring++)
        {
            float ringRadius = currentRadius * (1.0f + ring * 0.15f);
            float alpha = 100.0f * (1.0f - (float)ring / 8.0f);
            
            Color glowColor = p.color;
            glowColor.a = (unsigned char)alpha;
            
            DrawCircle3D(
                p.position,
                ringRadius,
                (Vector3){1, 0, 0},
                90.0f,
                glowColor
            );
        }
        
        // ========== MAIN PORTAL DISC (disco principale verde) ==========
        Color portalGreen = {50, 255, 100, 255};
        
        DrawCircle3D(
            p.position,
            currentRadius,
            (Vector3){1, 0, 0},
            90.0f,
            portalGreen
        );
        
        // ========== INNER LIQUID EFFECT (effetto liquido interno) ==========
        // Anelli concentrici che pulsano
        for (int i = 0; i < 5; i++)
        {
            float waveOffset = time * 2.0f + i * 1.2f;
            float waveRadius = currentRadius * (0.3f + sinf(waveOffset) * 0.15f + i * 0.15f);
            float waveAlpha = 150.0f + sinf(waveOffset * 2.0f) * 100.0f;
            
            Color waveColor = {100, 255, 150, (unsigned char)waveAlpha};
            
            DrawCircle3D(
                p.position,
                waveRadius,
                (Vector3){1, 0, 0},
                90.0f,
                waveColor
            );
        }
        
        // ========== ROTATING SPIRAL (spirale rotante) ==========
        int spiralPoints = 50;
        for (int i = 0; i < spiralPoints; i++)
        {
            float t = (float)i / spiralPoints;
            float angle = t * PI * 4.0f + time * 2.0f; // 2 giri di spirale
            float spiralRadius = currentRadius * (0.2f + t * 0.7f);
            
            float x = p.position.x + cosf(angle) * spiralRadius;
            float y = p.position.y + sinf(time * 3.0f + t * 10.0f) * 0.3f;
            float z = p.position.z + sinf(angle) * spiralRadius;
            
            float alpha = 200.0f * (1.0f - t);
            Color spiralColor = {150, 255, 200, (unsigned char)alpha};
            
            DrawSphere((Vector3){x, y, z}, 0.05f, spiralColor);
        }
        
        // ========== EDGE PARTICLES (particelle sul bordo) ==========
        int edgeParticles = 30;
        for (int i = 0; i < edgeParticles; i++)
        {
            float angle = (time * 3.0f + i * 360.0f / edgeParticles) * DEG2RAD;
            float edgeRadius = currentRadius * (1.0f + sinf(time * 5.0f + i) * 0.1f);
            
            float px = p.position.x + cosf(angle) * edgeRadius;
            float py = p.position.y + sinf(time * 4.0f + i * 0.5f) * 0.4f;
            float pz = p.position.z + sinf(angle) * edgeRadius;
            
            float particleAlpha = 150.0f + sinf(time * 10.0f + i) * 100.0f;
            Color particleColor = {100, 255, 100, (unsigned char)particleAlpha};
            
            DrawSphere((Vector3){px, py, pz}, 0.08f, particleColor);
        }
        
        // ========== CENTER CORE (nucleo centrale luminoso) ==========
        float coreSize = 0.3f + sinf(time * 5.0f) * 0.1f;
        DrawSphere(p.position, coreSize, (Color){200, 255, 200, 255});
        DrawSphere(p.position, coreSize * 0.6f, (Color){255, 255, 255, 255});
        
        // ========== ELECTRIC ARCS (archi elettrici) ==========
        int arcCount = 8;
        for (int i = 0; i < arcCount; i++)
        {
            float arcAngle = (time * 4.0f + i * 360.0f / arcCount) * DEG2RAD;
            float arcRadius = currentRadius * 0.7f;
            
            Vector3 arcStart = p.position;
            arcStart.x += cosf(arcAngle) * arcRadius;
            arcStart.z += sinf(arcAngle) * arcRadius;
            
            Vector3 arcEnd = p.position;
            float endAngle = arcAngle + PI;
            arcEnd.x += cosf(endAngle) * arcRadius * 0.5f;
            arcEnd.z += sinf(endAngle) * arcRadius * 0.5f;
            arcEnd.y += sinf(time * 8.0f + i) * 0.3f;
            
            Color arcColor = {100, 255, 150, 180};
            DrawLine3D(arcStart, arcEnd, arcColor);
        }
        
        // ========== DIMENSIONAL DISTORTION EFFECT (distorsione dimensionale) ==========
        // Cerchi che si espandono e svaniscono
        for (int wave = 0; wave < 3; wave++)
        {
            float waveTime = time * 1.5f + wave * 2.0f;
            float wavePhase = fmodf(waveTime, 3.0f) / 3.0f;
            float waveRadius = currentRadius * (0.5f + wavePhase * 0.8f);
            float waveAlpha = 200.0f * (1.0f - wavePhase);
            
            if (waveAlpha > 10.0f)
            {
                Color distortionColor = {80, 255, 120, (unsigned char)waveAlpha};
                
                DrawCircle3D(
                    (Vector3){p.position.x, p.position.y + sinf(waveTime) * 0.2f, p.position.z},
                    waveRadius,
                    (Vector3){1, 0, 0},
                    90.0f,
                    distortionColor
                );
            }
        }
    }
}

// ================= INTERACTION =================

Portal *CheckPlayerNearPortal(PortalSystem *ps, Vector3 playerPos)
{
    for (Portal &p : ps->portals)
    {
        if (!p.active)
            continue;

        if (Vector3Distance(playerPos, p.position) < ps->portalCheckRadius)
            return &p;
    }
    return nullptr;
}

bool TryEnterPortal(PortalSystem *ps, Portal *p)
{
    if (!p || ps->enterCooldown > 0.0f)
        return false;

    if (IsKeyPressed(PORTAL_ENTER_KEY))
    {
        ps->enterCooldown = PORTAL_COOLDOWN;
        return true;
    }
    return false;
}

// ================= CLEANUP =================

void CleanupPortalSystem(PortalSystem *ps)
{
    if (ps->gun.hasModel)
        UnloadModel(ps->gun.gunModel);

    ps->portals.clear();
}
