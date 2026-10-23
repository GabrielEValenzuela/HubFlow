#include "hubflow/logistics/domain/valueobjects/publicCodeCheck.hpp"

namespace hubflow::logistics::domain
{

int PublicCodeCheck::valueOf(const char symbol)
{
    int value {-1};
    for (unsigned i = 0; i < SYMBOL_COUNT && value < 0; ++i)
    {
        value = SYMBOLS[i] == symbol ? static_cast<int>(i) : -1;
    }
    return value;
}

int PublicCodeCheck::remainderOf(const std::string& body)
{
    int remainder {body.size() == BODY_LENGTH ? 0 : -1};
    for (std::size_t i = 0; i < body.size() && remainder >= 0; ++i)
    {
        const int value {valueOf(body[i])};
        remainder = value < 0 ? -1 : (remainder + value * static_cast<int>(i + 1)) % static_cast<int>(MODULUS);
    }
    return remainder;
}

char PublicCodeCheck::checkSymbolOf(const std::string& body)
{
    const int remainder {remainderOf(body)};
    return remainder >= 0 && remainder < static_cast<int>(SYMBOL_COUNT) ? SYMBOLS[remainder] : '\0';
}

bool PublicCodeCheck::isConsistent(const std::string& body, const char check)
{
    const char expected {checkSymbolOf(body)};
    return expected != '\0' && expected == check;
}

} // namespace hubflow::logistics::domain
