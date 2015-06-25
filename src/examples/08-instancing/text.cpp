#include "text.h"
#include "font.h"
#include "../common/shader.h"

#define MAX_STRING_LENGTH 10 // Maximum # characters

const char* VERTEX_SRC = "#version 330 core\n"
                         "layout(location=0) in vec2 position;"
                         "uniform int glyphWidth;" // Width of 1 glyph (fixed-width font!)
                         "uniform int fontWidth;"  // Width of the font file
                         "uniform vec2 texCoords[120];"
                         "out vec2 fTexcoord;"
                         "void main()"
                         "{"
                         "    fTexcoord = texCoords[gl_InstanceID * gl_VertexID];"
                         "    float newX = position.x + gl_InstanceID * glyphWidth;"
                         "    gl_Position = vec4(newX, position.y, 0.0, 1.0);"
                         "}";

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec2 fTexcoord;"
                           "out vec4 outputColor;"
                           "uniform sampler2D fontTexture;"
                           "void main()"
                           "{"
                           "    outputColor = texture(fontTexture, fTexcoord);"
                           "}";

Text::Text()
    : font(NULL), x(0.0f), y(0.0f), vao(0), program(0)
{
}

Text::~Text()
{
    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
    }
    if(program)
    {
        glDeleteProgram(program);
    }
}

bool Text::load(Font* fnt)
{
    font = fnt;

    // Shader
    GLuint vertex = createShader(VERTEX_SRC, GL_VERTEX_SHADER);
    GLuint fragment = createShader(FRAGMENT_SRC, GL_FRAGMENT_SHADER);
    program = createShaderProgram(vertex, fragment);
    linkShader(program);
    validateShader(program);

    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    glDetachShader(program, fragment);
    glDeleteShader(fragment);

    // Geometry
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    // We create vertices to fit one glyph
    float vertices[] =
    {
        // x   y
        -0.5f, 0.5f,
        0.5f,  0.5f,
        0.5f, -0.5f,
        0.5f, -0.5f,
        -0.5f, -0.5f,
        -0.5f, 0.5f
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

    glBindVertexArray(0);

    glDeleteBuffers(1, &vbo);

    return true;
}

void Text::setPosition(float x_, float y_)
{
    x = x_;
    y = y_;
}

void Text::setString(const char* str)
{
    string = str;
}

static void setTexCoord(int i, float x, float y, GLuint program)
{
    glUniform2f(glGetUniformLocation(program, ("texcoords[" + std::to_string(i) + "]").c_str()), x, y);
}

void Text::render()
{
    if(!program || !vao || !font)
    {
        std::cerr << "Tried to render text with invalid program, VAO or font" << std::endl;
        return;
    }
    if(string.length() > MAX_STRING_LENGTH)
    {
        std::cerr << "String is too long!" << std::endl;
        return;
    }

    glUseProgram(program);
    glBindVertexArray(vao);

    // Calculate the texture coordinates for each character
    // Normally, the font would store this information based on a file
    // that was distributed with the font.
    // For this example, I have made sure that the indices for the characters
    // match the ASCII index
    int vertIndex = -1;
    for(int i = 0; i < string.length(); ++i)
    {
        int index = string.at(i) - 'A';
        float relativeWidth = font->getGlyphWidth() / (float)font->getTotalWidth();
        // bottom left
        setTexCoord(++vertIndex, (float)index * relativeWidth, 1.0f, program);
        // bottom right
        setTexCoord(++vertIndex, (float)index * relativeWidth + relativeWidth, 1.0f, program);
        // top right
        setTexCoord(++vertIndex, (float)index * relativeWidth + relativeWidth, 0.0f, program);
        // top right
        setTexCoord(++vertIndex, (float)index * relativeWidth + relativeWidth, 0.0f, program);
        // top left
        setTexCoord(++vertIndex, (float)index * relativeWidth, 0.0f, program);
        // bottom left
        setTexCoord(++vertIndex, (float)index * relativeWidth, 1.0f, program);
    }

    font->bind();
    glUniform1i(glGetUniformLocation(program, "glyphWidth"), font->getGlyphWidth());
    glUniform1i(glGetUniformLocation(program, "fontTexture"), 0);
    // Draw TRIANGLES, starting at index 0. There are 6 vertices per glyph, and there are string.length() glyphs
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, string.length());

    glBindVertexArray(0);
    glUseProgram(0);
}
