#include "bootstrap/modules/insightsModule.hpp"

#include "hubflow/insights/adapters/in/http/insightController.hpp"
#include "hubflow/insights/application/usecases/queries/combineCodeSets.hpp"
#include "hubflow/insights/application/usecases/queries/compareShipmentStates.hpp"
#include "hubflow/insights/application/usecases/queries/getBPlusTree.hpp"
#include "hubflow/insights/application/usecases/queries/getBst.hpp"
#include "hubflow/insights/application/usecases/queries/getCodeSet.hpp"
#include "hubflow/insights/application/usecases/queries/lookupInBPlusTree.hpp"
#include "hubflow/insights/application/usecases/queries/lookupInBst.hpp"
#include "hubflow/insights/application/usecases/queries/rangeInBPlusTree.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/platform/http/apiRouter.hpp"

namespace hubflow::bootstrap
{

struct InsightsModule::Parts
{
    explicit Parts(const logistics::application::LogisticsModel& logistics)
        : lookupInBst {logistics}
        , getBst {logistics}
        , lookupInBPlusTree {logistics}
        , rangeInBPlusTree {logistics}
        , getBPlusTree {logistics}
        , compareShipmentStates {logistics}
        , getCodeSet {logistics}
        , combineCodeSets {logistics}
    {
    }

    insights::application::LookupInBst           lookupInBst;
    insights::application::GetBst                getBst;
    insights::application::LookupInBPlusTree     lookupInBPlusTree;
    insights::application::RangeInBPlusTree      rangeInBPlusTree;
    insights::application::GetBPlusTree          getBPlusTree;
    insights::application::CompareShipmentStates compareShipmentStates;
    insights::application::GetCodeSet            getCodeSet;
    insights::application::CombineCodeSets       combineCodeSets;
    std::unique_ptr<platform::Controller>        spController;
};

InsightsModule::InsightsModule(const logistics::application::LogisticsModel& logistics)
    : m_spParts {std::make_unique<Parts>(logistics)}
{
}

InsightsModule::~InsightsModule() = default;

void InsightsModule::registerRoutes(platform::ApiRouter& router)
{
    Parts& parts       = *m_spParts;
    parts.spController = std::make_unique<insights::adapters::InsightController>(
        insights::adapters::InsightUseCases {parts.getBst,
                                             parts.lookupInBst,
                                             parts.getBPlusTree,
                                             parts.lookupInBPlusTree,
                                             parts.rangeInBPlusTree,
                                             parts.compareShipmentStates,
                                             parts.getCodeSet,
                                             parts.combineCodeSets});
    parts.spController->registerRoutes(router);
}

} // namespace hubflow::bootstrap
