#include "hubflow/sorting/domain/services/sortingDesk.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::sorting::domain
{

    SortingDesk::~SortingDesk()
    {
        close();
    }

    SortingBatch& SortingDesk::open(const std::string& zone, const logistics::domain::PendingShipments& pending)
    {
        SortingBatch* pBatch = new SortingBatch {m_lastId + 1, zone, pending};
        close();
        m_pActive = pBatch;
        ++m_lastId;
        return *m_pActive;
    }

    SortingBatch& SortingDesk::require(const std::uint64_t id) const
    {
        if (m_pActive == nullptr || m_pActive->id() != id)
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::NotFound, "sorting.batch_not_found", "El lote no existe o ya fue cerrado"};
        }
        return *m_pActive;
    }

    const SortingBatch* SortingDesk::activeBatch() const noexcept
    {
        return m_pActive;
    }

    void SortingDesk::close() noexcept
    {
        delete m_pActive;
        m_pActive = nullptr;
    }

} // namespace hubflow::sorting::domain
