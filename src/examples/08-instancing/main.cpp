#include "../common/util.h"
#include "font.h"
#include "text.h"

/* NOTE:
 * This example uses font rendering to demonstrate how instancing
 * works. Instancing is mostly useful when you want to render a
 * very large quantity of objects. This example only renders
 * "HELLOWORLD" to the screen, which is hardly an appropriate
 * use case for instancing. In your own applications, you should
 * always consider if instancing is the correct way to go, or if
 * you should consider a dynamic buffer (which we will cover
 * in a later example).
 *
 * The text rendering in this example is very basic. It only renders
 * uppercase characters of fixed-width fonts. It is a contrived example
 * that allows to show how instancing works. I will add a better example
 * later, which can render TrueType fonts with any color and transformation.
 */

int main(void)
{
    GLFWwindow* window;

    window = init("Instancing", 640, 480);
    if(!window)
    {
        return -1;
    }

    // Enable transparency for font rendering!
    // If you render transparent objects, you should always think
    // about the drawing order
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    Font font;
    if(!font.load("font.png", 26))
    {
        return -1;
    }
    Text text;
    if(!text.load(&font))
    {
        return -1;
    }
    text.setPosition(0.0f, 0.0f);
    text.setString("HELLOWORLD");

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        text.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
