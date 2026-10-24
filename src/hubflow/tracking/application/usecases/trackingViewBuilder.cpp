#include "hubflow/tracking/application/usecases/trackingViewBuilder.hpp"

#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"
#include "hubflow/tracking/domain/services/customerTimeline.hpp"

namespace hubflow::tracking::application
{

    std::string TrackingViewBuilder::normalize(const std::string& code)
    {
        std::string value;
        for (const char character : code)
        {
            if (character != ' ')
            {
                value.push_back(character >= 'a' && character <= 'z' ? static_cast<char>(character - 'a' + 'A')
                                                                     : character);
            }
        }
        return value;
    }

    bool TrackingViewBuilder::looksLikeCode(const std::string& code)
    {
        return logistics::domain::TrackingCode::isValid(code);
    }

    PublicTracking TrackingViewBuilder::publicView(const TrackedShipment& shipment)
    {
        ds::DynamicArray<domain::MilestoneEvent> events;
        for (const auto& milestone : shipment.milestones)
        {
            events.pushBack(milestone);
        }
        const auto timeline = domain::CustomerTimeline::fromEvents(events);
        PublicTracking view;
        view.code = shipment.code;
        if (!timeline.isEmpty())
        {
            view.current = timeline.currentEntry();
            view.finished = timeline.isFinished();
        }
        for (std::size_t i = 0; i < timeline.size(); ++i)
        {
            view.timeline.push_back(timeline.entryAt(i));
        }
        return view;
    }

} // namespace hubflow::tracking::application
