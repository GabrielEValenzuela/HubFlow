#include "hubflow/shared/application/vocabulary.hpp"

#include <cstddef>

namespace hubflow::shared::application
{
    namespace
    {
        using identity::domain::Role;
        using logistics::domain::ShipmentStatus;
        using routing::domain::RouteStatus;
        using routing::domain::SearchAlgorithm;
        using routing::domain::StopOutcome;
        using sorting::domain::SortCriteria;

        template<typename TEnum>
        struct Term
        {
            TEnum value;
            const char* text;
        };

        constexpr Term<logistics::domain::ShipmentStatus> STATUS_TERMS[] {
            {logistics::domain::ShipmentStatus::Recibido, "RECIBIDO"},
            {logistics::domain::ShipmentStatus::Clasificado, "CLASIFICADO"},
            {logistics::domain::ShipmentStatus::EnReparto, "EN_REPARTO"},
            {logistics::domain::ShipmentStatus::Reprogramado, "REPROGRAMADO"},
            {logistics::domain::ShipmentStatus::Entregado, "ENTREGADO"}};

        constexpr Term<identity::domain::Role> ROLE_TERMS[] {{identity::domain::Role::Admin, "ADMIN"},
                                                             {identity::domain::Role::Operador, "OPERADOR"},
                                                             {identity::domain::Role::Pum, "PUM"}};

        constexpr Term<identity::domain::Permission> PERMISSION_TERMS[] {
            {identity::domain::Permission::ManageUsers, "MANAGE_USERS"},
            {identity::domain::Permission::OperateShipments, "OPERATE_SHIPMENTS"},
            {identity::domain::Permission::OperateSortingDesk, "OPERATE_SORTING_DESK"},
            {identity::domain::Permission::AssignRoutes, "ASSIGN_ROUTES"},
            {identity::domain::Permission::ResolveStops, "RESOLVE_STOPS"},
            {identity::domain::Permission::ViewIndexes, "VIEW_INDEXES"},
            {identity::domain::Permission::ViewAudit, "VIEW_AUDIT"},
            {identity::domain::Permission::ManageCatalogs, "MANAGE_CATALOGS"}};

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

        template<typename TEnum, std::size_t TERM_COUNT>
        std::string textOf(const Term<TEnum> (&terms)[TERM_COUNT], const TEnum value)
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

        template<typename TEnum, std::size_t TERM_COUNT>
        std::optional<TEnum> valueOf(const Term<TEnum> (&terms)[TERM_COUNT], const std::string& text)
        {
            std::optional<TEnum> value;
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

    std::string Vocabulary::name(const logistics::domain::ShipmentStatus status)
    {
        return textOf(STATUS_TERMS, status);
    }

    std::string Vocabulary::name(const identity::domain::Role role)
    {
        return textOf(ROLE_TERMS, role);
    }

    std::string Vocabulary::name(const identity::domain::Permission permission)
    {
        return textOf(PERMISSION_TERMS, permission);
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

    std::optional<logistics::domain::ShipmentStatus> Vocabulary::shipmentStatus(const std::string& text)
    {
        return valueOf(STATUS_TERMS, text);
    }

    std::optional<identity::domain::Role> Vocabulary::role(const std::string& text)
    {
        return valueOf(ROLE_TERMS, text);
    }

    std::optional<identity::domain::Permission> Vocabulary::permission(const std::string& text)
    {
        return valueOf(PERMISSION_TERMS, text);
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
