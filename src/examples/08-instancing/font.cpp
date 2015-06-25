#include "font.h"
#include "../common/util.h"

Font::Font()
    : texture(0), glyphWidth(0), glyphHeight(0)
{
}

Font::~Font()
{
    if(texture)
    {
        glDeleteTextures(1, &texture);
    }
}

bool Font::load(const char* fileName, int numChars)
{
    texture = loadImage(fileName, &totalWidth, &glyphHeight, 0, true);
    glyphWidth = totalWidth / numChars;
    return texture != 0;
}

bool Font::bind()
{
    if(!texture)
    {
        std::cerr << "Tried to bind a font that was not correctly loaded" << std::endl;
        return false;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    return true;
}

int Font::getGlyphWidth() const
{
    return glyphWidth;
}

int Font::getGlyphHeight() const
{
    return glyphHeight;
}

int Font::getTotalWidth() const
{
    return totalWidth;
}
