#ifndef TEXT_HEADER
#define TEXT_HEADER

#include "../common/util.h"
#include <string>

class Font;

class Text
{
public:
    Text();
    ~Text();

    bool load(Font* font);
    void setPosition(float x, float y);
    void setString(const char* str);
    void render();
private:
    Font* font;
    float x, y;
    GLuint vao;
    GLuint program;
    std::string string;
};

#endif
