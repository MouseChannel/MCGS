#pragma once
#include "example/base/raster_context.hpp"
// #include "shaders/Data_struct.h"
#include "Rendering/ComputePass.hpp"
#include "Rendering/GraphicPass.hpp"
#include "Rendering/Noise/NoiseManager.hpp"
#include "example/raster/shader/Constants.h"

namespace MCRT {
class Buffer;
class Skybox;
class raster_context_pbr : public raster_context {
public:
    enum Pass_index { Graphic,
                      Compute,
    };
    raster_context_pbr();
    ~raster_context_pbr();
    std::shared_ptr<CommandBuffer> Begin_Frame() override;
    void EndFrame() override;
    std::shared_ptr<RaytracingPass> get_rt_context() override
    {
        // auto base = PASS[Pass_index::Ray_tracing];
        // if (auto context = std::reinterpret_pointer_cast<RaytracingPass>(base); context != nullptr) {
        //     return context;
        // }
        throw std::runtime_error("it is not Ray_Tracing context");
    }
    std::shared_ptr<ComputePass> get_compute_context() override
    {
        auto base = PASS[Pass_index::Compute];
        if (auto context = std::reinterpret_pointer_cast<ComputePass>(base); context != nullptr) {
            return context;
        }
        // throw std::runtime_error("it is not compute context");
    }

    std::shared_ptr<GraphicPass> get_graphic_context() override
    {
        auto base = PASS[Pass_index::Graphic];
        if (auto context = std::reinterpret_pointer_cast<GraphicPass>(base); context != nullptr) {
            return context;
        }
        throw std::runtime_error("it is not graphic context");
    }
    // static float light_pos_x, light_pos_y, light_pos_z;
    // static bool use_normal_map, use_r_rm_map, use_ao;
    // static float gamma;

    void prepare(std::shared_ptr<Window> window) override;
    std::shared_ptr<Graphic_Pipeline> get_skybox_pipeline() override
    {
        return get_graphic_context()->get_skybox_pipeline();
        // throw std::runtime_error("no skybox");
    }
    std::shared_ptr<Mesh> get_skybox_mesh() override
    {
        throw std::runtime_error("no skybox");
        // return skybox_mesh;
    }

private:
    std::shared_ptr<CommandBuffer> BeginGraphicFrame() override;
    // std::shared_ptr<CommandBuffer> BeginComputeFrame();
    //    std::shared_ptr<CommandBuffer> BeginSkyboxFrame();
    //    void EndSkyboxFrame();
    // void EndComputeFrame();
    void EndGraphicFrame() override;

    // PushContant pushContant_Ray;
    // PC_Raster pc;
    // float angle;
    // std::shared_ptr<Mesh> skybox_mesh;
    // std::shared_ptr<Skybox> sky_box;


    
    std::shared_ptr<Buffer> m_vertex_buffer;
    std::shared_ptr<Buffer> index_buffer;
};

}