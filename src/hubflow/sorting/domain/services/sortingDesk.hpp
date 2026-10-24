#ifndef HUBFLOW_SORTING_DOMAIN_SERVICES_SORTING_DESK_HPP
#define HUBFLOW_SORTING_DOMAIN_SERVICES_SORTING_DESK_HPP

#include "hubflow/sorting/domain/entities/sortingBatch.hpp"

namespace hubflow::sorting::domain
{

/**
 * @brief Mesa de clasificación: administra el único lote activo.
 *
 * Es dueña del lote (creado con @c new); abrir uno nuevo o cerrarlo destruye el anterior. Como el
 * lote sólo guarda referencias no propietarias, destruirlo nunca afecta a los envíos.
 */
class SortingDesk final
{
public:
    SortingDesk() = default;

    ~SortingDesk();

    SortingDesk(const SortingDesk&)            = delete;
    SortingDesk& operator=(const SortingDesk&) = delete;

    /// @brief Abre un lote para @p zone reemplazando al activo.
    SortingBatch& open(const std::string& zone, const logistics::domain::PendingShipments& pending);

    /// @throws business_error (NotFound) si @p id no es el lote activo.
    SortingBatch& require(std::uint64_t id) const;

    /// @brief Lote activo o nullptr.
    const SortingBatch* activeBatch() const noexcept;

    /// @brief Cierra (destruye) el lote activo.
    void close() noexcept;

private:
    SortingBatch* m_pActive {nullptr};
    std::uint64_t m_lastId {0};
};

} // namespace hubflow::sorting::domain

#endif // HUBFLOW_SORTING_DOMAIN_SERVICES_SORTING_DESK_HPP
