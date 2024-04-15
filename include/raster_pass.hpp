// #include "Rendering/ComputePass.hpp"
#include "GSPassBase.hpp"
#include "shaders/push_contant.h"
namespace MCGS {
// using namespace MCRT;
class RasterPass : public GSPassBase {

public:
    RasterPass() = default;
    void run_pass(vk::CommandBuffer& cmd) override;
    // void Execute() override;
   

    std::shared_ptr<MCRT::Image> render_out;
private:

    void prepare_buffer() override;
    void prepare_shader_pc() override;
    void prepare_descriptorset() override;

     
};
}