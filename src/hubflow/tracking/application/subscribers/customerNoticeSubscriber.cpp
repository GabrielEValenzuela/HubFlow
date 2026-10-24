#include "hubflow/tracking/application/subscribers/customerNoticeSubscriber.hpp"

#include "hubflow/logistics/domain/valueobjects/phone.hpp"
#include "hubflow/tracking/domain/services/customerWording.hpp"

#include <iostream>
#include <utility>

namespace hubflow::tracking::application
{
    namespace
    {
        constexpr const char* CHANNEL {"SMS"};

        std::size_t deliveredParcels(const logistics::domain::ShipmentState& state)
        {
            std::size_t delivered {0};
            for (std::size_t parcelIndex = 0; parcelIndex < state.draft.parcels.size(); ++parcelIndex)
            {
                delivered += state.draft.parcels[parcelIndex].delivered ? 1 : 0;
            }
            return delivered;
        }
    } // namespace

    CustomerNoticeSubscriber::CustomerNoticeSubscriber(CustomerNotifier& notifier, std::string trackingLinkBase)
        : m_notifier {notifier}
        , m_trackingLinkBase {std::move(trackingLinkBase)}
    {
    }

    void CustomerNoticeSubscriber::on(const logistics::domain::ShipmentRegistered& event)
    {
        std::string message {domain::CustomerWording::notice(domain::Milestone::Received, event.code())};
        if (!m_trackingLinkBase.empty())
        {
            message += ". Seguilo acá: " + m_trackingLinkBase + "?codigo=" + event.code();
        }
        announce(event, message);
    }

    void CustomerNoticeSubscriber::on(const logistics::domain::ShipmentDispatched& event)
    {
        announceMilestone(event, domain::Milestone::OnTheWay);
    }

    void CustomerNoticeSubscriber::on(const logistics::domain::ShipmentRescheduled& event)
    {
        const std::size_t delivered {deliveredParcels(event.state())};
        const std::size_t total {event.state().draft.parcels.size()};
        if (delivered > 0 && delivered < total)
        {
            announce(event, domain::CustomerWording::partialNotice(event.code(), delivered, total));
        }
        else
        {
            announceMilestone(event, domain::Milestone::RetryScheduled);
        }
    }

    void CustomerNoticeSubscriber::on(const logistics::domain::ShipmentDelivered& event)
    {
        announceMilestone(event, domain::Milestone::Delivered);
    }

    void CustomerNoticeSubscriber::announceMilestone(const logistics::domain::ShipmentEvent& event,
                                                     const domain::Milestone milestone)
    {
        announce(event, domain::CustomerWording::notice(milestone, event.code()));
    }

    void CustomerNoticeSubscriber::announce(const logistics::domain::ShipmentEvent& event, const std::string& message)
    {
        try
        {
            const logistics::domain::Phone phone {event.state().draft.recipientPhone};
            m_notifier.send(
                CustomerNotice {event.code(), CHANNEL, phone.masked(), message, event.movement().occurredAt});
        }
        catch (const std::exception& error)
        {
            std::cerr << "[hubflow] no se pudo avisar al cliente del paquete " << event.code() << ": " << error.what()
                      << '\n';
        }
    }

} // namespace hubflow::tracking::application
