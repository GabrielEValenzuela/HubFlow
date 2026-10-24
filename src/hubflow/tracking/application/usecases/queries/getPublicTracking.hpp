#ifndef HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_GET_PUBLIC_TRACKING_HPP
#define HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_GET_PUBLIC_TRACKING_HPP

#include "hubflow/tracking/application/ports/in/getPublicTrackingUseCase.hpp"
#include "hubflow/tracking/application/ports/out/trackedShipmentDirectory.hpp"

namespace hubflow::tracking::application
{

    /// @brief Implementación de GetPublicTrackingUseCase.
    class GetPublicTracking final : public GetPublicTrackingUseCase
    {
    public:
        explicit GetPublicTracking(const TrackedShipmentDirectory& directory);

    private:
        PublicTracking execute(const std::string& code) const override;

        const TrackedShipmentDirectory& m_directory;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_USECASES_QUERIES_GET_PUBLIC_TRACKING_HPP
