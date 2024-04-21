#include "GSPassBase.hpp"
#include "Wrapper/Fence.hpp"
#include "Wrapper/Semaphore.hpp"
#include "sort/sort_histogram_pass.hpp"
#include "sort_multi_pass.hpp"
#include "Wrapper/Buffer.hpp"
namespace MCGS {
// class Buffer;
class SortManager : public GSPassBase {
public:
    void Init() override;
    void run_pass(vk::CommandBuffer& cmd) override;

    // private:
    void prepare_buffer() override;
    void prepare_shader_pc() override
    {
    }
    void prepare_descriptorset() override
    {
    }
    const uint num_element = 1000000;

    std::shared_ptr<SortHistogramPass> sort_histogram_pass;
    std::shared_ptr<SortMultiPass> sort_multi_pass;
    std::shared_ptr<SortHistogramPass> sort_histogram_pass2;
    std::shared_ptr<SortMultiPass> sort_multi_pass2;


    TestAddr testaddr;
    std::shared_ptr<Uniform_Stuff<TestAddr>> test_data;

    std::vector<uint64_t> element_in;
    std::vector<uint64_t> ping_pong;

    std::vector<uint32_t> histograms;
    std::shared_ptr<Buffer> element_in_buffer;
    std::shared_ptr<Buffer> pingpong_buffer;
    std::shared_ptr<Buffer>histograms_buffer;

    std::shared_ptr<Uniform_Stuff<uint64_t>> element_in_data;
    std::shared_ptr<Uniform_Stuff<uint64_t>> ping_pong_data;

    std::shared_ptr<Uniform_Stuff<uint32_t>> histograms_data;
    std::vector<Semaphore> semaphores;
    std::vector<Fence> fences;
};
}