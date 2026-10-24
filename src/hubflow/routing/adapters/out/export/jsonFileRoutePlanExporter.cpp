#include "hubflow/routing/adapters/out/export/jsonFileRoutePlanExporter.hpp"

#include "hubflow/routing/adapters/out/export/routePlanJson.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace hubflow::routing::adapters
{
    namespace
    {
        constexpr int JSON_INDENT {2};

        void write(const std::filesystem::path& file, const nlohmann::json& document)
        {
            std::ofstream output {file, std::ios::trunc};
            output << document.dump(JSON_INDENT) << '\n';
            if (!output)
            {
                throw std::runtime_error("No se pudo escribir " + file.string());
            }
        }
    } // namespace

    JsonFileRoutePlanExporter::JsonFileRoutePlanExporter(std::string exportDirectory)
        : m_exportDirectory {std::move(exportDirectory)}
    {
    }

    void JsonFileRoutePlanExporter::exportPlan(const domain::RoadMap& map, const domain::DeliveryRoute& route)
    {
        const std::filesystem::path folder {std::filesystem::path {m_exportDirectory} /
                                            ("route-" + std::to_string(route.id()))};
        std::error_code failure;
        std::filesystem::create_directories(folder, failure);
        if (failure)
        {
            throw std::runtime_error("No se pudo crear " + folder.string() + ": " + failure.message());
        }
        write(folder / "initial_state.json", RoutePlanJson::initialState(map, route));
        write(folder / "sequence.json", RoutePlanJson::sequence(map, route));
    }

} // namespace hubflow::routing::adapters
