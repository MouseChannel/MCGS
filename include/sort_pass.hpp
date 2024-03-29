// #include "Rendering/ComputePass.hpp"
#include "GSPassBase.hpp"
namespace MCGS {
using namespace MCRT;
class SortPass : public GSPassBase {

public:
    // SortPass();
    void Execute() override;

private:
    void prepare_buffer() override;
    void prepare_shader_pc(std::string shader_path, int pc_size) override;
    void prepare_descriptorset() override;

    void execute(uint offset);
    // void record_command();
    const uint num_element = 1000000;
    const uint num_blocks_per_workgroup = 32;
    const uint WORKGROUP_SIZE = 256;

    // std::shared_ptr<ComputePass> low_radixsort;
    // std::shared_ptr<ComputePass> high_radixsort;

    // std::shared_ptr<ComputePass> sort_content;

    std::vector<uint32_t> element_in;
    // std::shared_ptr<Buffer> element_in_buffer;
    // std::vector<uint32_t> zeros;
    std::vector<uint32_t> ping_pong;

    std::shared_ptr<Uniform_Stuff<uint32_t>> element_in_data;
    std::shared_ptr<Uniform_Stuff<uint32_t>> ping_pong_data;
    // std::shared_ptr<CommandBuffer> cmd;

    // std::shared_ptr<Uniform_Stuff<uint32_t>> zero_data;
    // std::shared_ptr<Uniform_Stuff<uint32_t>> histograms_data;

    // std::shared_ptr<Buffer> zero_buffer;
    // std::shared_ptr<Buffer> histograms_buffer;
};
}