#ifndef CAMERA_HEADER
#define CAMERA_HEADER

#include <glm/glm.hpp>

enum CameraType
{
    CAMERA_PERSPECTIVE,
    CAMERA_ORTHOGONAL
};

class Camera
{
public:
    /**
     * Create a new camera.
     * @param fov The vertical field of view
     * @param zNear Cut-off for things near the camera
     * @param zFar Cut-off for things far from the camera
     * @param width Of the viewport
     * @param height Of the viewport
     */
    Camera(CameraType type, float fov, float zNear, float zFar, float width, float height);
    ~Camera();

    void setPosition(float x, float y, float z);
    void setFov(float fov);
    void setZnear(float znear);
    void setZfar(float zfar);

    void setHorizontalAngle(float angle);
    void setVerticalAngle(float angle);

    const glm::mat4& getView();
    const glm::mat4& getProjection();

    const glm::vec3& getPosition() const;
    float getHorizontalAngle() const;
    float getVerticalAngle() const;

    /**
     * The direction the camera is pointing in
     */
    glm::vec3 getDirectionVector() const;
    /**
     * Perpendicular to the direction
     */
    glm::vec3 getRightVector() const;
    glm::vec3 getUpVector() const;
private:
    CameraType type;
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 position;
    glm::vec2 angle;
    float width, height, zNear, zFar, fov;
};

#endif
