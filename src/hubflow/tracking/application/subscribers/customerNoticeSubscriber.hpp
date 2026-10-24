#ifndef HUBFLOW_TRACKING_APPLICATION_SUBSCRIBERS_CUSTOMER_NOTICE_SUBSCRIBER_HPP
#define HUBFLOW_TRACKING_APPLICATION_SUBSCRIBERS_CUSTOMER_NOTICE_SUBSCRIBER_HPP

#include "hubflow/logistics/domain/events/shipmentEvents.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"
#include "hubflow/tracking/application/ports/out/customerNotifier.hpp"
#include "hubflow/tracking/domain/valueobjects/milestone.hpp"

#include <string>

namespace hubflow::tracking::application
{

    /**
     * @brief Avisa al cliente cuando su paquete ingresa, sale a reparto, no se pudo entregar (entera o parcialmente) o
     * se entrega.
     *
     * Los mensajes usan las mismas frases que la línea de tiempo de seguimiento. El aviso de ingreso incluye el enlace
     * para seguir el paquete. El contacto del aviso va enmascarado: el número completo nunca sale de la planta por esta
     * vía. Una falla del canal no frena la operación de la planta: se informa por la salida de error y el evento sigue
     * su camino.
     */
    class CustomerNoticeSubscriber final : public shared::domain::DomainEventVisitor
    {
    public:
        /// @param notifier canal por el que salen los avisos.
        /// @param trackingLinkBase dirección de la página de seguimiento (sin el código); vacía si no se quiere enlace.
        CustomerNoticeSubscriber(CustomerNotifier& notifier, std::string trackingLinkBase);

    private:
        void on(const logistics::domain::ShipmentRegistered& event) override;
        void on(const logistics::domain::ShipmentDispatched& event) override;
        void on(const logistics::domain::ShipmentRescheduled& event) override;
        void on(const logistics::domain::ShipmentDelivered& event) override;

        void announce(const logistics::domain::ShipmentEvent& event, const std::string& message);

        void announceMilestone(const logistics::domain::ShipmentEvent& event, domain::Milestone milestone);

        CustomerNotifier& m_notifier;
        std::string m_trackingLinkBase;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_SUBSCRIBERS_CUSTOMER_NOTICE_SUBSCRIBER_HPP
