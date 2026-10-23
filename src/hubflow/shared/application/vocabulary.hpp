#ifndef HUBFLOW_SHARED_APPLICATION_VOCABULARY_HPP
#define HUBFLOW_SHARED_APPLICATION_VOCABULARY_HPP

#include "hubflow/identity/domain/valueobjects/role.hpp"
#include "hubflow/logistics/domain/valueobjects/serviceLevel.hpp"
#include "hubflow/logistics/domain/valueobjects/shipmentStatus.hpp"
#include "hubflow/logistics/domain/valueobjects/zone.hpp"
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
 * la API REST y la persistencia.
 */
class Vocabulary final
{
public:
    static std::string name(logistics::domain::Zone zone);
    static std::string name(logistics::domain::ServiceLevel service);
    static std::string name(logistics::domain::ShipmentStatus status);
    static std::string name(identity::domain::Role role);
    static std::string name(routing::domain::SearchAlgorithm algorithm);
    static std::string name(routing::domain::StopOutcome outcome);
    static std::string name(routing::domain::RouteStatus status);
    static std::string name(sorting::domain::SortCriteria criteria);

    static std::optional<logistics::domain::Zone>           zone(const std::string& text);
    static std::optional<logistics::domain::ServiceLevel>   serviceLevel(const std::string& text);
    static std::optional<logistics::domain::ShipmentStatus> shipmentStatus(const std::string& text);
    static std::optional<identity::domain::Role>            role(const std::string& text);
    static std::optional<routing::domain::SearchAlgorithm>  searchAlgorithm(const std::string& text);
    static std::optional<routing::domain::StopOutcome>      stopOutcome(const std::string& text);
    static std::optional<sorting::domain::SortCriteria>     sortCriteria(const std::string& text);
};

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_VOCABULARY_HPP
