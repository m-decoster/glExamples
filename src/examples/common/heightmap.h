#ifndef HEIGHTMAP_HEADER
#define HEIGHTMAP_HEADER

#include <vector>

class HeightMap
{
public:
    /**
     * Create a new HeightMap
     * @param heightScale The total height difference between a black and a white value
     */
    HeightMap(float heightScale);
    ~HeightMap();

    int getWidth() const;
    int getHeight() const;
    const std::vector<float>& getData() const;

    bool load(const char* fileName);
private:
    float heightScale;
    int w, h;
    std::vector<float> data;
};

#endif
