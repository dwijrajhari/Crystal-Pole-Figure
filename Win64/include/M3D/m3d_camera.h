#ifndef M3D_CAMERA
#define M3D_CAMERA

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLFWwindow;

class M3DCamera
{
public:
    //Camera properties
    GLFWwindow* mWindow = nullptr;
    glm::vec3 mPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 mDir = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 mUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float mMoveSpeed = 0.05f, mRotateSpeed = 0.5f;
    float mYaw = -90.0f, mPitch = 0.0f;

    M3DCamera();
    glm::mat4 fGetView();
    void fHandleRotationFPS(float, float);
    void fHandleRotationTurntable(float, float);
    void fHandleRotationArcball(float, float, float, float);

};

#endif // !M3D_CAMERA

