#pragma once
 
#include "gaussian_context.hpp"
namespace MCGS {

class duplicate_Pass : public ComputePass<GaussianContext> {
public:
    duplicate_Pass(std::weak_ptr<GaussianContext> context);
    void Init();
    void Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z);
};
}