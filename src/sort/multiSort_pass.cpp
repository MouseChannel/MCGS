#include "sort/multiSort_pass.hpp"
namespace MCGS {

MultiSortPass::MultiSortPass(std::weak_ptr<GaussianContext> context)
{
    sortHistogramPass.reset(new SortHistogram_Pass(context));
    sortMultiPass.reset(new SortMulti_Pass(context));
}
void MultiSortPass::Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z)
{
    auto device = Context::Get_Singleton()->get_device();
    auto queue = device->Get_Compute_queue();
    PushContant_SortHisgram pc {
        .g_num_elements = static_cast<uint>(GaussianContext::num_renders),
        // .g_shift = i * 8,
        .g_num_workgroups = uint(ceil((float)GaussianContext::num_renders / 256.f / 32.f)),
        .g_num_blocks_per_workgroup = 32,
        // .pass = cur_fight
    };

    for (uint i = 0; i < 8; i++) {
        auto cur_fight = i % 2;

        pc.g_shift = i * 8;
        pc.pass = cur_fight;

        sortHistogramPass->pc = pc;
        sortMultiPass->pc = pc;

        sortHistogramPass->Dispach(cmd, 0, 0, 0);
        sortMultiPass->Dispach(cmd, 0, 0, 0);
    }
}
}