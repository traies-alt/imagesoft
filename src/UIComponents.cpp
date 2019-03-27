#include "UIComponents.h"
#include <sstream>
#include "imgui.h"
#include <stdlib.h>
#include <glad/glad.h>

using namespace std;

/**
 * Returns true on file select.
 *
 */
bool SimpleFileNavigation(string &path, filesystem::path &outFile)
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
								outFile = entry.path();
								return true;
            }
        }
    }
    return false;
}

bool ImageWindow(ImageWindowState &im, GLuint vertexBuffer, GLuint uvbuffer)
{
	auto title = string("Image Window ").append(to_string(im.id));

	if (ImGui::Begin(title.c_str())) {
		if (im.texture != 0) {
				ImGui::SliderFloat("Zoom", &im.zoom, 0, 2.0f);
				ImGui::Image(reinterpret_cast<ImTextureID>(im.texture), ImVec2(im.zoom * im.width, im.zoom * im.height));
				GLuint t = im.texture;
				GLuint ot;
				for (auto const& filter: im.filters) {
					t = filter->ApplyFilter(t);
				}
				ImGui::SameLine();
				ImGui::Image(reinterpret_cast<ImTextureID>(t), ImVec2(im.zoom * im.width, im.zoom * im.height));
				if (ImGui::Button("Save")) {
						ImGui::OpenPopup("Save");
				}
				if (ImGui::BeginPopup("Save")) {
					filesystem::path p;
					static char saveFileName[50];
					if (SimpleFileNavigation(im.outputPath, p)) {
							strncpy_s(saveFileName, p.filename().string().c_str(), 50);
					}
					ImGui::InputText("New image name", saveFileName, 50);
					if (ImGui::Button("Save")) {
							std::stringstream fnBuffer;
							fnBuffer << im.outputPath << "/" << string(saveFileName);
							SaveImageFile(fnBuffer.str().c_str(), &im);
							ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::InputInt("Width", &im.width);
				ImGui::InputInt("Height", &im.height);
				if (ImGui::Button("Reload Image")) {
						ReloadImage(&im);
				}
					if (ImGui::Button("Color Format"))
						ImGui::OpenPopup("Color Format");
				ImGui::Text("Color Format: %d", im.colorFormat);
				if (ImGui::BeginPopup("Color Format")) {
						if (ImGui::Selectable("GL_RED")) {
								im.colorFormat = GL_RED;
								ImGui::CloseCurrentPopup();
						}
						if (ImGui::Selectable("GL_GREEN")) {
								im.colorFormat = GL_GREEN;
								ImGui::CloseCurrentPopup();
						}
						if (ImGui::Selectable("GL_BLUE")) {
								im.colorFormat = GL_BLUE;
								ImGui::CloseCurrentPopup();
						}
						if (ImGui::Selectable("GL_LUMINANCE")) {
								im.colorFormat = GL_LUMINANCE;
								ImGui::CloseCurrentPopup();
						}
						if (ImGui::Selectable("GL_RGBA")) {
								im.colorFormat = GL_RGBA;
								ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
				}
		}
		ImGui::End();
	}
	return true;
}