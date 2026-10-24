#include "hubflow/tracking/adapters/out/logistics/plantTrackedShipments.hpp"

#include "hubflow/tracking/application/statusMilestones.hpp"

namespace hubflow::tracking::adapters
{

    PlantTrackedShipments::PlantTrackedShipments(const logistics::application::LogisticsModel& logistics,
                                                 const routing::application::RoutingModel& routing)
        : m_logistics {logistics}
        , m_routing {routing}
    {
    }

    std::optional<application::TrackedShipment> PlantTrackedShipments::find(const std::string& code) const
    {
        std::optional<application::TrackedShipment> tracked;
        if (m_logistics.catalog.contains(code))
        {
            const auto& shipment = m_logistics.catalog.require(code);
            application::TrackedShipment view;
            view.code = shipment.code();
            shipment.history().forEachChronological(
                [&view](const logistics::domain::Movement& movement)
                {
                    view.milestones.push_back(domain::MilestoneEvent {
                        application::StatusMilestones::of(movement.status), movement.occurredAt});
                });
            view.phoneLast4 = shipment.contact().phone().lastFour();
            view.postalCode = shipment.contact().postalCode().value();
            view.recipientFirstName = shipment.contact().firstName();
            view.neighbourhood = neighbourhoodOf(shipment.deliveryPoint());
            view.parcelCount = shipment.parcelCount();
            view.parcelsDelivered = shipment.parcelCount() - shipment.pendingParcelCount();
            const auto* pZone = m_logistics.zones.find(shipment.zone());
            view.zoneName = pZone != nullptr ? pZone->name() : shipment.zone();
            tracked = view;
        }
        return tracked;
    }

    std::string PlantTrackedShipments::neighbourhoodOf(const std::string& nodeId) const
    {
        const auto index = m_routing.map.indexOf(nodeId);
        return index.has_value() ? m_routing.map.node(*index).label : nodeId;
    }

} // namespace hubflow::tracking::adapters
