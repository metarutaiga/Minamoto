//==============================================================================
// Minamoto : Camera Header
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <utility/xxCamera.h>

class Camera
{
    xxCameraPtr camera;

    xxVector3 arcball;
    xxVector3 camera_offset;
    xxVector3 direction;

    Camera();
    ~Camera();

public:
    void SetFOV(float aspect, float fov, float far_value);

    xxCameraPtr GetCamera() const;

    void SetDistance(float distance);
    float GetDistance() const;
    xxVector3 const& GetDirection();

    void MoveArcball(float x, float y);
    xxVector3 const& GetArcball() const;

    void Update(float elapsed, float forward_backward, float left_right, float x, float y);

    static Camera* CreateCamera();
    static void DestroyCamera(Camera* control);
};
