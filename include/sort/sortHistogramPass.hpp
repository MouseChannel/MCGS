

#include "gaussian_context.hpp"
namespace MCGS {
class SortHistogram_Pass : public ComputePass<GaussianContext> { 
public:
    SortHistogram_Pass( std::weak_ptr<GaussianContext> context);
    
    void Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z);
    void Init();
    PushContant_SortHisgram pc;
};
}