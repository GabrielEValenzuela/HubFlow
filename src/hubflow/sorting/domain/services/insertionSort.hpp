#ifndef HUBFLOW_SORTING_DOMAIN_SERVICES_INSERTION_SORT_HPP
#define HUBFLOW_SORTING_DOMAIN_SERVICES_INSERTION_SORT_HPP

#include "hubflow/sorting/domain/services/shipmentOrdering.hpp"

#include <cstddef>

namespace hubflow::sorting::domain
{

/// @brief Métricas de una ejecución del algoritmo de ordenamiento.
struct SortStats
{
    std::size_t comparisons {0};
    std::size_t shifts {0};
};

/**
 * @brief Insertion Sort manual sobre un arreglo de punteros (no mueve los objetos Shipment).
 *
 * Es estable. Mejor caso O(n) con el arreglo ya ordenado (una comparación por elemento); peor
 * caso O(n²) con el arreglo en orden inverso. Espacio adicional O(1).
 */
SortStats insertionSort(logistics::domain::Shipment** pItems, std::size_t size, const ShipmentOrdering& ordering);

} // namespace hubflow::sorting::domain

#endif // HUBFLOW_SORTING_DOMAIN_SERVICES_INSERTION_SORT_HPP
