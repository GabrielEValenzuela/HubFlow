#ifndef HUBFLOW_SHARED_APPLICATION_VOCABULARY_HPP
#define HUBFLOW_SHARED_APPLICATION_VOCABULARY_HPP

#include "hubflow/identity/domain/valueobjects/role.hpp"
#include "hubflow/logistics/domain/valueobjects/shipmentStatus.hpp"
#include "hubflow/routing/domain/valueobjects/searchAlgorithm.hpp"
#include "hubflow/routing/domain/valueobjects/stopOutcome.hpp"
#include "hubflow/sorting/domain/valueobjects/sortCriteria.hpp"

#include <optional>
#include <string>

namespace hubflow::shared::application
{

/**
 * @brief Nombres canónicos del lenguaje ubicuo para los enumerados del dominio.
 *
 * El dominio no conoce representaciones textuales: esta traducción la comparten la auditoría,
 * la API REST y la persistencia. Sólo cubre los enumerados que son un conjunto cerrado (estados, roles, permisos,
 * resultados, algoritmos); las zonas, los servicios y las características son datos con su propio catálogo.
 */
class Vocabulary final
{
public:
    static std::string name(logistics::domain::ShipmentStatus status);
    static std::string name(identity::domain::Role role);
    static std::string name(identity::domain::Permission permission);
    static std::string name(routing::domain::SearchAlgorithm algorithm);
    static std::string name(routing::domain::StopOutcome outcome);
    static std::string name(routing::domain::RouteStatus status);
    static std::string name(sorting::domain::SortCriteria criteria);

    static std::optional<logistics::domain::ShipmentStatus> shipmentStatus(const std::string& text);
    static std::optional<identity::domain::Role>            role(const std::string& text);
    static std::optional<identity::domain::Permission>      permission(const std::string& text);
    static std::optional<routing::domain::SearchAlgorithm>  searchAlgorithm(const std::string& text);
    static std::optional<routing::domain::StopOutcome>      stopOutcome(const std::string& text);
    static std::optional<sorting::domain::SortCriteria>     sortCriteria(const std::string& text);
};

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_VOCABULARY_HPP
