#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include <stdio.h>
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <vector>
#include <filesystem>
#include <memory>
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>

#include "ImageWindowState.h"
#include "UIComponents.h"
#include "ShaderLayer.h"

using namespace std;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
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
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
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

		/* init vertex array for triangle */
		GLuint VertexArrayID;
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		// An array of 3 vectors which represents 3 vertices
		static const GLfloat g_vertex_buffer_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
		};

		static const GLfloat g_uv_buffer_data[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f,  1.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
		};

		// This will identify our vertex buffer
		GLuint vertexbuffer;
		// Generate 1 buffer, put the resulting identifier in vertexbuffer
		glGenBuffers(1, &vertexbuffer);
		// The following commands will talk about our 'vertexbuffer' buffer
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		// Give our vertices to OpenGL.
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

		GLuint uvbuffer;
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

		GLuint programID = LoadShaders("./src/SimpleVertexShader.vertexshader", "./src/SimpleFragmentShader.fragmentshader");
		GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

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
    bool showFileSelectRaw = false;
    int imageID = 0;
    string path = filesystem::current_path().string();
    char saveFileName[50] = {0};
    static bool no_close = false;

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
						filesystem::path p;
						if (SimpleFileNavigation(path, p)) {
							auto imOpt = LoadImageFile(p.string().c_str(), programID, TextureID);
							if (imOpt.has_value()) {
									auto val = imOpt.value();
									val.id = imageID++;
									imageWindows.push_back(make_unique<ImageWindowState>(val));
									showFileSelect = false;
							} else {
									showFileSelectError = true;
							}
						}
            ImGui::End();
        }

        if (showFileSelectRaw) {
            ImGui::Begin("Chose File Raw");
            static int rawWidth, rawHeight;
            ImGui::InputInt("Raw Width", &rawWidth);
            ImGui::InputInt("Raw Height", &rawHeight);
						filesystem::path p;
						if (SimpleFileNavigation(path, p)) {
                ifstream input(p.string().c_str(), ios::binary);
                std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
                GLuint texture_map;
                glGenTextures(1, &texture_map);
                glBindTexture(GL_TEXTURE_2D, texture_map);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rawWidth, rawHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer.data());
                glBindTexture(GL_TEXTURE_2D, 0);

								// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
								GLuint FramebufferName = 0;
								glGenFramebuffers(1, &FramebufferName);
								glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

								// The texture we're going to render to
								GLuint renderedTexture;
								glGenTextures(1, &renderedTexture);

								// "Bind" the newly created texture : all future texture functions will modify this texture
								glBindTexture(GL_TEXTURE_2D, renderedTexture);

								// Give an empty image to OpenGL ( the last "0" )
								glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, rawWidth, rawHeight, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);

								// Poor filtering. Needed !
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

								// Set "renderedTexture" as our colour attachement #0
								glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

								// Set the list of draw buffers.
								GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0};
								glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

								if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
									cout << "Error rendering to texture." << endl;

								unsigned char *b = new unsigned char[buffer.size()];
                std::copy(buffer.begin(), buffer.end(), b);
                ImageWindowState i = {
                    texture_map,
                    rawWidth,
                    rawHeight,
                    1.0f,
                    p.filename().string().c_str(),
                    imageID++,
                    b,
                    GL_LUMINANCE,
										FramebufferName,
										renderedTexture,
										"",
										programID,
										TextureID,
                };
                imageWindows.push_back(make_unique<ImageWindowState>(i));
                // showFileSelectRaw = false;
						}
            ImGui::End();
        }
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    showFileSelect = true;
                }
                if (ImGui::MenuItem("Open Raw", "Ctrl+O")) {
                    showFileSelectRaw = true;
                }
                if (ImGui::BeginPopup("Open Raw")) {

                    ImGui::EndPopup();
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
								ImageWindow(*im, vertexbuffer, uvbuffer);
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

				// glUseProgram(programID);
				// // 1st attribute buffer : vertices
				// glEnableVertexAttribArray(0);
				// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				// glVertexAttribPointer(
				// 	0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				// 	3,                  // size
				// 	GL_FLOAT,           // type
				// 	GL_FALSE,           // normalized?
				// 	0,                  // stride
				// 	(void*)0            // array buffer offset
				// );
				// // Draw the triangle !
				// glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
				// glDisableVertexAttribArray(0);

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
