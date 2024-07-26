#pragma once
#include "Wrapper/ComputePass/ComputePass.hpp"
// #include "GSPassBase.hpp"
#include "gaussian_context.hpp"
#include "shaders/push_contant.h"
namespace MCGS {
using namespace MCRT;
class Raster_Pass : public ComputePass<GaussianContext> {
    
public:
    Raster_Pass(std::weak_ptr<GaussianContext> computeContext);
    void Init();
    virtual void Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z);
};

}