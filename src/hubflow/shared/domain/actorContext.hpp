#ifndef HUBFLOW_SHARED_DOMAIN_ACTOR_CONTEXT_HPP
#define HUBFLOW_SHARED_DOMAIN_ACTOR_CONTEXT_HPP

#include <cstdint>
#include <string>

namespace hubflow::shared::domain
{

    /// @brief Quién ejecuta una operación de negocio y en qué instante (epoch en segundos).
    struct ActorContext
    {
        std::uint64_t userId {0};
        std::string actorName;
        std::int64_t occurredAt {0};
    };

} // namespace hubflow::shared::domain

#endif // HUBFLOW_SHARED_DOMAIN_ACTOR_CONTEXT_HPP
