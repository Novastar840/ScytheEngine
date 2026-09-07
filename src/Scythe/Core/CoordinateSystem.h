#pragma once

// ==========================================
// Coordinate System Convention
// ==========================================
// X-forward, Y-up, Z-right

namespace Scythe 
{
    constexpr Vec3 ForwardVector = Vec3(1.0f, 0.0f, 0.0f);
    constexpr Vec3 BackVector    = Vec3(-1.0f, 0.0f, 0.0f);
    constexpr Vec3 RightVector   = Vec3(0.0f, 0.0f, 1.0f);
    constexpr Vec3 LeftVector    = Vec3(0.0f, 0.0f, -1.0f);
    constexpr Vec3 UpVector      = Vec3(0.0f, 1.0f, 0.0f);
    constexpr Vec3 DownVector    = Vec3(0.0f, -1.0f, 0.0f);
}
