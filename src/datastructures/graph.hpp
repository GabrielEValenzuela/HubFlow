#ifndef HUBFLOW_DATASTRUCTURES_GRAPH_HPP
#define HUBFLOW_DATASTRUCTURES_GRAPH_HPP

#include "datastructures/dynamicArray.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>

namespace hubflow::ds
{

    /**
     * @brief Grafo no dirigido y ponderado con listas de adyacencia enlazadas.
     *
     * Los vértices viven en un DynamicArray (acceso O(1) por índice) y cada vértice posee una
     * lista simplemente enlazada de aristas. Las aristas se agregan al final para que el orden de
     * expansión de las búsquedas sea determinista. El grafo libera todas sus aristas.
     *
     * | Operación       | Complejidad     |
     * |-----------------|-----------------|
     * | addVertex       | O(1) amortizado |
     * | addEdge         | O(1)            |
     * | forEachNeighbor | O(grado)        |
     * | findVertex      | O(V)            |
     *
     * Espacio: O(V + E).
     *
     * @tparam TVertexData datos de cada vértice (construible por defecto y copiable).
     */
    template<typename TVertexData>
    class Graph final
    {
    public:
        /// @brief Arista de la lista de adyacencia.
        struct Edge
        {
            std::size_t destination;
            double weight;
            Edge* pNext;
        };

        Graph() = default;

        ~Graph()
        {
            for (std::size_t vertexIndex = 0; vertexIndex < m_vertices.size(); ++vertexIndex)
            {
                Edge* pEdge = m_vertices[vertexIndex].pHead;
                while (pEdge != nullptr)
                {
                    Edge* pNext = pEdge->pNext;
                    delete pEdge;
                    pEdge = pNext;
                }
            }
        }

        Graph(const Graph&) = delete;
        Graph& operator=(const Graph&) = delete;

        /// @return índice del nuevo vértice.
        std::size_t addVertex(const TVertexData& data)
        {
            m_vertices.pushBack(Vertex {data, nullptr, nullptr, 0});
            return m_vertices.size() - 1;
        }

        /**
         * @brief Agrega la arista no dirigida origin—destination en ambas listas de adyacencia.
         * @throws std::invalid_argument si origin == destination.
         */
        void addEdge(const std::size_t origin, const std::size_t destination, const double weight)
        {
            if (origin == destination)
            {
                throw std::invalid_argument("No se admiten lazos");
            }
            append(m_vertices[origin], destination, weight);
            append(m_vertices[destination], origin, weight);
            ++m_edgeCount;
        }

        /// @brief Visita (destino, peso) de cada arista incidente a @p origin.
        template<typename TVisitor>
        void forEachNeighbor(const std::size_t origin, TVisitor visit) const
        {
            for (const Edge* pEdge = m_vertices[origin].pHead; pEdge != nullptr; pEdge = pEdge->pNext)
            {
                visit(pEdge->destination, pEdge->weight);
            }
        }

        /// @brief Visita (origin, destination, peso) de cada arista no dirigida una sola vez (origin < destination).
        template<typename TVisitor>
        void forEachEdge(TVisitor visit) const
        {
            for (std::size_t origin = 0; origin < m_vertices.size(); ++origin)
            {
                for (const Edge* pEdge = m_vertices[origin].pHead; pEdge != nullptr; pEdge = pEdge->pNext)
                {
                    if (origin < pEdge->destination)
                    {
                        visit(origin, pEdge->destination, pEdge->weight);
                    }
                }
            }
        }

        /// @brief Búsqueda lineal del primer vértice que cumple @p matches. O(V).
        template<typename TPredicate>
        std::optional<std::size_t> findVertex(TPredicate matches) const
        {
            std::optional<std::size_t> found;
            for (std::size_t vertexIndex = 0; vertexIndex < m_vertices.size() && !found.has_value(); ++vertexIndex)
            {
                if (matches(m_vertices[vertexIndex].data))
                {
                    found = vertexIndex;
                }
            }
            return found;
        }

        const TVertexData& vertex(const std::size_t index) const
        {
            return m_vertices[index].data;
        }

        /// @brief Reemplaza los datos del vértice sin tocar sus aristas. O(1).
        void replaceVertex(const std::size_t index, const TVertexData& data)
        {
            m_vertices[index].data = data;
        }

        std::size_t degree(const std::size_t index) const
        {
            return m_vertices[index].degree;
        }

        std::size_t vertexCount() const noexcept
        {
            return m_vertices.size();
        }

        std::size_t edgeCount() const noexcept
        {
            return m_edgeCount;
        }

    private:
        struct Vertex
        {
            TVertexData data;
            Edge* pHead;
            Edge* pTail;
            std::size_t degree;
        };

        static void append(Vertex& vertex, const std::size_t destination, const double weight)
        {
            Edge* pEdge = new Edge {destination, weight, nullptr};
            if (vertex.pTail == nullptr)
            {
                vertex.pHead = pEdge;
            }
            else
            {
                vertex.pTail->pNext = pEdge;
            }
            vertex.pTail = pEdge;
            ++vertex.degree;
        }

        DynamicArray<Vertex> m_vertices;
        std::size_t m_edgeCount {0};
    };

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_GRAPH_HPP
