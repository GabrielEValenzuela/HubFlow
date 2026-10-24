#include "support/applicationHarness.hpp"

#include "hubflow/routing/adapters/out/export/failureTolerantRoutePlanExporter.hpp"
#include "hubflow/routing/adapters/out/export/jsonFileRoutePlanExporter.hpp"
#include "hubflow/routing/adapters/out/export/routePlanJson.hpp"
#include "hubflow/routing/application/subscribers/routePlanExportSubscriber.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using nlohmann::json;

namespace
{
    class RecordingExporter final : public routing::application::RoutePlanExporter
    {
    public:
        std::vector<std::uint64_t> exported;

    private:
        void exportPlan(const routing::domain::RoadMap&, const routing::domain::DeliveryRoute& route) override
        {
            exported.push_back(route.id());
        }
    };

    class FailingExporter final : public routing::application::RoutePlanExporter
    {
    private:
        void exportPlan(const routing::domain::RoadMap&, const routing::domain::DeliveryRoute&) override
        {
            throw std::runtime_error {"disco lleno"};
        }
    };

    const routing::domain::DeliveryRoute& assignNorthRoute(test::ApplicationHarness& app)
    {
        const auto courierId =
            app.registerUser
                .execute(app.admin(),
                         identity::application::RegisterUserRequest {
                             "pum1@hubflow.local", "Repartidor", identity::domain::Role::Pum, "clave-segura"})
                .id;
        const auto& batch = app.openSortingBatch.execute(app.operador(), "NORTE");
        app.prepareSortingBatch.execute(app.operador(), batch.id());
        for (int i = 0; i < 3; ++i)
        {
            app.processSortingBatchItem.execute(app.operador(), batch.id(), {});
        }
        return app.assignDeliveryRoute.execute(
            app.operador(),
            routing::application::AssignRouteRequest {batch.id(), courierId, routing::domain::SearchAlgorithm::AStar});
    }

    json readJson(const std::filesystem::path& file)
    {
        std::ifstream input {file};
        return json::parse(input);
    }

    std::filesystem::path freshDirectory(const std::string& name)
    {
        const auto directory = std::filesystem::temp_directory_path() / name;
        std::filesystem::remove_all(directory);
        return directory;
    }
} // namespace

TEST(RoutePlanExport, AssigningARouteAsksTheExporterOnce)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    RecordingExporter exporter;
    routing::application::RoutePlanExportSubscriber subscriber {app.routing, exporter};
    app.bus.subscribe(subscriber);

    const auto routeId = assignNorthRoute(app).id();

    EXPECT_EQ(exporter.exported, std::vector<std::uint64_t> {routeId});
}

TEST(RoutePlanExport, InitialStateDescribesTheProblem)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto& route = assignNorthRoute(app);

    const json state = routing::adapters::RoutePlanJson::initialState(app.routing.map, route);

    EXPECT_EQ(state.at("routeId"), route.id());
    EXPECT_EQ(state.at("courier"), "Repartidor");
    EXPECT_EQ(state.at("algorithm"), "ASTAR");
    EXPECT_EQ(state.at("start"), "HUB");
    EXPECT_EQ(state.at("goals").size(), route.stops().size());
    EXPECT_EQ(state.at("nodes").size(), app.routing.map.nodeCount());
    const json& hub = state.at("nodes").at(0);
    for (const char* key : {"id", "label", "zone", "isHub", "x", "y"})
    {
        EXPECT_TRUE(hub.contains(key)) << key;
    }
    EXPECT_TRUE(hub.at("isHub").get<bool>());
    EXPECT_EQ(state.at("edges").size(), 28U);
    for (const char* key : {"from", "to", "costKm"})
    {
        EXPECT_TRUE(state.at("edges").at(0).contains(key)) << key;
    }
}

TEST(RoutePlanExport, SequenceGoesFromHubThroughEveryStopBackToHub)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    const auto& route = assignNorthRoute(app);

    const json sequence = routing::adapters::RoutePlanJson::sequence(app.routing.map, route);

    const json& segments = sequence.at("segments");
    ASSERT_EQ(segments.size(), route.stops().size() + 1);
    EXPECT_EQ(sequence.at("algorithm"), "ASTAR");
    EXPECT_EQ(segments.front().at("from"), "HUB");
    EXPECT_EQ(segments.back().at("to"), "HUB");
    double cost {0.0};
    for (std::size_t segmentIndex = 0; segmentIndex < segments.size(); ++segmentIndex)
    {
        const json& segment = segments.at(segmentIndex);
        EXPECT_EQ(segment.at("path").front(), segment.at("from"));
        EXPECT_EQ(segment.at("path").back(), segment.at("to"));
        if (segmentIndex + 1 < segments.size())
        {
            EXPECT_EQ(segment.at("to"), route.stops()[segmentIndex].nodeId);
            EXPECT_EQ(segment.at("to"), segments.at(segmentIndex + 1).at("from"));
        }
        const json& step = segment.at("steps").at(0);
        for (const char* key : {"step", "expanded", "frontier", "explored"})
        {
            EXPECT_TRUE(step.contains(key)) << key;
        }
        for (const char* key :
             {"solution_found", "nodes_explored", "states_visited", "nodes_in_frontier", "max_depth", "cost_total"})
        {
            EXPECT_TRUE(segment.at("metrics").contains(key)) << key;
        }
        EXPECT_TRUE(segment.at("metrics").at("solution_found").get<bool>());
        cost += segment.at("metrics").at("cost_total").get<double>();
    }
    EXPECT_EQ(sequence.at("path").front(), "HUB");
    EXPECT_EQ(sequence.at("path").back(), "HUB");
    EXPECT_NEAR(sequence.at("metrics").at("cost_total").get<double>(), cost, 1e-9);
    EXPECT_EQ(sequence.at("metrics").at("nodes_explored").get<std::size_t>(), route.totalNodesExplored());
}

TEST(RoutePlanExport, FileExporterWritesBothDocumentsPerRoute)
{
    const auto directory = freshDirectory("hubflow-route-export-test");
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    routing::adapters::JsonFileRoutePlanExporter exporter {directory.string()};
    routing::application::RoutePlanExportSubscriber subscriber {app.routing, exporter};
    app.bus.subscribe(subscriber);

    const auto routeId = assignNorthRoute(app).id();

    const auto folder = directory / ("route-" + std::to_string(routeId));
    EXPECT_EQ(readJson(folder / "initial_state.json").at("routeId"), routeId);
    EXPECT_EQ(readJson(folder / "sequence.json").at("segments").size(), 4U);
    std::filesystem::remove_all(directory);
}

TEST(RoutePlanExport, ExportFailureDoesNotFailRouteAssignment)
{
    test::Storage storage;
    test::ApplicationHarness app {storage};
    app.seedDataset();
    FailingExporter failing;
    routing::adapters::FailureTolerantRoutePlanExporter tolerant {failing};
    routing::application::RoutePlanExportSubscriber subscriber {app.routing, tolerant};
    app.bus.subscribe(subscriber);

    const auto& route = assignNorthRoute(app);

    EXPECT_EQ(route.stops().size(), 3U);
    EXPECT_EQ(storage.routes.records.size(), 1U);
}
