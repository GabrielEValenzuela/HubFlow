#include "hubflow/logistics/domain/valueobjects/phone.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
    namespace
    {
        constexpr const char* SEPARATORS {" -()."};

        bool isDigit(const char character)
        {
            return character >= '0' && character <= '9';
        }

        bool isSeparator(const char character)
        {
            return std::string {SEPARATORS}.find(character) != std::string::npos;
        }

        bool extractDigits(const std::string& text, std::string& digits)
        {
            bool valid {true};
            for (std::size_t i = 0; i < text.size() && valid; ++i)
            {
                if (isDigit(text[i]))
                {
                    digits.push_back(text[i]);
                }
                else
                {
                    valid = isSeparator(text[i]) || (i == 0 && text[i] == '+');
                }
            }
            return valid && digits.size() >= Phone::MIN_DIGITS && digits.size() <= Phone::MAX_DIGITS;
        }
    } // namespace

    Phone::Phone(const std::string& text)
    {
        if (!extractDigits(text, m_digits))
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "recipient.phone_invalid",
                                                  "El teléfono debe tener entre 8 y 15 dígitos"};
        }
    }

    bool Phone::accepts(const std::string& text)
    {
        std::string digits;
        return extractDigits(text, digits);
    }

    bool Phone::isProof(const std::string& text)
    {
        bool valid {text.size() == PROOF_DIGITS};
        for (std::size_t i = 0; i < text.size() && valid; ++i)
        {
            valid = isDigit(text[i]);
        }
        return valid;
    }

    const std::string& Phone::digits() const noexcept
    {
        return m_digits;
    }

    std::string Phone::lastFour() const
    {
        return m_digits.substr(m_digits.size() - PROOF_DIGITS);
    }

    std::string Phone::masked() const
    {
        return "****" + lastFour();
    }

} // namespace hubflow::logistics::domain
