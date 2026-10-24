#ifndef HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COLLECT_CODES_HPP
#define HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COLLECT_CODES_HPP

#include "hubflow/insights/application/ports/in/getCodeSetUseCase.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"

namespace hubflow::insights::application
{

    /// @brief Reúne los códigos del catálogo que cumplen el criterio pedido.
    domain::CodeSet collectCodes(const logistics::domain::ShipmentCatalog& catalog, const SetSelector& selector);

} // namespace hubflow::insights::application

#endif // HUBFLOW_INSIGHTS_APPLICATION_USECASES_QUERIES_COLLECT_CODES_HPP
