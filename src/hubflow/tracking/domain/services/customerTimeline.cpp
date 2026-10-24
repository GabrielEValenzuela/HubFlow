#include "hubflow/tracking/domain/services/customerTimeline.hpp"

#include "hubflow/tracking/domain/services/customerWording.hpp"

namespace hubflow::tracking::domain
{

    CustomerTimeline CustomerTimeline::fromEvents(const ds::DynamicArray<MilestoneEvent>& events)
    {
        CustomerTimeline timeline;
        for (std::size_t eventIndex = 0; eventIndex < events.size(); ++eventIndex)
        {
            timeline.m_entries.pushBack(TimelineEntry {events[eventIndex].milestone,
                                                       CustomerWording::sentence(events[eventIndex].milestone),
                                                       events[eventIndex].occurredAt});
        }
        return timeline;
    }

    std::size_t CustomerTimeline::size() const noexcept
    {
        return m_entries.size();
    }

    bool CustomerTimeline::isEmpty() const noexcept
    {
        return m_entries.isEmpty();
    }

    const TimelineEntry& CustomerTimeline::entryAt(const std::size_t index) const
    {
        return m_entries[index];
    }

    const TimelineEntry& CustomerTimeline::currentEntry() const
    {
        return m_entries[m_entries.size() - 1];
    }

    bool CustomerTimeline::isFinished() const noexcept
    {
        return !m_entries.isEmpty() && m_entries[m_entries.size() - 1].milestone == Milestone::Delivered;
    }

} // namespace hubflow::tracking::domain
