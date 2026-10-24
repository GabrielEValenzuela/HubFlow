#ifndef HUBFLOW_SORTING_DOMAIN_ENTITIES_SORTING_BATCH_HPP
#define HUBFLOW_SORTING_DOMAIN_ENTITIES_SORTING_BATCH_HPP

#include "datastructures/dynamicArray.hpp"
#include "datastructures/queue.hpp"
#include "datastructures/stack.hpp"
#include "hubflow/logistics/domain/services/pendingShipments.hpp"
#include "hubflow/logistics/domain/services/shipmentGroupBook.hpp"
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
     * @brief Copia las referencias de los pendientes de la zona con código @p zone a un arreglo dinámico.
     * @throws business_error (Conflict) si la zona no tiene pendientes.
     */
    SortingBatch(std::uint64_t id, const std::string& zone, const logistics::domain::PendingShipments& pending);

    ~SortingBatch();

    SortingBatch(const SortingBatch&)            = delete;
    SortingBatch& operator=(const SortingBatch&) = delete;

    /// @brief Ordena el lote con Insertion Sort según @p ordering.
    SortStats sort(const ShipmentOrdering& ordering);

    /**
     * @brief Encola, en el orden actual del lote, los envíos que todavía no se cargaron al vehículo
     * (descarta la cola previa). Los ya procesados no se vuelven a encolar, así que repetir la
     * operación deja el mismo resultado. O(n * m), con m los envíos procesados.
     */
    void prepare();

    /**
     * @brief Saca el siguiente envío de la cola de preparación y lo apila como procesado. O(1) si no viaja en grupo.
     *
     * Si el envío viaja junto con otros, se cargan todos los del grupo a la vez, uno detrás de otro, para que queden
     * juntos en el vehículo.
     * @return los envíos cargados, en el orden en que quedaron apilados.
     * @throws business_error (Conflict) si la cola está vacía o falta cargable algún envío de su grupo.
     */
    ds::DynamicArray<logistics::domain::Shipment*> processNext(const logistics::domain::ShipmentGroupBook& groups);

    /**
     * @brief Carga al vehículo los envíos indicados por código, en ese orden.
     *
     * La mesa no carga un envío de un grupo sin los demás: si falta alguno de sus compañeros en @p codes se rechaza
     * todo sin cargar nada.
     * @throws business_error NotFound (un código no está por cargar) o Conflict (grupo incompleto).
     */
    ds::DynamicArray<logistics::domain::Shipment*> processByCodes(const ds::DynamicArray<std::string>&        codes,
                                                                  const logistics::domain::ShipmentGroupBook& groups);

    /**
     * @brief Devuelve a la mesa el envío @p code, siempre que sea el último que se procesó, junto con los
     * compañeros de su grupo que se cargaron con él.
     *
     * Repetir la operación es seguro: una vez devuelto el envío ya no está en el tope y el
     * segundo intento falla sin modificar el lote.
     * @throws business_error (NotFound) si @p code no es el tope de la pila de procesados.
     */
    ds::DynamicArray<logistics::domain::Shipment*> returnProcessed(const std::string&                          code,
                                                                   const logistics::domain::ShipmentGroupBook& groups);

    /**
     * @brief Descarga la pila de procesados en orden LIFO: lo último cargado al vehículo es lo
     * primero que se entrega. Deja la pila vacía.
     */
    ds::DynamicArray<logistics::domain::Shipment*> unloadProcessed();

    std::uint64_t                                  id() const noexcept;
    const std::string&                             zone() const noexcept;
    std::size_t                                    size() const noexcept;
    SortCriteria                                   sortedBy() const noexcept;
    const logistics::domain::Shipment&             shipmentAt(std::size_t index) const;
    const ds::Queue<logistics::domain::Shipment*>& preparation() const noexcept;
    const ds::Stack<logistics::domain::Shipment*>& processed() const noexcept;

private:
    bool isProcessed(const logistics::domain::Shipment* pShipment) const;

    ds::DynamicArray<logistics::domain::Shipment*> load(const ds::DynamicArray<logistics::domain::Shipment*>& selection,
                                                        const logistics::domain::ShipmentGroupBook&           groups);

    std::uint64_t                           m_id;
    std::string                             m_zone;
    logistics::domain::Shipment**           m_ppItems {nullptr};
    std::size_t                             m_size {0};
    SortCriteria                            m_sortedBy {SortCriteria::None};
    ds::Queue<logistics::domain::Shipment*> m_preparation;
    ds::Stack<logistics::domain::Shipment*> m_processed;
};

} // namespace hubflow::sorting::domain

#endif // HUBFLOW_SORTING_DOMAIN_ENTITIES_SORTING_BATCH_HPP
