#pragma once
#include "Wrapper/Texture.hpp"

#include "Context/raster_context.hpp"
// #include "shaders/Data_struct.h"
#include "Rendering/GraphicContext.hpp" 
 

namespace MCRT {
class Buffer;
// class Skybox;
class offscreen_context : public raster_context {
public:
    enum Pass_index { Graphic };
 
    offscreen_context();
    ~offscreen_context();
    std::shared_ptr<CommandBuffer> Begin_Frame() override;
    void EndFrame() override;
    std::shared_ptr<RaytracingPass> get_rt_context() override
    {
        throw std::runtime_error("it is not Ray_Tracing context");
    }
    std::shared_ptr<ComputeContext> get_compute_context() override
    {

        throw std::runtime_error("it is not compute context");
    }

    std::shared_ptr<GraphicContext> get_graphic_context() override
    {
        auto base = PASS[Pass_index::Graphic];
        if (auto context = std::reinterpret_pointer_cast<GraphicContext>(base); context != nullptr) {
            return context;
        }
        throw std::runtime_error("it is not Ray_Tracing context");
    }

    static float light_pos_x, light_pos_y, light_pos_z;
    static bool use_normal_map, use_r_rm_map, use_ao;
    static float gamma;

    void prepare(std::shared_ptr<Window> window) override;

private:
    std::shared_ptr<CommandBuffer> BeginGraphicFrame() override;
    std::shared_ptr<Texture> target_texture; void EndGraphicFrame() override;
    std::shared_ptr<Mesh> offscreen_mesh;


    vk::Buffer indirecBuffer;

    
};

}