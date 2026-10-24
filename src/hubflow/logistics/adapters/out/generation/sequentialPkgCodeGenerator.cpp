#include "hubflow/logistics/adapters/out/generation/sequentialPkgCodeGenerator.hpp"

#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <string>

namespace hubflow::logistics::adapters
{
    namespace
    {
        constexpr std::size_t DIGITS {4};

        std::string codeFor(const unsigned number)
        {
            std::string digits {std::to_string(number)};
            return std::string {domain::TrackingCode::SEQUENTIAL_PREFIX} + std::string(DIGITS - digits.size(), '0') +
                   digits;
        }
    } // namespace

    SequentialPkgCodeGenerator::SequentialPkgCodeGenerator(const unsigned firstNumber)
        : m_next {firstNumber}
    {
    }

    std::string SequentialPkgCodeGenerator::next(const domain::ShipmentCatalog& catalog)
    {
        std::string code;
        while (code.empty() && m_next <= LAST_NUMBER)
        {
            const std::string candidate {codeFor(m_next)};
            ++m_next;
            code = catalog.contains(candidate) ? std::string {} : candidate;
        }
        if (code.empty())
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                  "tracking_code.exhausted",
                                                  "Se agotaron los códigos de seguimiento correlativos"};
        }
        return code;
    }

} // namespace hubflow::logistics::adapters
