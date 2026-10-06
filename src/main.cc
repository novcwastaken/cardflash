#include "ui/state.hh"

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define DISABLE_RESIZE false

int main() {
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #if DISABLE_RESIZE
    glfwWindowHint(GLFW_RESIZABLE, false);
    #endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Cardflash", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Disable the ini file
    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    CardflashUI::UiState* state_ptr = new CardflashUI::UiState();

    state_ptr->SetCurrentTheme(&(CardflashUI::GetThemes()->mocha));
    state_ptr->LoadFonts();
    state_ptr->Scan();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Uncomment for docking support
        // ImGui::DockSpaceOverViewport();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        state_ptr->Render(w, h, window);


        ImGui::Render();
        glViewport(0, 0, w, h);
        glClearColor(
            (float)state_ptr->GetCurrentTheme()->crust.r/255,
            (float)state_ptr->GetCurrentTheme()->crust.g/255,
            (float)state_ptr->GetCurrentTheme()->crust.b/255,
            1
        );
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    delete state_ptr;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
