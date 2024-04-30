//==============================================================================
// Minamoto : Camera Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <algorithm>
#include <utility/xxCamera.h>
#include "Camera.h"

//------------------------------------------------------------------------------
Camera::Camera()
{
    arcball = {0.85f, -M_PI_2, 14.0f};
    direction = {1.0f, 0.0f, 0.0f};
    MoveArcball(0, 0);
}
//------------------------------------------------------------------------------
Camera::~Camera()
{
}
//------------------------------------------------------------------------------
void Camera::SetFOV(float aspect, float fov, float far_value)
{
    camera->SetFOV(aspect, fov, far_value);
}
//------------------------------------------------------------------------------
xxCameraPtr const& Camera::GetCamera() const
{
    return camera;
}
//------------------------------------------------------------------------------
void Camera::SetDistance(float distance)
{
    arcball.z = distance;
    MoveArcball(0, 0);
}
//------------------------------------------------------------------------------
float Camera::GetDistance() const
{
    return arcball.z;
}
//------------------------------------------------------------------------------
xxVector3 const& Camera::GetDirection()
{
    return camera->Direction;
}
//------------------------------------------------------------------------------
void Camera::MoveArcball(float x, float y)
{
    arcball.x = std::clamp<float>(arcball.x + y, -M_PI_2 * 0.8f, M_PI_2 * 0.8f);
    arcball.y = std::fmodf(arcball.y - x + M_PI, M_PI * 2.0f) - M_PI;
    arcball.z = std::clamp<float>(arcball.z, 5.0f, 50.0f);

    float xsin = std::sinf(arcball.x);
    float xcos = std::cosf(arcball.x);
    float ysin = std::sinf(arcball.y);
    float ycos = std::cosf(arcball.y);
    camera_offset.x = xcos * arcball.z * ycos;
    camera_offset.y = xcos * arcball.z * ysin;
    camera_offset.z = xsin * arcball.z;
}
//------------------------------------------------------------------------------
xxVector3 const& Camera::GetArcball() const
{
    return arcball;
}
//------------------------------------------------------------------------------
void Camera::Update(float elapsed, float forward_backward, float left_right, float x, float y)
{
    bool update = false;
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
    if (update)
    {
        camera->Update();
    }
}
//------------------------------------------------------------------------------
Camera* Camera::CreateCamera()
{
    Camera* pointer = new Camera;
    if (pointer == nullptr)
        return nullptr;

    pointer->camera = xxCamera::Create();
    pointer->SetFOV(16.0f / 9.0f, 60.0f, 10000.0f);

    return pointer;
}
//------------------------------------------------------------------------------
void Camera::DestroyCamera(Camera* camera)
{
    delete camera;
}
//------------------------------------------------------------------------------
