#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_REQUIRE_COURIER_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_REQUIRE_COURIER_HPP

#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/routing/domain/entities/deliveryRoute.hpp"

namespace hubflow::routing::application
{

    /**
     * @brief Busca al repartidor que va a llevar una ruta.
     * @throws BusinessError (Validation) si el usuario no existe, está inactivo o no es un PUM.
     */
    domain::Courier requireCourier(const identity::application::UserRepository& users, std::uint64_t courierId);

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_REQUIRE_COURIER_HPP
