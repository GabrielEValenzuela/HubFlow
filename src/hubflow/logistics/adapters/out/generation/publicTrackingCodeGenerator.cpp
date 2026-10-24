#include "hubflow/logistics/adapters/out/generation/publicTrackingCodeGenerator.hpp"

#include "hubflow/logistics/domain/valueobjects/publicCodeCheck.hpp"
#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::adapters
{

    PublicTrackingCodeGenerator::PublicTrackingCodeGenerator()
        : m_random {std::random_device {}()}
    {
    }

    PublicTrackingCodeGenerator::PublicTrackingCodeGenerator(const std::uint64_t seed)
        : m_random {seed}
    {
    }

    std::string PublicTrackingCodeGenerator::next(const domain::ShipmentCatalog& catalog)
    {
        std::string code;
        for (unsigned attempt = 0; attempt < MAX_ATTEMPTS && code.empty(); ++attempt)
        {
            std::string body;
            for (std::size_t i = 0; i < domain::PublicCodeCheck::BODY_LENGTH; ++i)
            {
                body.push_back(domain::PublicCodeCheck::SYMBOLS[m_random() % domain::PublicCodeCheck::SYMBOL_COUNT]);
            }
            const char check {domain::PublicCodeCheck::checkSymbolOf(body)};
            if (check != '\0')
            {
                const std::string candidate {std::string {domain::TrackingCode::PUBLIC_PREFIX} + body + check};
                code = catalog.contains(candidate) ? std::string {} : candidate;
            }
        }
        if (code.empty())
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                  "tracking_code.exhausted",
                                                  "No se pudo generar un código de seguimiento libre"};
        }
        return code;
    }

} // namespace hubflow::logistics::adapters
