#include "App.hpp"
#include "Helper/ImGui_Context.hpp"
#include "Helper/Uniform_Manager.hpp"
#include "Rendering/AppWindow.hpp"
#include "Rendering/Context.hpp"
#include "example/raster/raster_pbr_context.hpp"

namespace MCRT {
void App::init()
{
    window.reset(new Window(1200, 800));
    raster_context_pbr::Get_Singleton()->prepare(window);
    imgui.reset(new ImGuiContext);

    imgui->Init(window);
}

void App::run()
{
    // std::cout << "oonon" << std::endl;
    auto& context = raster_context_pbr::Get_Singleton();
    while (!window->Should_Close()) {
        window->PollEvents();
        auto cmd = context->Begin_Frame();
        imgui->Update(cmd,
                      [&]() {
                          ImGui::Text("im a sentence!");
                          ImGui::SliderFloat("light_pos_x", &raster_context_pbr::light_pos_x, -2.0f, 2.0f);
                          ImGui::SliderFloat("light_pos_y", &raster_context_pbr::light_pos_y, -2.0f, 2.0f);
                          ImGui::SliderFloat("light_pos_z", &raster_context_pbr::light_pos_z, 0.f, 20.0f);
                          ImGui::Checkbox("use_normal_map", &raster_context_pbr::use_normal_map);
                          ImGui::Checkbox("rm", &raster_context_pbr::use_r_rm_map);
                      });
        context->EndFrame();
    }
}
} // namespace MCRT
