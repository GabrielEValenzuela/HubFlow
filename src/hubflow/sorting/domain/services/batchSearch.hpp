#ifndef HUBFLOW_SORTING_DOMAIN_SERVICES_BATCH_SEARCH_HPP
#define HUBFLOW_SORTING_DOMAIN_SERVICES_BATCH_SEARCH_HPP

#include "datastructures/singlyLinkedList.hpp"
#include "hubflow/sorting/domain/entities/sortingBatch.hpp"

#include <optional>
#include <string>

namespace hubflow::sorting::domain
{

    /// @brief Estado de los límites en una iteración de la búsqueda binaria.
    struct BinarySearchStep
    {
        std::size_t left;
        std::size_t right;
        std::size_t middle;
        std::string codeAtMiddle;
    };

    /// @brief Resultado de buscar un código dentro del lote.
    struct BatchSearchOutcome
    {
        std::optional<std::size_t> position;
        std::size_t comparisons {0};
        ds::SinglyLinkedList<BinarySearchStep> steps;
    };

    /// @brief Algoritmo de búsqueda sobre el arreglo del lote (Strategy).
    class BatchSearch
    {
    public:
        virtual ~BatchSearch() = default;

        /// @brief Busca @p code contando comparaciones.
        virtual BatchSearchOutcome search(const SortingBatch& batch, const std::string& code) const = 0;
    };

    /// @brief Recorre desde la posición 0 hasta encontrar o terminar. Mejor O(1), peor O(n).
    class LinearBatchSearch final : public BatchSearch
    {
    private:
        BatchSearchOutcome search(const SortingBatch& batch, const std::string& code) const override;
    };

    /**
     * @brief Búsqueda binaria manual. Precondición: lote ordenado por código ascendente.
     *
     * mid = izq + (der - izq) / 2; si el código del medio es menor se descarta la mitad izquierda,
     * si es mayor la derecha; termina al encontrar o cuando izq > der. Mejor O(1), peor O(log n).
     * @throws business_error (InvalidState) si el lote no está ordenado por código.
     */
    class BinaryBatchSearch final : public BatchSearch
    {
    private:
        BatchSearchOutcome search(const SortingBatch& batch, const std::string& code) const override;
    };

} // namespace hubflow::sorting::domain

#endif // HUBFLOW_SORTING_DOMAIN_SERVICES_BATCH_SEARCH_HPP
