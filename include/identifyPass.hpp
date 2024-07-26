#include "Wrapper/ComputePass/ComputePass.hpp"
#include "gaussian_context.hpp"

namespace MCGS {

class Identify_Pass : public ComputePass<GaussianContext> {
public:
    Identify_Pass(std::weak_ptr<GaussianContext> context);
    void Init();
    void Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z);
};
}
