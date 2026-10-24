#ifndef HUBFLOW_BOOTSTRAP_SHARED_SERVICES_HPP
#define HUBFLOW_BOOTSTRAP_SHARED_SERVICES_HPP

namespace hubflow::platform
{
    struct Settings;
    class MySqlConnection;
} // namespace hubflow::platform

namespace hubflow::shared::application
{
    class Clock;
    class EventBus;
    class IdentityGenerator;
} // namespace hubflow::shared::application

namespace hubflow::identity::application
{
    class TokenIssuer;
}

namespace hubflow::identity::domain
{
    class PasswordHasher;
}

namespace hubflow::bootstrap
{

    /**
     * @brief Piezas técnicas que todos los contextos usan y que ninguno posee.
     *
     * Son sólo referencias: la vida de cada pieza la maneja el módulo de infraestructura, que siempre se crea primero
     * y se destruye último.
     */
    struct SharedServices
    {
        // cppcheck-suppress-begin uninitMemberVarNoCtor
        const platform::Settings& settings;
        platform::MySqlConnection& connection;
        const shared::application::Clock& clock;
        shared::application::EventBus& bus;
        shared::application::IdentityGenerator& identities;
        const identity::domain::PasswordHasher& hasher;
        const identity::application::TokenIssuer& tokens;
        // cppcheck-suppress-end uninitMemberVarNoCtor
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_SHARED_SERVICES_HPP
