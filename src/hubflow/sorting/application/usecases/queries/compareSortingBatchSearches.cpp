#include "hubflow/sorting/application/usecases/queries/compareSortingBatchSearches.hpp"

#include "hubflow/identity/domain/services/accessPolicy.hpp"

namespace hubflow::sorting::application
{
    namespace
    {
        const domain::LinearBatchSearch gs_linearSearchImpl;
        const domain::BinaryBatchSearch gs_binarySearchImpl;
        const domain::BatchSearch& gs_linearSearch {gs_linearSearchImpl};
        const domain::BatchSearch& gs_binarySearch {gs_binarySearchImpl};

        constexpr const char* BINARY_REQUIRES_ORDER {"La búsqueda binaria requiere el lote ordenado por código"};
    } // namespace

    CompareSortingBatchSearches::CompareSortingBatchSearches(const domain::SortingDesk& desk)
        : m_desk {desk}
    {
    }

    SearchComparison CompareSortingBatchSearches::execute(const shared::application::Principal& principal,
                                                          std::uint64_t batchId,
                                                          const std::string& code) const
    {
        principal.require(identity::domain::Permission::OperateSortingDesk);
        const domain::SortingBatch& target = m_desk.require(batchId);
        SearchComparison comparison {gs_linearSearch.search(target, code), std::nullopt, ""};
        if (target.sortedBy() == domain::SortCriteria::CodeAscending)
        {
            comparison.binary = gs_binarySearch.search(target, code);
        }
        else
        {
            comparison.binaryUnavailableReason = BINARY_REQUIRES_ORDER;
        }
        return comparison;
    }

} // namespace hubflow::sorting::application
