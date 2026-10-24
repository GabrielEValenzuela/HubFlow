#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_USER_REPOSITORY_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_USER_REPOSITORY_HPP

#include "hubflow/identity/domain/entities/user.hpp"

#include <optional>

namespace hubflow::identity::application
{

    /// @brief Receptor de usuarios al listar.
    class UserVisitor
    {
    public:
        virtual ~UserVisitor() = default;

        virtual void visit(const domain::UserState& state) = 0;
    };

    /**
     * @brief Repositorio de usuarios. A diferencia de los envíos y las rutas, los usuarios no se mantienen en
     * memoria: se leen bajo demanda desde el almacenamiento.
     */
    class UserRepository
    {
    public:
        virtual ~UserRepository() = default;

        /// @brief Identidad para un usuario nuevo.
        virtual std::uint64_t nextIdentity() = 0;

        virtual void add(const domain::UserState& state) = 0;

        virtual void update(const domain::UserState& state) = 0;

        virtual std::optional<domain::UserState> byId(std::uint64_t id) const = 0;

        virtual std::optional<domain::UserState> byEmail(const std::string& email) const = 0;

        virtual void forEach(UserVisitor& visitor) const = 0;

        virtual std::size_t count() const = 0;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_OUT_USER_REPOSITORY_HPP
