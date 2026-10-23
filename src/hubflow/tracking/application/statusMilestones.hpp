#ifndef HUBFLOW_TRACKING_APPLICATION_STATUS_MILESTONES_HPP
#define HUBFLOW_TRACKING_APPLICATION_STATUS_MILESTONES_HPP

#include "hubflow/logistics/domain/valueobjects/shipmentStatus.hpp"
#include "hubflow/tracking/domain/valueobjects/milestone.hpp"

namespace hubflow::tracking::application
{

/// @brief Traduce cada estado interno del envío al hito que se le cuenta al cliente.
class StatusMilestones final
{
public:
    static domain::Milestone of(logistics::domain::ShipmentStatus status) noexcept;
};

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_STATUS_MILESTONES_HPP
