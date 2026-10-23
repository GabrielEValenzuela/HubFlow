#include "support/fixtures.hpp"

namespace hubflow::test
{
namespace
{
using logistics::domain::ServiceLevel;
using logistics::domain::Zone;

struct EdgeData
{
    const char* from;
    const char* to;
    double      costKm;
};

const routing::domain::RoadNode CITY_NODES[] {
    {"HUB", "Centro de Distribución", true, Zone::Centro, 10.0, 10.0},
    {"C1", "Plaza Mayor", false, Zone::Centro, 7.0, 10.0},
    {"C2", "Mercado Central", false, Zone::Centro, 13.0, 11.0},
    {"C3", "Estación Norte", false, Zone::Centro, 10.0, 7.0},
    {"C4", "Barrio Universitario", false, Zone::Centro, 5.0, 7.0},
    {"N1", "Av. Libertador", false, Zone::Norte, 9.0, 5.0},
    {"N2", "Parque Norte", false, Zone::Norte, 13.0, 4.0},
    {"N3", "Villa Alta", false, Zone::Norte, 6.0, 2.0},
    {"N4", "Las Lomas", false, Zone::Norte, 11.0, 1.0},
    {"N5", "Puerto Norte", false, Zone::Norte, 16.0, 2.0},
    {"S1", "Av. del Sur", false, Zone::Sur, 9.0, 14.0},
    {"S2", "Barrio Industrial", false, Zone::Sur, 13.0, 15.0},
    {"S3", "Villa Sur", false, Zone::Sur, 6.0, 17.0},
    {"S4", "Costanera", false, Zone::Sur, 11.0, 18.0},
    {"S5", "Polo Sur", false, Zone::Sur, 16.0, 17.0},
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
        {"PKG-1001", "Ana Torres", Zone::Centro, 1.20, ServiceLevel::Estandar, "C1"},
        {"PKG-1002", "Bruno Díaz", Zone::Norte, 0.75, ServiceLevel::Express, "N2"},
        {"PKG-1003", "Carla Ruiz", Zone::Sur, 4.10, ServiceLevel::Prioritario, "S3"},
        {"PKG-1004", "Diego López", Zone::Centro, 2.30, ServiceLevel::Estandar, "C2"},
        {"PKG-1005", "Elena Castro", Zone::Norte, 1.90, ServiceLevel::Prioritario, "N3"},
        {"PKG-1006", "Franco Gómez", Zone::Sur, 0.50, ServiceLevel::Express, "S5"},
        {"PKG-1007", "Gabriela Soto", Zone::Centro, 6.20, ServiceLevel::Estandar, "C4"},
        {"PKG-1008", "Hugo Pérez", Zone::Norte, 3.40, ServiceLevel::Prioritario, "N5"},
    };
}

logistics::domain::ShipmentDraft
draft(const std::string& code, const ServiceLevel service, const Zone zone, const double weight)
{
    return logistics::domain::ShipmentDraft {code, "Cliente de prueba", zone, weight, service, "N1"};
}

void buildReferenceCity(routing::domain::RoadMap& map)
{
    for (const auto& node : CITY_NODES)
    {
        map.addNode(node);
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
