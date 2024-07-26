#pragma once
// #include "Helper/ImGui_Context.hpp"
#include <memory>

namespace MCRT {
// class ImGuiContext;
class Window;
// class Context;
class App {
public:
    App() = default;
    ~App();
    void init();
    void run();

private:
    std::shared_ptr<Window> window;

    // std::unique_ptr<ImGuiContext> imgui;
};
} // namespace MCRT