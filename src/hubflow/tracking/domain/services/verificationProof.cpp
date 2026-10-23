#include "hubflow/tracking/domain/services/verificationProof.hpp"

namespace hubflow::tracking::domain
{
namespace
{
bool sameText(const std::string& first, const std::string& second) noexcept
{
    unsigned          difference {static_cast<unsigned>(first.size() ^ second.size())};
    const std::size_t shared {first.size() < second.size() ? first.size() : second.size()};
    for (std::size_t i = 0; i < shared; ++i)
    {
        difference |=
            static_cast<unsigned>(static_cast<unsigned char>(first[i]) ^ static_cast<unsigned char>(second[i]));
    }
    return difference == 0;
}
} // namespace

bool ProofCheck::matches(const VerificationProof& proof,
                         const std::string&       expectedPhoneLast4,
                         const std::string&       expectedPostalCode)
{
    const bool phoneOk {proof.phoneLast4.empty() || sameText(proof.phoneLast4, expectedPhoneLast4)};
    const bool postalOk {proof.postalCode.empty() ||
                         sameText(normalizePostalCode(proof.postalCode), normalizePostalCode(expectedPostalCode))};
    return !proof.empty() && !expectedPhoneLast4.empty() && phoneOk && postalOk;
}

std::string ProofCheck::normalizePostalCode(const std::string& text)
{
    std::string value;
    for (const char character : text)
    {
        if (character != ' ')
        {
            value.push_back(character >= 'a' && character <= 'z' ? static_cast<char>(character - 'a' + 'A')
                                                                 : character);
        }
    }
    return value;
}

} // namespace hubflow::tracking::domain
