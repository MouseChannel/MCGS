// #include "Rendering/ComputePass.hpp"
#include "GSPassBase.hpp"
#include "shaders/push_contant.h"
namespace MCGS {
using namespace MCRT;
class ProcessPass : public GSPassBase {

public:
    void Execute() override;
    // void set_address(std::shared_ptr<Uniform_Stuff<GS_Address>> _address)
    // {
    //     address = _address;
    // }

private:
    void prepare_buffer() override;
    void prepare_shader_pc() override;
    void prepare_descriptorset() override;

    // void execute(uint offset);

    std::shared_ptr<Uniform_Stuff<GS_Address>> address;
};
}