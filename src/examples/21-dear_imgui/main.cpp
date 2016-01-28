#include "../common/util.h"
#include "../common/shader.h"
#include "imgui_impl_glfw_gl3.h" // Provided by imgui
#include "imgui/imgui.h"

int main(void)
{
    GLFWwindow* window;

    window = init("Dear IMGUI", 640, 480);
    if(!window)
    {
        return -1;
    }

    ImGui_ImplGlfwGL3_Init(window, true);

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    while(!glfwWindowShouldClose(window))
    {
        // Clear
        glClear(GL_COLOR_BUFFER_BIT);

        // Clear ImGui
        ImGui_ImplGlfwGL3_NewFrame();

        // Show a window
        ImGui::Begin("MyWindow");
        // Create a label and a button
        ImGui::Text("Hello, ImGui!");
        if (ImGui::Button("Quit"))
        {
            break;
        }
        ImGui::End();

        // Show some floating objects
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        // Window title, closing boolean, size, background alpha, flags
        ImGui::Begin("", NULL, ImVec2(0, 0), 1.0f, window_flags);
        ImGui::Text("Floating text!");
        ImGui::End();
        
        // Render ImGui
        ImGui::Render();

        // Swap buffers to show current image on screen (for more information google 'backbuffer')
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    ImGui_ImplGlfwGL3_Shutdown();

    glfwTerminate();
    return 0;
}
