#include "hubflow/insights/adapters/in/http/insightsJsonPresenter.hpp"

#include "hubflow/logistics/adapters/in/http/logisticsJsonPresenter.hpp"
#include "hubflow/shared/application/vocabulary.hpp"

namespace hubflow::insights::adapters
{
namespace
{
using logistics::adapters::LogisticsJsonPresenter;
using platform::Json;
using shared::application::Vocabulary;

constexpr std::size_t B_PLUS_ORDER {4};

Json bstNode(const logistics::domain::ShipmentBst::Node* pNode)
{
    Json node = nullptr;
    if (pNode != nullptr)
    {
        node = Json {{"code", pNode->key}, {"left", bstNode(pNode->pLeft)}, {"right", bstNode(pNode->pRight)}};
    }
    return node;
}

Json bPlusNode(const logistics::domain::ShipmentBPlusTree::Node* pNode)
{
    Json node = nullptr;
    if (pNode != nullptr)
    {
        Json keys     = Json::array();
        Json children = Json::array();
        for (std::size_t i = 0; i < pNode->keyCount; ++i)
        {
            keys.push_back(pNode->keys[i]);
        }
        for (std::size_t i = 0; !pNode->isLeaf && i <= pNode->keyCount; ++i)
        {
            children.push_back(bPlusNode(pNode->pChildren[i]));
        }
        node = Json {{"leaf", pNode->isLeaf}, {"keys", keys}, {"children", children}};
    }
    return node;
}

struct StepName
{
    ds::BstStep step;
    const char* text;
};

constexpr StepName STEP_NAMES[] {{ds::BstStep::Left, "izquierda"},
                                 {ds::BstStep::Right, "derecha"},
                                 {ds::BstStep::Found, "encontrado"}};

std::string directionOf(const ds::BstStep step)
{
    std::string text;
    for (const auto& entry : STEP_NAMES)
    {
        if (entry.step == step)
        {
            text = entry.text;
        }
    }
    return text;
}
} // namespace

Json InsightsJsonPresenter::bst(const logistics::domain::ShipmentBst& tree)
{
    Json inOrder = Json::array();
    tree.inOrder(
        [&inOrder](const std::string& code, const logistics::domain::Shipment*)
        {
            inOrder.push_back(code);
        });
    return Json {
        {"size", tree.size()}, {"height", tree.height()}, {"inOrder", inOrder}, {"root", bstNode(tree.root())}};
}

Json InsightsJsonPresenter::bstLookup(const application::BstLookup& lookup)
{
    Json path = Json::array();
    for (std::size_t i = 0; i < lookup.path.size(); ++i)
    {
        const auto& visit = lookup.path[i];
        path.push_back(Json {{"code", visit.code}, {"direction", directionOf(visit.step)}});
    }
    return Json {{"found", lookup.pShipment != nullptr},
                 {"visited", lookup.visited},
                 {"path", path},
                 {"shipment",
                  lookup.pShipment == nullptr ? Json(nullptr) : LogisticsJsonPresenter::shipment(*lookup.pShipment)}};
}

Json InsightsJsonPresenter::bPlusTree(const logistics::domain::ShipmentBPlusTree& tree)
{
    Json leaves = Json::array();
    tree.forEachInOrder(
        [&leaves](const std::string& code, const logistics::domain::Shipment*)
        {
            leaves.push_back(code);
        });
    return Json {{"order", B_PLUS_ORDER},
                 {"size", tree.size()},
                 {"levels", tree.levels()},
                 {"leavesAtSameDepth", tree.leavesAtSameDepth()},
                 {"sequence", leaves},
                 {"root", bPlusNode(tree.root())}};
}

Json InsightsJsonPresenter::codeSet(const domain::CodeSet& set)
{
    Json codes = Json::array();
    set.forEach(
        [&codes](const std::string& code)
        {
            codes.push_back(code);
        });
    return Json {{"size", set.size()}, {"codes", codes}};
}

Json InsightsJsonPresenter::stateComparison(const domain::StateComparison& comparison)
{
    return Json {{"common", LogisticsJsonPresenter::stateHistory(comparison.common)},
                 {"onlyFirst", LogisticsJsonPresenter::stateHistory(comparison.onlyFirst)},
                 {"onlySecond", LogisticsJsonPresenter::stateHistory(comparison.onlySecond)},
                 {"either", LogisticsJsonPresenter::stateHistory(comparison.either)},
                 {"exclusive", LogisticsJsonPresenter::stateHistory(comparison.exclusive)}};
}

Json InsightsJsonPresenter::selector(const application::SetSelector& selector)
{
    return selector.kind == application::SetSelector::Kind::Zone ? Json {{"zone", selector.zone}}
                                                                 : Json {{"service", selector.service}};
}

} // namespace hubflow::insights::adapters
