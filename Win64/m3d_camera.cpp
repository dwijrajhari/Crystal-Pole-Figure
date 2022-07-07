#include <M3D/m3d_camera.h>
#include <iostream>
#define min(A, B) (A<=B?A:B)

M3DCamera::M3DCamera() { }

glm::mat4 M3DCamera::fGetView()
{
	return glm::lookAt(mPos, mDir + mPos, mUp);
}

void M3DCamera::fHandleRotationFPS(float dx, float dy)
{
    mYaw += dx;
    mPitch -= dy;

    if      (mPitch >  89.9f)  mPitch =  89.9f;
    else if (mPitch < -89.9f)  mPitch = -89.9f;

    if      (mYaw >  180.0f)   mYaw -= 360.0f;
    else if (mYaw < -180.0f)   mYaw += 360.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    direction.y = sin(glm::radians(mPitch));
    direction.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mDir = glm::normalize(direction);
}

void M3DCamera::fHandleRotationTurntable(float dx, float dy)
{
    // Update yaw and pitch values
    mYaw   += dx;
    mPitch -= dy;

    // Restrict pitch to (-90, 90)
    if      (mPitch >  89.9f)  mPitch =  89.9f;
    else if (mPitch < -89.9f)  mPitch = -89.9f;

    // Restrict yaw to (-180, 180)
    if      (mYaw >  180.0f)   mYaw -= 360.0f;
    else if (mYaw < -180.0f)   mYaw += 360.0f;

    // Calculate new direction
    glm::vec3 direction;
    direction.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    direction.y = sin(glm::radians(mPitch));
    direction.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mDir = glm::normalize(direction);

    // Position = -Direction
    mPos = -mDir * glm::length(mPos);
}

void M3DCamera::fHandleRotationArcball(float x, float y, float xp, float yp)
{
    float z  = x * x + y * y;
    float zp = xp * xp + yp * yp;

    z  = z  <= 1 ? glm::sqrt(1 - z ) : 0;
    zp = zp <= 1 ? glm::sqrt(1 - zp) : 0;

    glm::vec3 ui = glm::normalize(glm::vec3({ x,  -y,  z }));
    glm::vec3 uf = glm::normalize(glm::vec3({ xp, -yp, zp }));

    float theta = glm::acos(min(1, glm::dot(ui, uf)));
    glm::vec3 u = glm::cross(ui, uf);
    std::cout << ui.x <<" " << ui.y << " " << ui.z << '\n';
    glm::mat4 rotmat = glm::rotate(glm::mat4(1.0f), -theta, u);
    mPos = rotmat * glm::vec4(mPos, 0.0f);
    mUp = rotmat * glm::vec4(mUp, 0.0f);
    mDir = -glm::normalize(mPos);
}

