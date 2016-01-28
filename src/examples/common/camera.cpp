#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(CameraType type, float fov, float zNear, float zFar, float width, float height)
    : type(type), position(0.0f, 0.0f, 0.0f), width(width), height(height), zNear(zNear), zFar(zFar), fov(fov)
{
    angle = glm::vec2(0.0f, 0.0f);
    if(type == CAMERA_ORTHOGONAL)
    {
        projection = glm::ortho(0.0f, width, 0.0f, height);
    }
    else
    {
        projection = glm::perspective(glm::radians(fov), width / height, zNear, zFar);
    }
}

Camera::~Camera()
{
}

void Camera::setPosition(float x, float y, float z)
{
    position = glm::vec3(x, y, z);
}

void Camera::setFov(float fov)
{
    this->fov = fov;
}

void Camera::setZnear(float znear)
{
    zNear = znear;
}

void Camera::setZfar(float zfar)
{
    zFar = zfar;
}

void Camera::setHorizontalAngle(float angle)
{
    this->angle.x = angle;
}

void Camera::setVerticalAngle(float angle)
{
    this->angle.y = angle;
}

const glm::mat4& Camera::getView()
{
    // Recalculate the view matrix and return it
    glm::vec3 direction = getDirectionVector();
    glm::vec3 up = getUpVector();
    view = glm::lookAt(position, position + direction, up);
    return view;
}

const glm::mat4& Camera::getProjection()
{
    // Recalculate the projection matrix and return it
    if(type == CAMERA_ORTHOGONAL)
    {
        float left = position.x;
        float right = position.x + width;
        float bottom = position.y + height;
        float top = position.y;
        projection = glm::ortho(left, right, bottom, top, zNear, zFar);
    }
    else
    {
        projection = glm::perspective(glm::radians(fov), width / height, zNear, zFar);
    }
    return projection;
}

glm::vec3 Camera::getUpVector() const
{
    glm::vec3 direction = getDirectionVector();
    glm::vec3 right = getRightVector();
    // Right hand rule: right and direction are perpendicular to up
    return glm::cross(right, direction);
}

glm::vec3 Camera::getDirectionVector() const
{
    return glm::vec3(cos(angle.y) * sin(angle.x), sin(angle.y),
            cos(angle.y) * cos(angle.x));
}

glm::vec3 Camera::getRightVector() const
{
    float piHalf = 3.14 / 2.0f;
    return glm::vec3(sin(angle.x - piHalf), 0, cos(angle.x - piHalf));
}

const glm::vec3& Camera::getPosition() const
{
    return position;
}

float Camera::getHorizontalAngle() const
{
    return angle.x;
}

float Camera::getVerticalAngle() const
{
    return angle.y;
}
