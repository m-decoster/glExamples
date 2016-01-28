#include "util.h"

// Don't actually draw quads like this
void drawQuad(GLuint program, int* boundingBox, int width, int height)
{
    static bool firstRun = true;
    static GLuint vao = 0;
    if(firstRun)
    {
        glGenVertexArrays(1, &vao);
    }
    glBindVertexArray(vao);
    static GLuint vbo = 0;
    if(firstRun)
    {
        glGenBuffers(1, &vbo);
    }
    // Oh boy
    float vertices[] = {
        (float)boundingBox[0] / (width / 2) - 1, (float)boundingBox[1] / (height / 2) - 1, // top left
        (float)boundingBox[2] / (width / 2) - 1, (float)boundingBox[1] / (height / 2) - 1, // top right
        (float)boundingBox[0] / (width / 2) - 1, (float)boundingBox[3] / (height / 2) - 1, // bottom left
        (float)boundingBox[0] / (width / 2) - 1, (float)boundingBox[3] / (height / 2) - 1, // bottom left
        (float)boundingBox[2] / (width / 2) - 1, (float)boundingBox[1] / (height / 2) - 1, // top right
        (float)boundingBox[2] / (width / 2) - 1, (float)boundingBox[3] / (height / 2) - 1, // bottom right
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    if(firstRun)
    {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
    glBindVertexArray(0);
    firstRun = false;
}

// Courtesy of http://learnopengl.com/#!Advanced-Lighting/Deferred-Shading
// Calculate a light's radius based on its attenuation
float lightRadius(const PointLight& light)
{
    float maxComponent = std::fmaxf(std::fmaxf(light.color.x, light.color.y), light.color.z);
    float constant = light.attenuation.x;
    float linear = light.attenuation.y;
    float quadratic = light.attenuation.z;
    float radius = (-linear + std::sqrtf(linear * linear - 4.0f * quadratic * (constant - (256.0f / 2.0f) * maxComponent))) / (2.0f * quadratic);
    return radius;
}

// Calculate a light's 3D bounding box based on its radius and position
std::vector<glm::vec3> lightBB(const PointLight& light)
{
    std::vector<glm::vec3> result;
    result.reserve(8);
    float radius = lightRadius(light);
    float diameter = 2.0f * radius;

    // TOP
    glm::vec3 topLeftBack = light.position;
              topLeftBack.x -= radius;
              topLeftBack.y += radius;
              topLeftBack.z += radius;
    glm::vec3 topRightBack = topLeftBack;
              topRightBack.x += diameter;
    glm::vec3 topLeftFront = topLeftBack;
              topLeftFront.z -= diameter;
    glm::vec3 topRightFront = topLeftFront;
              topRightFront.x += diameter;
    // BOTTOM
    glm::vec3 bottomLeftBack = topLeftBack;
              bottomLeftBack.y -= diameter;
    glm::vec3 bottomRightBack = bottomLeftBack;
              bottomRightBack.x += diameter;
    glm::vec3 bottomLeftFront = bottomLeftBack;
              bottomLeftFront.z -= diameter;
    glm::vec3 bottomRightFront = bottomLeftFront;
              bottomRightFront.x += diameter;
    
    result.push_back(topLeftBack);
    result.push_back(topRightBack);
    result.push_back(topLeftFront);
    result.push_back(topRightFront);

    result.push_back(bottomLeftBack);
    result.push_back(bottomRightBack);
    result.push_back(bottomLeftFront);
    result.push_back(bottomRightFront);

    return result;
}

// Calculate a light's bounding box in screen space based on its bounding box in world space
void lightBBScreen(const PointLight &light, const glm::mat4& proj, const glm::mat4& view, int width, int height, int* result)
{
    std::vector<glm::vec3> worldBB = lightBB(light);
    int minX = width, maxX = 0, minY = height, maxY = 0;
    for(auto it = worldBB.begin(); it != worldBB.end(); ++it)
    {
        glm::vec4 v = glm::vec4(it->x, it->y, it->z, 1.0);
        v = proj * (view * v);
        if (v.w <= 0.0)
        {
            // We are very close to the light source...
            // Using the scissors here would generate artifacts.
            // Because we are close, we can just render the entire screen.
            minX = 0;
            minY = 0;
            maxX = width;
            maxY = height;
            break;
        }
        glm::vec3 norm_dev_coord_v = glm::vec3(v) / v.w;
        // [-1,1] and [-1,1] -> [0,width] and [0,height]
        int x = static_cast<int> (((norm_dev_coord_v.x + 1.0f) / 2.0f) * width);
        int y = static_cast<int> (((norm_dev_coord_v.y + 1.0f) / 2.0f) * height);
        if (x < minX && x >= 0)       minX = x;
        if (x > maxX && x <= width)   maxX = x;
        if (y < minY && y >= 0)       minY = y;
        if (y > maxY && y <= height)  maxY = y;
    }
    result[0] = minX;
    result[1] = minY;
    result[2] = maxX;
    result[3] = maxY;
}
