#include "hubflow/insights/application/usecases/queries/rangeInBPlusTree.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::insights::application
{

RangeInBPlusTree::RangeInBPlusTree(const logistics::application::LogisticsModel& model)
    : m_model {model}
{
}

RangeResult RangeInBPlusTree::execute(const shared::application::Principal& principal,
                                      const std::string&                    from,
                                      const std::string&                    to) const
{
    principal.require(identity::domain::Permission::ViewIndexes);
    RangeResult result;
    result.visited =
        m_model.catalog.bPlusTree().range(from,
                                          to,
                                          [&result](const std::string&, const logistics::domain::Shipment* pShipment)
                                          {
                                              result.shipments.pushBack(pShipment);
                                          });
    return result;
}

} // namespace hubflow::insights::application
