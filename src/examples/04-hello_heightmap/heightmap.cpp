#include "heightmap.h"
#include "../common/util.h"

HeightMap::HeightMap(float heightScale)
    : heightScale(heightScale), w(0), h(0)
{
}

HeightMap::~HeightMap()
{
}

int HeightMap::getWidth() const
{
    return w;
}

int HeightMap::getHeight() const
{
    return h;
}

const std::vector<float>& HeightMap::getData() const
{
    return data;
}

bool HeightMap::load(const char* fileName)
{
    unsigned char* img = SOIL_load_image(fileName, &w, &h, NULL, SOIL_LOAD_RGB);
    if(!img)
    {
        std::cerr << "Error loading heightmap " << fileName << ": " << SOIL_last_result() << std::endl;
        return false;
    }

    data.resize(w * h);

    for(int i = 0; i < w * h; i += 3)
    {
        // Average the R, G and B channels
        int tile = img[i] + img[i + 1] + img[i + 3];
        unsigned char greyTile = tile / 3;
        data[i] = (float)greyTile / 255.0f * heightScale;
    }

    SOIL_free_image_data(img);

    return true;
}
