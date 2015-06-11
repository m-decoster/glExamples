#include "util.h"

void error_callback(int error, const char* description)
{
    std::cerr << description << std::endl;
}
