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
				if (im.outputName != 0) {
					glBindFramebuffer(GL_FRAMEBUFFER, im.outputName);
					glViewport(0,0,im.width,im.height);
					glClear(GL_COLOR_BUFFER_BIT);

					//match projection to window resolution (could be in reshape callback)
					// glMatrixMode(GL_PROJECTION);

					glUseProgram(im.mainShader);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, im.texture);

					// Texture Sampler
					glUniform1i(im.textureSampler, 0);

					glEnableVertexAttribArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
					glVertexAttribPointer(
						0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
						3,                  // size
						GL_FLOAT,           // type
						GL_FALSE,           // normalized?
						0,                  // stride
						(void*)0            // array buffer offset
					);

					glEnableVertexAttribArray(1);
					glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
					glVertexAttribPointer(
						1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
						2,                                // size : U+V => 2
						GL_FLOAT,                         // type
						GL_FALSE,                         // normalized?
						0,                                // stride
						(void*)0                          // array buffer offset
					);
					// Draw the triangle !
					glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 3 vertices total -> 1 triangle
					glDisableVertexAttribArray(0);
					glDisableVertexAttribArray(1);

					glDisable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, 0);
					glFlush();

					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					ImGui::Image(reinterpret_cast<ImTextureID>(im.outputTexture), ImVec2(im.zoom * im.width, im.zoom * im.height));
				}
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