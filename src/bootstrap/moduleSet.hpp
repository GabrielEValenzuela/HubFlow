#ifndef HUBFLOW_BOOTSTRAP_MODULE_SET_HPP
#define HUBFLOW_BOOTSTRAP_MODULE_SET_HPP

#include "bootstrap/boundedContextModule.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace hubflow::bootstrap
{

    /**
     * @brief Lista ordenada de módulos de contexto y las operaciones de arranque que se aplican a todos.
     *
     * El orden de alta es el orden de hidratación, de carga de datos iniciales y de declaración de endpoints. Los
     * reactores de eventos se suscriben etapa por etapa (ver @ref SubscriptionPhase), no módulo por módulo.
     */
    class ModuleSet final
    {
    public:
        /// @brief Da de alta un módulo y devuelve una referencia para que otros módulos tomen sus puertos.
        template<typename TModule>
        TModule& add(std::unique_ptr<TModule> spModule)
        {
            TModule& module = *spModule;
            m_modules.push_back(std::move(spModule));
            return module;
        }

        /// @brief Suscribe todos los reactores al bus: etapa por etapa y, dentro de cada una, módulo por módulo.
        void subscribeAll(shared::application::EventBus& bus);

        /// @brief Hidrata cada módulo en el orden de alta.
        void hydrateAll();

        /// @brief Carga los datos iniciales de cada módulo en el orden de alta.
        void seedAll();

        /// @brief Declara los endpoints de cada módulo en el orden de alta.
        void registerAllRoutes(platform::ApiRouter& router);

    private:
        std::vector<std::unique_ptr<BoundedContextModule>> m_modules;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_MODULE_SET_HPP
