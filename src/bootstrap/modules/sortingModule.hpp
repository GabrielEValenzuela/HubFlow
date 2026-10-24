#ifndef HUBFLOW_BOOTSTRAP_MODULES_SORTING_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_MODULES_SORTING_MODULE_HPP

#include "bootstrap/boundedContextModule.hpp"

#include <memory>

namespace hubflow::logistics::application
{
    struct LogisticsModel;
}

namespace hubflow::sorting::domain
{
    class SortingDesk;
}

namespace hubflow::bootstrap
{

    /**
     * @brief Arma el contexto de la mesa de clasificación: lotes, ordenamiento, cola de preparación y búsquedas.
     *
     * Trabaja sobre la mesa en memoria y consulta los envíos de logística; no guarda nada en la base de datos ni
     * reacciona a eventos.
     */
    class SortingModule final : public BoundedContextModule
    {
    public:
        SortingModule(sorting::domain::SortingDesk& desk, const logistics::application::LogisticsModel& logistics);

        ~SortingModule() override;

        SortingModule(const SortingModule&) = delete;
        SortingModule& operator=(const SortingModule&) = delete;

    private:
        void registerRoutes(platform::ApiRouter& router) override;

        struct Parts;

        std::unique_ptr<Parts> m_spParts;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_MODULES_SORTING_MODULE_HPP
