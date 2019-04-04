#include "UIComponents.h"
#include <sstream>
#include "imgui.h"
#include <stdlib.h>
#include <glad/glad.h>
#include "ShaderLayer.h"

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

float HistMaxValue(float * hist, int from, int to)
{
	float max = 0;
	for (int i = from; i < to; i++) {
		if (max < hist[i]) {
			max = hist[i];
		}
	}
	return max;
}

bool ImageWindow(ImageWindowState &im, GLuint vertexBuffer, GLuint uvbuffer)
{
	auto title = string("Image Window ").append(to_string(im.id));

	ImGui::Begin(title.c_str());
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

			ImGui::Begin(string("Histogram").append(to_string(im.id)).c_str());
			
			ImGui::SliderInt2("Histogram Bounds", &im._histStart, 0, 256);
			im._histStart = clamp(im._histStart, 0, im._histEnd);
			im._histEnd = clamp(im._histEnd, im._histStart, 256);
			float scale = HistMaxValue(im._hist, im._histStart, im._histEnd);
			ImGui::PlotHistogram("Histogram", im._hist + im._histStart, im._histEnd - im._histStart, 0, NULL, 0, scale , ImVec2(0,200));
			if (ImGui::Selectable("RED", im.histogramBand == 0)) {
				im.histogramBand = 0;
			} else if (ImGui::Selectable("GREEN", im.histogramBand == 1)) {
				im.histogramBand = 1;
			} else if (ImGui::Selectable("BLUE", im.histogramBand == 2)) {
				im.histogramBand = 2;
			} else if (ImGui::Selectable("LUMINOSITY", im.histogramBand == 3)) {
				im.histogramBand = 3;
			}
			ImGui::Checkbox("Calc histogram every frame?", &im._calcHistogram);

			if (im._calcHistogram) {
				memset(im._hist, 0, sizeof(im._hist));
				im._maxVal = GetHistogram(t, im.width, im.height, im.histogramBand, im._hist);
			}
			ImGui::End();

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
					if (ImGui::Selectable("GL_RGB")) {
							im.colorFormat = GL_RGB;
							ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
			}
	}
	ImGui::End();
	return true;
}