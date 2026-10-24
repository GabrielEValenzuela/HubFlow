#include "hubflow/logistics/domain/valueobjects/uuidText.hpp"

namespace hubflow::logistics::domain
{
    namespace
    {
        constexpr std::size_t VERSION_POSITION {14};
        constexpr std::size_t VARIANT_POSITION {19};
        constexpr const char* VARIANT_DIGITS {"89ab"};

        bool isSeparatorPosition(const std::size_t position)
        {
            return position == 8 || position == 13 || position == 18 || position == 23;
        }

        bool isLowerHex(const char symbol)
        {
            return (symbol >= '0' && symbol <= '9') || (symbol >= 'a' && symbol <= 'f');
        }
    } // namespace

    bool UuidText::isValid(const std::string& value)
    {
        bool valid {value.size() == LENGTH};
        for (std::size_t i = 0; i < value.size() && valid; ++i)
        {
            valid = isSeparatorPosition(i) ? value[i] == '-' : isLowerHex(value[i]);
        }
        if (valid)
        {
            valid = value[VERSION_POSITION] == '7';
        }
        if (valid)
        {
            valid = std::string {VARIANT_DIGITS}.find(value[VARIANT_POSITION]) != std::string::npos;
        }
        return valid;
    }

} // namespace hubflow::logistics::domain
