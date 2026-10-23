#include "hubflow/tracking/application/statusMilestones.hpp"

namespace hubflow::tracking::application
{
namespace
{
using logistics::domain::ShipmentStatus;

struct Translation
{
    ShipmentStatus    status;
    domain::Milestone milestone;
};

constexpr Translation TRANSLATIONS[] {{ShipmentStatus::Recibido, domain::Milestone::Received},
                                      {ShipmentStatus::Clasificado, domain::Milestone::Prepared},
                                      {ShipmentStatus::EnReparto, domain::Milestone::OnTheWay},
                                      {ShipmentStatus::Reprogramado, domain::Milestone::RetryScheduled},
                                      {ShipmentStatus::Entregado, domain::Milestone::Delivered}};
} // namespace

domain::Milestone StatusMilestones::of(const ShipmentStatus status) noexcept
{
    domain::Milestone milestone {domain::Milestone::Received};
    for (const auto& translation : TRANSLATIONS)
    {
        if (translation.status == status)
        {
            milestone = translation.milestone;
        }
    }
    return milestone;
}

} // namespace hubflow::tracking::application
