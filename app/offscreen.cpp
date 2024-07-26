#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "example/offscreen/App.hpp"
#include <iostream>
#include <vulkan/vulkan_core.h>

int main(int, char**)
{

    MCRT::App app;
    app.init();
    try {

        app.run();
    } catch (const std::system_error& e) {
        if (e.code().value() == VK_ERROR_DEVICE_LOST) {
            // #if _WIN32
            //             MessageBoxA(nullptr, e.what(), "Fatal Error", MB_ICONERROR | MB_OK | MB_DEFBUTTON1);
            // #endif
            std::cout << "e.what()" << std::endl;
        }
        std::cout << e.what() << std::endl;
        return e.code().value();
    }
}
