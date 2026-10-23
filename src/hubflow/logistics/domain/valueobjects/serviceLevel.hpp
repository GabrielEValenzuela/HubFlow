#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SERVICE_LEVEL_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SERVICE_LEVEL_HPP

namespace hubflow::logistics::domain
{

/// @brief Nivel de servicio contratado. Prioridad: EXPRESS > PRIORITARIO > ESTANDAR.
enum class ServiceLevel
{
    Express,
    Prioritario,
    Estandar
};

inline constexpr ServiceLevel ALL_SERVICE_LEVELS[] {ServiceLevel::Express,
                                                    ServiceLevel::Prioritario,
                                                    ServiceLevel::Estandar};

/// @brief Prioridad de despacho: un valor mayor se despacha antes.
unsigned priorityOf(ServiceLevel level) noexcept;

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SERVICE_LEVEL_HPP
