// #include "Rendering/ComputePass.hpp"
#include "GSPassBase.hpp"
namespace MCGS {
using namespace MCRT;
class SortPass : public GSPassBase {

public:
    SortPass(std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_key, std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_value);
 
    void run_pass(vk::CommandBuffer& cmd) override;

private:
    void prepare_buffer() override;
    void prepare_shader_pc() override;
    void prepare_descriptorset() override;

    void execute(uint offset);

    std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_key;
    std::vector<uint64_t> ping_pong_temp;
    std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_pingpong;
    std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_value;

    // void record_command();
    const uint num_element = 1625771;
    // const uint num_blocks_per_workgroup = 32;
    // const uint WORKGROUP_SIZE = 256;

    // std::shared_ptr<ComputePass> low_radixsort;
    // std::shared_ptr<ComputePass> high_radixsort;

    // std::shared_ptr<ComputePass> sort_content;
    using temptype = uint64_t;
    std::vector<temptype> element_in;
    std::vector<temptype> ping_pong;
    // std::shared_ptr<Buffer> element_in_buffer;
    // std::vector<uint32_t> zeros;
    std::vector<uint32_t> element_value_in;
    std::vector<uint32_t> ping_pong_value;

    std::shared_ptr<Uniform_Stuff<temptype>> element_in_data;
    std::shared_ptr<Uniform_Stuff<temptype>> ping_pong_data;

    std::shared_ptr<Uniform_Stuff<uint32_t>> element_value_in_data;
    std::shared_ptr<Uniform_Stuff<uint32_t>> ping_pong_value_data;

    // std::shared_ptr<CommandBuffer> cmd;

    // std::shared_ptr<Uniform_Stuff<uint32_t>> zero_data;
    // std::shared_ptr<Uniform_Stuff<uint32_t>> histograms_data;

    // std::shared_ptr<Buffer> zero_buffer;
    // std::shared_ptr<Buffer> histograms_buffer;
};
}