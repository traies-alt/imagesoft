#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include <stdio.h>
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <GLFW/glfw3.h>
#include <SOIL.h>
#include "ImageWindowState.h"
#include <vector>
#include <filesystem>
#include <memory>
#include <functional>
#include <sstream>     

using namespace std;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool SimpleFileNavigation(string &path, function<void(filesystem::path)> onFileSelect)
{
    ImGui::Text(path.c_str());
    if (ImGui::Button("..")) {
        auto pathEntry = filesystem::directory_entry(path);
        path = pathEntry.path().parent_path().string();
    }
    for (const auto & entry : filesystem::directory_iterator(path)) {
        if (ImGui::Button(entry.path().filename().string().c_str())) {
            if (entry.is_directory()) {
                path = entry.path().string();
            } else {
                onFileSelect(entry.path());
            }
        }
    }
    return true;
}



int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    if (!gladLoadGL())
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    float zoom = 1;

    auto imageWindows = vector<unique_ptr<ImageWindowState>>();
    bool showFileSelect = false;
    bool showFileSelectError = false;
    int imageID = 0;
    string path = filesystem::current_path().string();
    char saveFileName[50] = {0};
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (showFileSelectError) {
            ImGui::OpenPopup("Image Select Error");
            if (ImGui::BeginPopupModal("Image Select Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("File could not be opened. Was it an Image?");
                if (ImGui::Button("Accept")) {
                    showFileSelectError = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

        }
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        if (showFileSelect) {
            ImGui::Begin("Choose File Window");
           

            // Simple File Navigation
            SimpleFileNavigation(path, [&](filesystem::path p){
                auto imOpt = LoadImageFile(p.string().c_str());
                if (imOpt.has_value()) {
                    auto val = imOpt.value();
                    val.id = imageID++;
                    imageWindows.push_back(make_unique<ImageWindowState>(val));
                    showFileSelect = false;
                } else {
                    showFileSelectError = true;
                }
            });
            ImGui::End();
        }
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    showFileSelect = true;
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    
                }
                if (ImGui::MenuItem("Save As..")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "Alt+F4")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            for (auto it = imageWindows.begin(); it != imageWindows.end(); it++) {
                auto im = it->get();
                static float f = 0.0f;
                static int counter = 0;
                auto title = string("Image Window ").append(to_string(im->id));
                ImGui::Begin(title.c_str());

                if (im->texture != 0) {
                    ImGui::SliderFloat("Zoom", &im->zoom, 0, 2.0f);
                    ImGui::Image(reinterpret_cast<ImTextureID>(im->texture), ImVec2(im->zoom * im->width, im->zoom * im->height));
                    if (ImGui::Button("Save")) {
                        ImGui::OpenPopup("Save");
                    }
                    if (ImGui::BeginPopup("Save")) {
                        
                        SimpleFileNavigation(path, [&saveFileName](filesystem::path p){
                            strncpy_s(saveFileName, p.filename().string().c_str(), 50);
                        });
                        ImGui::InputText("New image name", saveFileName, 50);
                        if (ImGui::Button("Save")) {
                            std::stringstream fnBuffer;   
                            fnBuffer << path << "/" << string(saveFileName);   
                            SaveImageFile(fnBuffer.str().c_str(), im);
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                }
                ImGui::End();
            }
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
