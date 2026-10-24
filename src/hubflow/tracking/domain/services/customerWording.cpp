#include "hubflow/tracking/domain/services/customerWording.hpp"

namespace hubflow::tracking::domain
{
    namespace
    {
        struct Wording
        {
            Milestone milestone;
            const char* sentence;
            const char* stage;
        };

        constexpr Wording WORDINGS[] {
            {Milestone::Received, "Recibimos tu paquete en el centro de distribución", "RECEIVED"},
            {Milestone::Prepared, "Tu paquete fue preparado para el reparto", "PREPARED"},
            {Milestone::OnTheWay, "Está en camino", "ON_THE_WAY"},
            {Milestone::RetryScheduled, "No pudimos entregarlo, vamos a intentar de nuevo", "RETRY_SCHEDULED"},
            {Milestone::Delivered, "Entregado", "DELIVERED"}};

        const Wording& wordingOf(const Milestone milestone) noexcept
        {
            const Wording* pFound {&WORDINGS[0]};
            for (const auto& wording : WORDINGS)
            {
                if (wording.milestone == milestone)
                {
                    pFound = &wording;
                }
            }
            return *pFound;
        }
    } // namespace

    const char* CustomerWording::sentence(const Milestone milestone) noexcept
    {
        return wordingOf(milestone).sentence;
    }

    const char* CustomerWording::stage(const Milestone milestone) noexcept
    {
        return wordingOf(milestone).stage;
    }

    std::string CustomerWording::notice(const Milestone milestone, const std::string& code)
    {
        return "Paquete " + code + ": " + sentence(milestone);
    }

    std::string
    CustomerWording::partialNotice(const std::string& code, const std::size_t delivered, const std::size_t total)
    {
        return "Paquete " + code + ": entregamos " + std::to_string(delivered) + " de " + std::to_string(total) +
               " bultos. El resto lo llevamos en el próximo intento";
    }

} // namespace hubflow::tracking::domain
