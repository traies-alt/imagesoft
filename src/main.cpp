#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "FilesystemAdapter.h"
#include <stdio.h>
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <vector>
#include <memory>
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tuple>

#include "ImageWindowState.h"
#include "UIComponents.h"
#include "ShaderLayer.h"
#include "Filters.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

using namespace std;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


struct SiftState {
	fs::path path1, path2;
	std::string p1, p2;
	bool b1 = false, b2 = false, pending_update = false, pending_filter = false, pending_draw = false;
	GLuint texture = 0;
	bool texture_present = false;
	ImVec2 size = ImVec2(0, 0);
	float zoom = 1;
	std::vector< std::vector<cv::DMatch> > matches, filtered;
	cv::Mat img1, img2;
	std::vector<cv::KeyPoint> keypoints1, keypoints2;
	float threshold = 0.75;
	bool drawUnmatch = false;
	int k = 2;
	float edgeThreshold = 10, contrastThreshold = 0.03, sigma = 1.6f;
	int octaveLayers = 2;
};



int real_main(int, char**)
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
	 glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMaximizeWindow(window);
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

	// GLuint programID = LoadShaders("./src/shaders/SimpleVertexShader.vertexshader", "./src/shaders/SimpleFragmentShader.fragmentshader");
	// GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

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


	enum Shape { CIRCLE, SQUARE, GRADIENT };
	float zoom = 1;
	auto imageWindows = vector<unique_ptr<ImageWindowState>>();
	bool showFileSelect = false;
	bool showVideoSelect = false;
	bool showFileSelectError = false;
	bool showFileSelectRaw = false;
	bool generateImage = false;
	Shape shape = CIRCLE;

	int imageID = 0;
	string path = fs::current_path().string();
	char saveFileName[50] = {0};
	static bool no_close = false;
	bool sift = false;

	SiftState siftState;
	InitVertexBuffer();
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
		if (ImGui::Begin("Counter")) {
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		if (sift) {
			ImGui::Begin("Sift Window");
			if (ImGui::Button("First Image")) {
					ImGui::OpenPopup("FirstImage");
			}
			ImGui::SameLine();
			ImGui::Text(siftState.path1.filename().string().c_str());
			if (ImGui::Button("Second Image")) {
					ImGui::OpenPopup("SecondImage");
			}
			ImGui::SameLine();
			ImGui::Text(siftState.path2.filename().string().c_str());
			if (ImGui::BeginPopup("FirstImage")) {
					if (SimpleFileNavigation(siftState.p1, siftState.path1, false)) {
							ImGui::CloseCurrentPopup();
							siftState.b1 = true;
							siftState.pending_update = true;
					}
					ImGui::EndPopup();
			}
			
			if (ImGui::BeginPopup("SecondImage")) {
					if (SimpleFileNavigation(siftState.p2, siftState.path2, false)) {
							ImGui::CloseCurrentPopup();
							siftState.b2 = true;
							siftState.pending_update = true;
					}
					ImGui::EndPopup();
			}

			if (siftState.texture == 0) {
				glGenTextures(1, &siftState.texture);
				glBindTexture(GL_TEXTURE_2D, siftState.texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			}

			if (siftState.texture_present) {
				ImGui::SliderFloat("Zoom", &siftState.zoom, 0, 2, "%.2f");
				glBindTexture(GL_TEXTURE_2D, siftState.texture);
				ImGui::Image(reinterpret_cast<ImTextureID>(siftState.texture), ImVec2(siftState.size.x * siftState.zoom, siftState.size.y * siftState.zoom));
			}

			if (ImGui::SliderFloat("Threshold", &siftState.threshold, 0, 1)) {
				siftState.pending_filter = true;
			}
			if (ImGui::Checkbox("Draw not matched", &siftState.drawUnmatch)) {
				siftState.pending_draw = true;
			}
			if (ImGui::InputInt("K", &siftState.k)) {
				siftState.pending_update = true;
			}
			if (ImGui::InputFloat("Contrast Threshold", &siftState.contrastThreshold, 0.001, 0.01, 3)) {
				siftState.pending_update = true;
			}
			if (ImGui::InputFloat("Edge Threshold", &siftState.edgeThreshold, 0.1, 1, 1)) {
				siftState.pending_update = true;
			}
			if (ImGui::InputFloat("Sigma", &siftState.sigma, 0.01, 0.1, 2)) {
				siftState.pending_update = true;
			}
			if (ImGui::InputInt("Octave Layers", &siftState.octaveLayers)) {
				siftState.pending_update = true;
			}
			if (siftState.b1 && siftState.b2) {
				if (siftState.pending_update && siftState.b1 && siftState.b2) {
					siftState.img1 = cv::imread(siftState.path1.string().c_str(), cv::IMREAD_GRAYSCALE);
					siftState.img2 = cv::imread(siftState.path2.string().c_str(), cv::IMREAD_GRAYSCALE);
					auto detector = cv::xfeatures2d::SiftFeatureDetector::create(0, siftState.octaveLayers, siftState.contrastThreshold, siftState.edgeThreshold, siftState.sigma);
					cv::Mat descriptors1, descriptors2;
					detector->detectAndCompute(siftState.img1, cv::noArray(), siftState.keypoints1, descriptors1);
					detector->detectAndCompute(siftState.img2, cv::noArray(), siftState.keypoints2, descriptors2);
					auto matcher = cv::BFMatcher::create();
					matcher->knnMatch(descriptors1, descriptors2, siftState.matches, siftState.k);
					siftState.pending_update = false;
					siftState.pending_filter = true;
				}
				if (siftState.pending_filter) {
					siftState.filtered.clear();
					for(const auto& m: siftState.matches) {
						if (m[0].distance < siftState.threshold * m[1].distance) {
							siftState.filtered.push_back(m);
						}
					}
					siftState.pending_filter = false;
					siftState.pending_draw = true;
				}
				if (siftState.pending_draw){
					// Add results to image and save.
					cv::Mat output;
					if (siftState.drawUnmatch) {
						cv::drawMatches(siftState.img1, siftState.keypoints1, siftState.img2, siftState.keypoints2, siftState.filtered, output);
					} else {
						cv::drawMatches(siftState.img1, siftState.keypoints1, siftState.img2, siftState.keypoints2, siftState.filtered, output, cv::Scalar::all(-1), cv::Scalar::all(-1), std::vector<std::vector<char>>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
					}
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, output.cols, output.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, output.ptr());
					// cv::imshow("Matches", output);
					siftState.texture_present = true;
					siftState.size =  ImVec2(output.size[1], output.size[0]);
					siftState.pending_draw = false;
				}
			}
			ImGui::End();
		}

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
		if (show_demo_window) {
			ImGui::ShowDemoWindow(&show_demo_window);
		}

		if (showVideoSelect) {
			ImGui::Begin("Choose Directory Window");

			// Simple File Navigation
			fs::path p;
			if (SimpleFileNavigation(path, p, true)) {
				cout << p.string().c_str() << endl;
				auto imOpt = LoadVideoFile(p.string().c_str());
				if (imOpt.has_value()) {
					auto val = imOpt.value();
					imageWindows.push_back(make_unique<ImageWindowStateVideo>(val));

					auto videoState = (ImageWindowStateVideo*)imageWindows.back().get();;

                    IFilter * videoFilter = new VideoFilter(
							videoState->width,
							videoState->height,
							videoState);

					videoFilter->InitShader();
					videoState->filters.push_back(videoFilter);
					showVideoSelect = false;
				} else {
					showVideoSelect = true;
				}
			}
			ImGui::End();
		}

		if (showFileSelect) {
			ImGui::Begin("Choose File Window");

			// Simple File Navigation
			fs::path p;
			if (SimpleFileNavigation(path, p, false)) {
				cout << p.string().c_str() << endl;
				auto imOpt = LoadImageFile(p.string().c_str());
				if (imOpt.has_value()) {
					auto val = imOpt.value();
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
			static int rawWidth = 256, rawHeight = 256;
			ImGui::InputInt("Raw Width", &rawWidth);
			ImGui::InputInt("Raw Height", &rawHeight);
			fs::path p;
			if (SimpleFileNavigation(path, p, false)) {
				 auto imOpt = LoadImageFileRaw(p.string().c_str(), rawWidth, rawHeight);
				if (imOpt.has_value()) {
					auto val = imOpt.value();
					imageWindows.push_back(make_unique<ImageWindowState>(val));
					showFileSelect = false;
				} else {
					showFileSelectError = true;
				}
			}
			ImGui::End();
		}

		if (generateImage) {
			static int size = 200;
			ImGui::Begin("Generate Shape");
			switch (shape) {
				case CIRCLE:
				case SQUARE: {
                    static float filled = 0.8;
                    static float color[3] = {1, 1, 1};

                    ImGui::InputInt("Size", &size);
                    ImGui::SliderFloat("Filled Percentage", &filled, 0.1, 0.9);
                    ImGui::ColorEdit3("Color", color);

                    static float to[3] = {0, 0, 0};
                    if (ImGui::Button("Generate")) {
                        auto image = new unsigned char[3 * size * size];

                        if (shape == SQUARE) {
                            float margin = size / 2.0f * filled;
                            int center = size / 2;
                            fillBuffer(image, size, size, [margin, center](int i, int j) -> std::tuple<u_char, u_char, u_char> {
								if(abs((i - center)) < margin && abs(j - center) < margin) {
									return std::make_tuple(0xFF, 0xFF, 0xFF);
								} else {
									return std::make_tuple(0, 0, 0);
								}
                            });

                        } else if (shape == CIRCLE) {
                            float radius = size / 2.0f * filled;
                            int center = size / 2;

                            fillBuffer(image, size, size, [center, radius](int i, int j) -> std::tuple<u_char, u_char, u_char> {
                                int x = i - center;
                                int y = j - center;
                                if(x * x + y * y < radius * radius) {
                                	return std::make_tuple(0xFF, 0xFF, 0xFF);
                                } else {
									return std::make_tuple(0, 0, 0);
                                }
                            });
                        }

                        imageWindows.push_back(make_unique<ImageWindowState>(CreateImage(image, size, size).value()));
                        delete[] image;
                        generateImage = false;
                    }
                }

					break;
				case GRADIENT: {
                    static float from[3] = {0, 0, 0};
                    static float to[3] = {0, 0, 0};
                    ImGui::InputInt("Size", &size);
                    ImGui::ColorEdit3("From", from);
                    ImGui::ColorEdit3("To", to);

                    if (ImGui::Button("Generate")) {
						auto image = new unsigned char[3 * size * size];

						fillBuffer(image, size, size, [](int i, int j) -> std::tuple<u_char, u_char, u_char> {
							return std::make_tuple(
									(to[0] + (from[0] - to[0]) * j/size) * 0xFF,
									(to[1] + (from[1] - to[1]) * j/size) * 0xFF,
									(to[2] + (from[2] - to[2]) * j/size) * 0xFF);
						});

						imageWindows.push_back(make_unique<ImageWindowState>(CreateImage(image, size, size).value()));
						delete[] image;
						generateImage = false;
                    }
                }
					break;
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
				if (ImGui::MenuItem("Open Video", "Ctrl+O")) {
					showVideoSelect = true;
				}
				if (ImGui::MenuItem("Open Raw", "Ctrl+O")) {
					showFileSelectRaw = true;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Generate"))
			{
				if (ImGui::MenuItem("Circle")) {
					generateImage = true;
					shape = CIRCLE;
				}

				if (ImGui::MenuItem("Square")) {
					generateImage = true;
					shape = SQUARE;
				}

				if (ImGui::MenuItem("Gradient")) {
					generateImage = true;
					shape = GRADIENT;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("SIFT"))
			{   
					sift = true;
					ImGui::EndMenu();
			}   
			ImGui::EndMainMenuBar();
		}

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		for (auto it = imageWindows.begin(); it != imageWindows.end(); it++) {
			auto im = it->get();
			static float f = 0.0f;
			static int counter = 0;
			auto title = string("Image Window ").append(to_string(im->id));
			ImageWindow(*im, vertexbuffer, uvbuffer);
			ImGui::Begin((string("Filters") + to_string(im->id)).c_str());
			for (auto it = im->filters.begin(); it != im->filters.end(); ) {
				auto index = it - im->filters.begin();

				std::ostringstream treeTitle;
				treeTitle << std::to_string(index) << " " << (*it)->_name;
				std::string filterTitle = treeTitle.str();
				if(ImGui::TreeNode(filterTitle.c_str())) {
					if (ImGui::Button("Remove filter")) {
						delete *it;
						it = im->filters.erase(it);
					} else {
					    if(it != im->filters.begin()) {
                            ImGui::Checkbox("Enabled", &(*it)->_enabled);

                            ImGui::SameLine();
							if (ImGui::Button("Move Up")) {
								std::iter_swap(it, it - 1);
							}
						}
					    if(it != im->filters.end() - 1) {
							ImGui::SameLine();
							if (ImGui::Button("Move Down")) {
								std::iter_swap(it, it + 1);
							}
						}
						(*it)->RenderUI();
						it++;
					}
					ImGui::TreePop();
					ImGui::Separator();
				} else {
					it++;
				}
			};
			if (ImGui::Button("Add Filter")) {
				ImGui::OpenPopup("Add Filter");
			}

			if (ImGui::BeginPopup("Add Filter")) {

				auto FilterButton = [&im](char * name, std::function<IFilter * (int width, int height)> constr) {
					if (ImGui::Button(name)) {
						IFilter * filter = constr(im->width, im->height);
						filter->InitShader();
						im->filters.push_back(filter);
						ImGui::CloseCurrentPopup();
					}
				};

				FilterButton("SingleBand", [](int width, int height){
					return new SingleBandFilter(width, height);
				});
				FilterButton("Subtract", [](int width, int height){
					return new SubstractionFilter(width, height);
				});
				FilterButton("Negative", [](int width, int height){
					return new NegativeFilter(width, height);
				});
				FilterButton("Scalar", [](int width, int height){
					return new ScalarFilter(width, height);
				});
				FilterButton("DynamicRangeCompression", [](int width, int height){
					return new DynamicRangeCompressionFilter(width, height);
				});
				FilterButton("Gamma", [](int width, int height){
					return new GammaFilter(width, height);
				});
				FilterButton("Threshold", [](int width, int height){
					return new ThresholdFilter(width, height);
				});
				FilterButton("Contrast", [](int width, int height){
					return new ContrastFilter(width, height);
				});
				FilterButton("Equalization", [](int width, int height){
					return new EqualizationFilter(width, height);
				});
				FilterButton("ExponentialNoise", [](int width, int height){
					return new ExponentialNoiseFilter(width, height);
				});
				FilterButton("RayleighNoise", [](int width, int height){
					return new RayleighNoiseFilter(width, height);
				});
				FilterButton("GaussianNoise", [](int width, int height){
					return new GaussianNoiseFilter(width, height);
				});
				FilterButton("SaltAndPepper", [](int width, int height){
					return new SaltAndPepperNoiseFilter(width, height);
				});
				FilterButton("Mean", [](int width, int height){
					return new MeanFilter(width, height);
				});
				FilterButton("Median", [](int width, int height){
					return new MedianFilter(width, height);
				});
				FilterButton("Border", [](int width, int height){
					return new BorderFilter(width, height);
				});
				FilterButton("Laplace", [](int width, int height){
					return new LaplaceFilter(width, height);
				});
				FilterButton("Global Threshold", [](int width, int height){
						return new GlobalThresholdFilter(width, height);
				});
				FilterButton("Otsu Threshold", [](int width, int height){
					return new OtsuThresholdFilter(width, height);
				});
				FilterButton("Bilateral Filter", [](int width, int height){
					return new BilateralFilter(width, height);
				});
				FilterButton("Heat Filter", [](int width, int height){
					return new HeatFilter(width, height);
				});
				FilterButton("Susan Filter", [](int width, int height){
					return new SusanFilter(width, height);
				});
				FilterButton("Canny Filter", [](int width, int height){
					return new CannyFilter(width, height);
				});
				FilterButton("Hugh Filter", [](int width, int height){
					return new HughFilter(width, height);
				});
				FilterButton("Hugh Cicle Filter", [](int width, int height){
					return new HughCircleFilter(width, height);
				});
				FilterButton("Active Border Filter", [](int width, int height){
						return new ActiveBorder(width, height);
				});
				FilterButton("Harris Filter", [](int width, int height){
					return new HarrisFilter(width, height);
				});
				ImGui::EndPopup();
			}
			ImGui::End();
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


int main(int argc, char * argv[])
{
	try {
		real_main(argc, argv);
	}
	catch (std::exception &e) {
		cout << e.what() << endl;
	}
}

// void Sift()
// {
//     const cv::Mat input = cv::imread("./images/lenna.bmp", 0); //Load as grayscale

//     cv::Ptr<cv::xfeatures2d::SiftFeatureDetector> detector = cv::xfeatures2d::SiftFeatureDetector::create();
//     std::vector<cv::KeyPoint> keypoints;
//     detector->detect(input, keypoints);
//     // Add results to image and save.
//     cv::Mat output;
//     cv::drawKeypoints(input, keypoints, output, -1, cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
//     cv::imwrite("sift_result.jpg", output);
//     return 0;
// }
