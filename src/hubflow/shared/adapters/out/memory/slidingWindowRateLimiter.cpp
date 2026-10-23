#include "hubflow/shared/adapters/out/memory/slidingWindowRateLimiter.hpp"

namespace hubflow::shared::adapters
{
namespace
{
constexpr std::size_t PRUNE_THRESHOLD {1024};
} // namespace

SlidingWindowRateLimiter::SlidingWindowRateLimiter(const std::size_t maxAttempts, const std::int64_t windowSeconds)
    : m_maxAttempts {maxAttempts}
    , m_windowSeconds {windowSeconds}
{
}

application::RateDecision SlidingWindowRateLimiter::attempt(const std::string& key, const std::int64_t now)
{
    if (m_attempts.size() > PRUNE_THRESHOLD)
    {
        discardExpired(now);
    }
    auto& recent = m_attempts[key];
    while (!recent.empty() && recent.front() <= now - m_windowSeconds)
    {
        recent.pop_front();
    }
    application::RateDecision decision;
    if (recent.size() >= m_maxAttempts)
    {
        const std::int64_t wait    = recent.front() + m_windowSeconds - now;
        decision.allowed           = false;
        decision.retryAfterSeconds = static_cast<std::uint32_t>(wait > 0 ? wait : 1);
    }
    else
    {
        recent.push_back(now);
    }
    return decision;
}

void SlidingWindowRateLimiter::discardExpired(const std::int64_t now)
{
    for (auto it = m_attempts.begin(); it != m_attempts.end();)
    {
        if (it->second.empty() || it->second.back() <= now - m_windowSeconds)
        {
            it = m_attempts.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

} // namespace hubflow::shared::adapters
