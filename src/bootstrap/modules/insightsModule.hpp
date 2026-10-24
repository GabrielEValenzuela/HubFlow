#ifndef HUBFLOW_BOOTSTRAP_MODULES_INSIGHTS_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_MODULES_INSIGHTS_MODULE_HPP

#include "bootstrap/boundedContextModule.hpp"

#include <memory>

namespace hubflow::logistics::application
{
struct LogisticsModel;
}

namespace hubflow::bootstrap
{

/**
 * @brief Arma el contexto de análisis: consultas sobre los índices de envíos, comparación de historiales y
 * operaciones entre conjuntos de códigos.
 *
 * Sólo lee el modelo de logística; no guarda nada ni reacciona a eventos.
 */
class InsightsModule final : public BoundedContextModule
{
public:
    explicit InsightsModule(const logistics::application::LogisticsModel& logistics);

    ~InsightsModule() override;

    InsightsModule(const InsightsModule&)            = delete;
    InsightsModule& operator=(const InsightsModule&) = delete;

private:
    void registerRoutes(platform::ApiRouter& router) override;

    struct Parts;

    std::unique_ptr<Parts> m_spParts;
};

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_MODULES_INSIGHTS_MODULE_HPP
