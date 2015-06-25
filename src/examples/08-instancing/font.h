#ifndef FONT_HEADER
#define FONT_HEADER

#include "../common/util.h"

class Font
{
public:
    Font();
    ~Font();

    bool load(const char* fileName, int numChars);
    bool bind();
    int getGlyphWidth() const;
    int getGlyphHeight() const;
    int getTotalWidth() const;
private:
    GLuint texture;
    int glyphWidth, glyphHeight, totalWidth;
};

#endif
