// #include "Rendering/ComputePass.hpp"
#include "GSPassBase.hpp"
#include "shaders/push_contant.h"
namespace MCGS {
using namespace MCRT;
class SumPass : public GSPassBase {

public:
    void Execute() override;
    // void set_address(std::shared_ptr<Uniform_Stuff<GS_Address>> _address)
    // {
    //     address = _address;
    // }

private:
    void execute_first();
    void execute_second();
    void execute_last();
    void execute_test();

    void prepare_buffer() override;
    void prepare_shader_pc() override;
    void prepare_descriptorset() override;

    // std::shared_ptr<Uniform_Stuff<GS_Address>> address;
    // void execute(uint offset);

    const uint num_element = 1000000;
    // const uint num_blocks_per_workgroup = 32;
    // const uint WORKGROUP_SIZE = 256;

    std::vector<uint32_t> element_in;

    std::shared_ptr<Uniform_Stuff<uint32_t>> element_in_data;
};
}