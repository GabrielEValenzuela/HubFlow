#include "hubflow/shared/adapters/out/generation/uuidV7IdentityGenerator.hpp"

#include <array>
#include <chrono>

namespace hubflow::shared::adapters
{
namespace
{
constexpr std::uint32_t COUNTER_LIMIT {0x1000};
constexpr std::uint32_t COUNTER_START_RANGE {0x800};
constexpr std::size_t   UUID_BYTES {16};
constexpr const char*   HEX_DIGITS {"0123456789abcdef"};

std::uint64_t systemMilliseconds()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

std::string hexText(const std::array<std::uint8_t, UUID_BYTES>& bytes)
{
    std::string text;
    for (std::size_t i = 0; i < bytes.size(); ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
        {
            text.push_back('-');
        }
        text.push_back(HEX_DIGITS[bytes[i] >> 4]);
        text.push_back(HEX_DIGITS[bytes[i] & 0x0F]);
    }
    return text;
}
} // namespace

UuidV7IdentityGenerator::UuidV7IdentityGenerator()
    : UuidV7IdentityGenerator {MillisecondSource {systemMilliseconds}}
{
}

UuidV7IdentityGenerator::UuidV7IdentityGenerator(MillisecondSource source)
    : m_now {std::move(source)}
    , m_random {std::random_device {}()}
{
}

std::string UuidV7IdentityGenerator::next()
{
    const std::uint64_t now {m_now()};
    if (now > m_lastMillis)
    {
        m_lastMillis = now;
        m_counter    = static_cast<std::uint32_t>(m_random() % COUNTER_START_RANGE);
    }
    else
    {
        ++m_counter;
        if (m_counter >= COUNTER_LIMIT)
        {
            ++m_lastMillis;
            m_counter = static_cast<std::uint32_t>(m_random() % COUNTER_START_RANGE);
        }
    }
    std::array<std::uint8_t, UUID_BYTES> bytes {};
    for (std::size_t i = 0; i < 6; ++i)
    {
        bytes[i] = static_cast<std::uint8_t>(m_lastMillis >> (8 * (5 - i)));
    }
    bytes[6] = static_cast<std::uint8_t>(0x70 | (m_counter >> 8));
    bytes[7] = static_cast<std::uint8_t>(m_counter & 0xFF);
    const std::uint64_t tail {m_random()};
    bytes[8] = static_cast<std::uint8_t>(0x80 | ((tail >> 56) & 0x3F));
    for (std::size_t i = 9; i < UUID_BYTES; ++i)
    {
        bytes[i] = static_cast<std::uint8_t>(tail >> (8 * (UUID_BYTES - 1 - i)));
    }
    return hexText(bytes);
}

} // namespace hubflow::shared::adapters
