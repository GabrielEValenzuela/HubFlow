#ifndef HUBFLOW_INSIGHTS_DOMAIN_SERVICES_CODE_SET_HPP
#define HUBFLOW_INSIGHTS_DOMAIN_SERVICES_CODE_SET_HPP

#include "datastructures/set.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"

#include <string>

namespace hubflow::insights::domain
{

/// @brief Conjunto de códigos de seguimiento sin repetidos.
using CodeSet = ds::Set<std::string>;

/// @brief Criterio de pertenencia a un conjunto de envíos.
class SetCriterion
{
public:
    virtual ~SetCriterion() = default;

    /// @brief true si @p shipment pertenece al conjunto.
    virtual bool matches(const logistics::domain::Shipment& shipment) const = 0;

    /// @brief Construye el conjunto recorriendo el catálogo. Mejor O(n), peor O(n²).
    CodeSet collect(const logistics::domain::ShipmentCatalog& catalog) const;
};

/// @brief Envíos de una zona (por su código de catálogo).
class ZoneCriterion final : public SetCriterion
{
public:
    explicit ZoneCriterion(std::string zone) noexcept;

private:
    bool matches(const logistics::domain::Shipment& shipment) const override;

    std::string m_zone;
};

/// @brief Envíos de un nivel de servicio (por su código de catálogo).
class ServiceCriterion final : public SetCriterion
{
public:
    explicit ServiceCriterion(std::string service) noexcept;

private:
    bool matches(const logistics::domain::Shipment& shipment) const override;

    std::string m_service;
};

/// @brief Operación entre conjuntos (Strategy). Todas son O(n + m).
class SetOperation
{
public:
    virtual ~SetOperation() = default;

    /// @brief Combina @p first con @p second y devuelve el conjunto resultante.
    virtual CodeSet apply(const CodeSet& first, const CodeSet& second) const = 0;
};

/// @brief A ∪ B.
class UnionOperation final : public SetOperation
{
private:
    CodeSet apply(const CodeSet& first, const CodeSet& second) const override;
};

/// @brief A ∩ B.
class IntersectionOperation final : public SetOperation
{
private:
    CodeSet apply(const CodeSet& first, const CodeSet& second) const override;
};

/// @brief A − B.
class DifferenceOperation final : public SetOperation
{
private:
    CodeSet apply(const CodeSet& first, const CodeSet& second) const override;
};

} // namespace hubflow::insights::domain

#endif // HUBFLOW_INSIGHTS_DOMAIN_SERVICES_CODE_SET_HPP
