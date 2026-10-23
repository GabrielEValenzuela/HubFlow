#include "support/fixtures.hpp"

namespace hubflow::test
{
namespace
{

struct EdgeData
{
    const char* from;
    const char* to;
    double      costKm;
};

const routing::domain::RoadNode CITY_NODES[] {
    {"HUB", "Centro de Distribución", true, "CENTRO", 10.0, 10.0},
    {"C1", "Plaza Mayor", false, "CENTRO", 7.0, 10.0},
    {"C2", "Mercado Central", false, "CENTRO", 13.0, 11.0},
    {"C3", "Estación Norte", false, "CENTRO", 10.0, 7.0},
    {"C4", "Barrio Universitario", false, "CENTRO", 5.0, 7.0},
    {"N1", "Av. Libertador", false, "NORTE", 9.0, 5.0},
    {"N2", "Parque Norte", false, "NORTE", 13.0, 4.0},
    {"N3", "Villa Alta", false, "NORTE", 6.0, 2.0},
    {"N4", "Las Lomas", false, "NORTE", 11.0, 1.0},
    {"N5", "Puerto Norte", false, "NORTE", 16.0, 2.0},
    {"S1", "Av. del Sur", false, "SUR", 9.0, 14.0},
    {"S2", "Barrio Industrial", false, "SUR", 13.0, 15.0},
    {"S3", "Villa Sur", false, "SUR", 6.0, 17.0},
    {"S4", "Costanera", false, "SUR", 11.0, 18.0},
    {"S5", "Polo Sur", false, "SUR", 16.0, 17.0},
};

constexpr EdgeData CITY_EDGES[] {
    {"HUB", "C1", 3.0}, {"HUB", "C2", 3.7}, {"HUB", "C3", 3.9}, {"HUB", "S1", 6.0}, {"C1", "C3", 4.3},
    {"C2", "C3", 5.8},  {"C1", "C4", 4.7},  {"C3", "N1", 3.3},  {"C4", "N3", 5.1},  {"C4", "N1", 5.2},
    {"N1", "N3", 5.6},  {"N1", "N2", 6.0},  {"N1", "N4", 4.5},  {"N3", "N4", 5.9},  {"N2", "N4", 4.7},
    {"N2", "N5", 5.3},  {"N4", "N5", 5.1},  {"C2", "N2", 8.1},  {"C2", "S2", 5.2},  {"C1", "S1", 6.5},
    {"S1", "S2", 4.2},  {"S1", "S3", 4.9},  {"S1", "S4", 5.9},  {"S2", "S4", 5.3},  {"S2", "S5", 3.7},
    {"S3", "S4", 5.9},  {"S4", "S5", 6.7},  {"C1", "S3", 10.3},
};
} // namespace

std::vector<logistics::domain::ShipmentDraft> initialDataset()
{
    return {
        {"PKG-1001", "Ana Torres", "CENTRO", 1.20, "ESTANDAR", "C1"},
        {"PKG-1002", "Bruno Díaz", "NORTE", 0.75, "EXPRESS", "N2"},
        {"PKG-1003", "Carla Ruiz", "SUR", 4.10, "PRIORITARIO", "S3"},
        {"PKG-1004", "Diego López", "CENTRO", 2.30, "ESTANDAR", "C2"},
        {"PKG-1005", "Elena Castro", "NORTE", 1.90, "PRIORITARIO", "N3"},
        {"PKG-1006", "Franco Gómez", "SUR", 0.50, "EXPRESS", "S5"},
        {"PKG-1007", "Gabriela Soto", "CENTRO", 6.20, "ESTANDAR", "C4"},
        {"PKG-1008", "Hugo Pérez", "NORTE", 3.40, "PRIORITARIO", "N5"},
    };
}

logistics::domain::ShipmentDraft
draft(const std::string& code, const std::string& service, const std::string& zone, const double weight)
{
    return logistics::domain::ShipmentDraft {code, "Cliente de prueba", zone, weight, service, "N1"};
}

void seedZones(logistics::domain::ZoneCatalog& zones)
{
    zones.adopt(logistics::domain::Zone::restore({"CENTRO", "Centro", true}));
    zones.adopt(logistics::domain::Zone::restore({"NORTE", "Norte", true}));
    zones.adopt(logistics::domain::Zone::restore({"SUR", "Sur", true}));
}

void seedServiceLevels(logistics::domain::ServiceLevelCatalog& services)
{
    services.adopt(logistics::domain::ServiceLevel::restore({"EXPRESS", "Express", 3, true}));
    services.adopt(logistics::domain::ServiceLevel::restore({"PRIORITARIO", "Prioritario", 2, true}));
    services.adopt(logistics::domain::ServiceLevel::restore({"ESTANDAR", "Estándar", 1, true}));
}

ReferencePriorities::ReferencePriorities()
    : services {logistics::domain::CatalogNouns {"service_level", "el servicio", "un servicio"}}
    , policy {services}
{
    seedServiceLevels(services);
}

void seedFeatures(logistics::domain::FeatureCatalog& features)
{
    features.adopt(logistics::domain::ShipmentFeature::restore({"FRAGIL", "Frágil", true}));
    features.adopt(logistics::domain::ShipmentFeature::restore({"REFRIGERADO", "Refrigerado", true}));
    features.adopt(logistics::domain::ShipmentFeature::restore({"VOLUMINOSO", "Voluminoso", true}));
}

void grantDefaultPermissions(identity::domain::AccessPolicy& policy)
{
    using identity::domain::Permission;
    using identity::domain::Role;
    for (const auto permission : {Permission::ManageUsers,
                                  Permission::OperateShipments,
                                  Permission::OperateSortingDesk,
                                  Permission::AssignRoutes,
                                  Permission::ResolveStops,
                                  Permission::ViewIndexes,
                                  Permission::ViewAudit,
                                  Permission::ManageCatalogs})
    {
        policy.grant(Role::Admin, permission);
    }
    for (const auto permission : {Permission::OperateShipments,
                                  Permission::OperateSortingDesk,
                                  Permission::AssignRoutes,
                                  Permission::ResolveStops,
                                  Permission::ViewIndexes})
    {
        policy.grant(Role::Operador, permission);
    }
    policy.grant(Role::Pum, Permission::ResolveStops);
}

void buildReferenceCity(routing::domain::RoadMap&                               map,
                        const std::map<std::string, routing::domain::RoadNode>& overrides)
{
    for (const auto& node : CITY_NODES)
    {
        const auto found = overrides.find(node.id);
        map.addNode(found == overrides.end() ? node : found->second);
    }
    for (const auto& edge : CITY_EDGES)
    {
        map.connect(edge.from, edge.to, edge.costKm);
    }
}

shared::domain::ActorContext actor(const std::int64_t at)
{
    return shared::domain::ActorContext {1, "Operador de prueba", at};
}

} // namespace hubflow::test
