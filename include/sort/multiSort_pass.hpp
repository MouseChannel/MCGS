#pragma once

#include "gaussian_context.hpp"
#include "sortHistogramPass.hpp"
#include "sortMultiExcutePass.hpp"
namespace MCGS {

class MultiSortPass : public ComputePass<GaussianContext> {
public:
    MultiSortPass(std::weak_ptr<GaussianContext> context);
    void Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z);

private:
    std::shared_ptr<SortHistogram_Pass> sortHistogramPass;
    std::shared_ptr<SortMulti_Pass> sortMultiPass;
};

}