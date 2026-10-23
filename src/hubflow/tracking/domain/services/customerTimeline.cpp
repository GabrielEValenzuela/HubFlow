#include "hubflow/tracking/domain/services/customerTimeline.hpp"

#include "hubflow/tracking/domain/services/customerWording.hpp"

namespace hubflow::tracking::domain
{

CustomerTimeline CustomerTimeline::from(const ds::DynamicArray<MilestoneEvent>& events)
{
    CustomerTimeline timeline;
    for (std::size_t i = 0; i < events.size(); ++i)
    {
        timeline.m_entries.pushBack(
            TimelineEntry {events[i].milestone, CustomerWording::sentence(events[i].milestone), events[i].occurredAt});
    }
    return timeline;
}

std::size_t CustomerTimeline::size() const noexcept
{
    return m_entries.size();
}

bool CustomerTimeline::empty() const noexcept
{
    return m_entries.empty();
}

const TimelineEntry& CustomerTimeline::entry(const std::size_t index) const
{
    return m_entries[index];
}

const TimelineEntry& CustomerTimeline::current() const
{
    return m_entries[m_entries.size() - 1];
}

bool CustomerTimeline::finished() const noexcept
{
    return !m_entries.empty() && m_entries[m_entries.size() - 1].milestone == Milestone::Delivered;
}

} // namespace hubflow::tracking::domain
