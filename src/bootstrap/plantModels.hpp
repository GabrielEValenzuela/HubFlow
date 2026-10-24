#ifndef HUBFLOW_BOOTSTRAP_PLANT_MODELS_HPP
#define HUBFLOW_BOOTSTRAP_PLANT_MODELS_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/sorting/domain/services/sortingDesk.hpp"

namespace hubflow::bootstrap
{

    /**
     * @brief Estado en memoria de la planta que varios contextos consultan a la vez.
     *
     * Logística, ruteo y mesa de clasificación se miran entre sí (por ejemplo, un envío necesita conocer los puntos
     * del mapa y una ruta necesita conocer los envíos). Para que ningún módulo dependa de otro módulo completo, estos
     * tres modelos se crean antes que todos y cada módulo recibe sólo el que necesita.
     */
    struct PlantModels
    {
        logistics::application::LogisticsModel logistics;
        routing::application::RoutingModel routing;
        sorting::domain::SortingDesk desk;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_PLANT_MODELS_HPP
