#include "hubflow/shared/application/vocabulary.hpp"

#include <cstddef>

namespace hubflow::shared::application
{
namespace
{
using identity::domain::Role;
using logistics::domain::ServiceLevel;
using logistics::domain::ShipmentStatus;
using logistics::domain::Zone;
using routing::domain::RouteStatus;
using routing::domain::SearchAlgorithm;
using routing::domain::StopOutcome;
using sorting::domain::SortCriteria;

template <typename Enum>
struct Term
{
    Enum        value;
    const char* text;
};

constexpr Term<logistics::domain::Zone> ZONE_TERMS[] {{logistics::domain::Zone::Centro, "CENTRO"},
                                                      {logistics::domain::Zone::Norte, "NORTE"},
                                                      {logistics::domain::Zone::Sur, "SUR"}};

constexpr Term<logistics::domain::ServiceLevel> SERVICE_TERMS[] {
    {logistics::domain::ServiceLevel::Express, "EXPRESS"},
    {logistics::domain::ServiceLevel::Prioritario, "PRIORITARIO"},
    {logistics::domain::ServiceLevel::Estandar, "ESTANDAR"}};

constexpr Term<logistics::domain::ShipmentStatus> STATUS_TERMS[] {
    {logistics::domain::ShipmentStatus::Recibido, "RECIBIDO"},
    {logistics::domain::ShipmentStatus::Clasificado, "CLASIFICADO"},
    {logistics::domain::ShipmentStatus::EnReparto, "EN_REPARTO"},
    {logistics::domain::ShipmentStatus::Reprogramado, "REPROGRAMADO"},
    {logistics::domain::ShipmentStatus::Entregado, "ENTREGADO"}};

constexpr Term<identity::domain::Role> ROLE_TERMS[] {{identity::domain::Role::Admin, "ADMIN"},
                                                     {identity::domain::Role::Operador, "OPERADOR"},
                                                     {identity::domain::Role::Pum, "PUM"}};

constexpr Term<routing::domain::SearchAlgorithm> ALGORITHM_TERMS[] {
    {routing::domain::SearchAlgorithm::BreadthFirst, "BFS"},
    {routing::domain::SearchAlgorithm::DepthFirst, "DFS"},
    {routing::domain::SearchAlgorithm::GreedyBestFirst, "GREEDY"},
    {routing::domain::SearchAlgorithm::AStar, "ASTAR"}};

constexpr Term<routing::domain::StopOutcome> OUTCOME_TERMS[] {
    {routing::domain::StopOutcome::Pending, "PENDIENTE"},
    {routing::domain::StopOutcome::Delivered, "ENTREGADO"},
    {routing::domain::StopOutcome::Rescheduled, "REPROGRAMADO"}};

constexpr Term<routing::domain::RouteStatus> ROUTE_STATUS_TERMS[] {
    {routing::domain::RouteStatus::InProgress, "EN_CURSO"},
    {routing::domain::RouteStatus::Completed, "COMPLETADA"}};

constexpr Term<sorting::domain::SortCriteria> CRITERIA_TERMS[] {
    {sorting::domain::SortCriteria::None, "NONE"},
    {sorting::domain::SortCriteria::CodeAscending, "CODE_ASC"},
    {sorting::domain::SortCriteria::WeightDescending, "WEIGHT_DESC"}};

template <typename Enum, std::size_t N>
std::string textOf(const Term<Enum> (&terms)[N], const Enum value)
{
    std::string text {terms[0].text};
    for (const auto& term : terms)
    {
        if (term.value == value)
        {
            text = term.text;
        }
    }
    return text;
}

template <typename Enum, std::size_t N>
std::optional<Enum> valueOf(const Term<Enum> (&terms)[N], const std::string& text)
{
    std::optional<Enum> value;
    for (const auto& term : terms)
    {
        if (text == term.text)
        {
            value = term.value;
        }
    }
    return value;
}
} // namespace

std::string Vocabulary::name(const logistics::domain::Zone zone)
{
    return textOf(ZONE_TERMS, zone);
}

std::string Vocabulary::name(const logistics::domain::ServiceLevel service)
{
    return textOf(SERVICE_TERMS, service);
}

std::string Vocabulary::name(const logistics::domain::ShipmentStatus status)
{
    return textOf(STATUS_TERMS, status);
}

std::string Vocabulary::name(const identity::domain::Role role)
{
    return textOf(ROLE_TERMS, role);
}

std::string Vocabulary::name(const routing::domain::SearchAlgorithm algorithm)
{
    return textOf(ALGORITHM_TERMS, algorithm);
}

std::string Vocabulary::name(const routing::domain::StopOutcome outcome)
{
    return textOf(OUTCOME_TERMS, outcome);
}

std::string Vocabulary::name(const routing::domain::RouteStatus status)
{
    return textOf(ROUTE_STATUS_TERMS, status);
}

std::string Vocabulary::name(const sorting::domain::SortCriteria criteria)
{
    return textOf(CRITERIA_TERMS, criteria);
}

std::optional<logistics::domain::Zone> Vocabulary::zone(const std::string& text)
{
    return valueOf(ZONE_TERMS, text);
}

std::optional<logistics::domain::ServiceLevel> Vocabulary::serviceLevel(const std::string& text)
{
    return valueOf(SERVICE_TERMS, text);
}

std::optional<logistics::domain::ShipmentStatus> Vocabulary::shipmentStatus(const std::string& text)
{
    return valueOf(STATUS_TERMS, text);
}

std::optional<identity::domain::Role> Vocabulary::role(const std::string& text)
{
    return valueOf(ROLE_TERMS, text);
}

std::optional<routing::domain::SearchAlgorithm> Vocabulary::searchAlgorithm(const std::string& text)
{
    return valueOf(ALGORITHM_TERMS, text);
}

std::optional<routing::domain::StopOutcome> Vocabulary::stopOutcome(const std::string& text)
{
    return valueOf(OUTCOME_TERMS, text);
}

std::optional<sorting::domain::SortCriteria> Vocabulary::sortCriteria(const std::string& text)
{
    return valueOf(CRITERIA_TERMS, text);
}

} // namespace hubflow::shared::application
