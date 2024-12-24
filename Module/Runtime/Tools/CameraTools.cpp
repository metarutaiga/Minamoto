//==============================================================================
// Minamoto : CameraTools Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <algorithm>
#include <xxGraphicPlus/xxCamera.h>
#include "CameraTools.h"

//==============================================================================
void CameraTools::MoveArcball(xxVector3& offset, xxVector3& arcball, float x, float y)
{
    float PI = float(M_PI);
    float PI_2 = float(M_PI_2);

    arcball.x = std::clamp<float>(arcball.x + y, -PI_2 * 0.8f, PI_2 * 0.8f);
    arcball.y = std::fmodf(arcball.y - x + PI, PI * 2.0f) - PI;
    arcball.z = std::clamp<float>(arcball.z, 5.0f, 50.0f);

    float xsin = std::sinf(arcball.x);
    float xcos = std::cosf(arcball.x);
    float ysin = std::sinf(arcball.y);
    float ycos = std::cosf(arcball.y);
    offset.x = xcos * arcball.z * ycos;
    offset.y = xcos * arcball.z * ysin;
    offset.z = xsin * arcball.z;
}
//------------------------------------------------------------------------------
bool CameraTools::MoveCamera(xxCameraPtr const& camera, float elapsed, float forward_backward, float left_right, float up_down, float x, float y)
{
    bool update = false;
    if (up_down != 0)
    {
        camera->Location = (camera->Location + camera->Up * up_down * elapsed);
        update = true;
    }
    if (left_right != 0)
    {
        camera->Location = (camera->Location + camera->Right * left_right * elapsed);
        update = true;
    }
    if (forward_backward != 0)
    {
        camera->Location = (camera->Location + camera->Direction * forward_backward * elapsed);
        update = true;
    }
    if (x != 0 || y != 0)
    {
        xxVector3 pos = camera->Location + camera->Direction;

        if (x != 0)
        {
            pos = pos + camera->Right * x;
        }
        if (y != 0)
        {
            pos = pos - camera->Up * y;
        }

        camera->LookAt(pos, xxVector3::Z);
        update = true;
    }
    return update;
}
//------------------------------------------------------------------------------
void CameraTools::SetViewport(xxCameraPtr const& camera, float fromWidth, float fromHeight, float toX, float toY, float toWidth, float toHeight)
{
    float oldCenterX = fromWidth / 2.0f;
    float oldCenterY = fromHeight / 2.0f;
    float newCenterX = toX + toWidth / 2.0f;
    float newCenterY = toY + toHeight / 2.0f;
    float scaleX = toWidth / fromWidth;
    float scaleY = toHeight / fromHeight;
    float translateX = (newCenterX - oldCenterX) * 2.0f / fromWidth;
    float translateY = -(newCenterY - oldCenterY) * 2.0f / fromHeight;

    xxMatrix4 viewport;
    viewport.v[0] = {     scaleX,          0, 0, 0 };
    viewport.v[1] = {          0,     scaleY, 0, 0 };
    viewport.v[2] = {          0,          0, 1, 0 };
    viewport.v[3] = { translateX, translateY, 0, 1 };

    camera->ProjectionMatrix = viewport * camera->ProjectionMatrix;
    camera->ViewProjectionMatrix = camera->ProjectionMatrix * camera->ViewMatrix;
}
//------------------------------------------------------------------------------
xxVector3 CameraTools::GetDirectionFromScreenPos(xxCameraPtr const& camera, float x, float y)
{
    float right = camera->FrustumLeft + x * (camera->FrustumRight - camera->FrustumLeft);
    float up = camera->FrustumBottom + y * (camera->FrustumTop - camera->FrustumBottom);

    xxVector3 direction = camera->Direction + camera->Right * right + camera->Up * up;
    direction /= direction.Length();
    return direction;
}
//------------------------------------------------------------------------------
xxVector3 CameraTools::GetScreenPosToWorldPos(xxCameraPtr const& camera, xxVector3 const& point)
{
    return camera->Location + GetDirectionFromScreenPos(camera, point.x, point.y) * point.z;
}
//------------------------------------------------------------------------------
xxVector4 CameraTools::GetWorldPosToScreenPos(xxCameraPtr const& camera, xxVector3 const& point)
{
    xxVector4 screen = camera->ViewProjectionMatrix * xxVector4{point.x, point.y, point.z, 1.0f};
    screen.xyz /= std::fabs(screen.w);
    screen.x = screen.x *  0.5f + 0.5f;
    screen.y = screen.y * -0.5f + 0.5f;
    return screen;
}
//==============================================================================
