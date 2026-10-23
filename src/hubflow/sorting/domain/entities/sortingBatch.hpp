#ifndef HUBFLOW_SORTING_DOMAIN_ENTITIES_SORTING_BATCH_HPP
#define HUBFLOW_SORTING_DOMAIN_ENTITIES_SORTING_BATCH_HPP

#include "datastructures/dynamicArray.hpp"
#include "datastructures/queue.hpp"
#include "datastructures/stack.hpp"
#include "hubflow/logistics/domain/services/pendingShipments.hpp"
#include "hubflow/sorting/domain/services/insertionSort.hpp"

#include <cstdint>

namespace hubflow::sorting::domain
{

/**
 * @brief Agregado "lote de la mesa de clasificación y despacho": una selección temporal de envíos pendientes
 * de una zona que se ordena, se prepara y se va procesando.
 *
 * Ownership: el arreglo dinámico @c m_ppItems (Shipment**), la Queue de preparación y
 * la Stack de procesados guardan referencias NO propietarias a envíos del ShipmentCatalog. El
 * destructor libera el arreglo con @c delete[] y las estructuras liberan sus nodos; ninguna
 * ejecuta @c delete sobre un Shipment, evitando double free y dangling pointers.
 */
class SortingBatch final
{
public:
    /**
     * @brief Copia las referencias de los pendientes de @p zone a un arreglo dinámico.
     * @throws business_error (Conflict) si la zona no tiene pendientes.
     */
    SortingBatch(std::uint64_t id, logistics::domain::Zone zone, const logistics::domain::PendingShipments& pending);

    ~SortingBatch();

    SortingBatch(const SortingBatch&)            = delete;
    SortingBatch& operator=(const SortingBatch&) = delete;

    /// @brief Ordena el lote con Insertion Sort según @p ordering.
    SortStats sort(const ShipmentOrdering& ordering);

    /// @brief Encola el lote en su orden actual (descarta la cola previa). O(n).
    void prepare();

    /**
     * @brief Saca el siguiente envío de la cola de preparación y lo apila como procesado. O(1).
     * @throws business_error (Conflict) si la cola está vacía.
     */
    logistics::domain::Shipment& processNext();

    /**
     * @brief Devuelve a la mesa el envío @p code, siempre que sea el último que se procesó.
     *
     * Repetir la operación es seguro: una vez devuelto el envío ya no está en el tope y el
     * segundo intento falla sin modificar el lote. O(1).
     * @throws business_error (NotFound) si @p code no es el tope de la pila de procesados.
     */
    logistics::domain::Shipment& returnProcessed(const std::string& code);

    /**
     * @brief Descarga la pila de procesados en orden LIFO: lo último cargado al vehículo es lo
     * primero que se entrega. Deja la pila vacía.
     */
    ds::DynamicArray<logistics::domain::Shipment*> unloadProcessed();

    std::uint64_t                                  id() const noexcept;
    logistics::domain::Zone                        zone() const noexcept;
    std::size_t                                    size() const noexcept;
    SortCriteria                                   sortedBy() const noexcept;
    const logistics::domain::Shipment&             at(std::size_t index) const;
    const ds::Queue<logistics::domain::Shipment*>& preparation() const noexcept;
    const ds::Stack<logistics::domain::Shipment*>& processed() const noexcept;

private:
    std::uint64_t                           m_id;
    logistics::domain::Zone                 m_zone;
    logistics::domain::Shipment**           m_ppItems {nullptr};
    std::size_t                             m_size {0};
    SortCriteria                            m_sortedBy {SortCriteria::None};
    ds::Queue<logistics::domain::Shipment*> m_preparation;
    ds::Stack<logistics::domain::Shipment*> m_processed;
};

} // namespace hubflow::sorting::domain

#endif // HUBFLOW_SORTING_DOMAIN_ENTITIES_SORTING_BATCH_HPP
