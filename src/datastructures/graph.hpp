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
 * @tparam VertexData datos de cada vértice (construible por defecto y copiable).
 */
template <typename VertexData>
class Graph final
{
public:
    /// @brief Arista de la lista de adyacencia.
    struct Edge
    {
        std::size_t to;
        double      weight;
        Edge*       pNext;
    };

    Graph() = default;

    ~Graph()
    {
        for (std::size_t i = 0; i < m_vertices.size(); ++i)
        {
            Edge* pEdge = m_vertices[i].pHead;
            while (pEdge != nullptr)
            {
                Edge* pNext = pEdge->pNext;
                delete pEdge;
                pEdge = pNext;
            }
        }
    }

    Graph(const Graph&)            = delete;
    Graph& operator=(const Graph&) = delete;

    /// @return índice del nuevo vértice.
    std::size_t addVertex(const VertexData& data)
    {
        m_vertices.pushBack(Vertex {data, nullptr, nullptr, 0});
        return m_vertices.size() - 1;
    }

    /**
     * @brief Agrega la arista no dirigida u—v en ambas listas de adyacencia.
     * @throws std::invalid_argument si u == v.
     */
    void addEdge(const std::size_t u, const std::size_t v, const double weight)
    {
        if (u == v)
        {
            throw std::invalid_argument("No se admiten lazos");
        }
        append(m_vertices[u], v, weight);
        append(m_vertices[v], u, weight);
        ++m_edgeCount;
    }

    /// @brief Visita (destino, peso) de cada arista incidente a @p u.
    template <typename Visitor>
    void forEachNeighbor(const std::size_t u, Visitor visit) const
    {
        for (const Edge* pEdge = m_vertices[u].pHead; pEdge != nullptr; pEdge = pEdge->pNext)
        {
            visit(pEdge->to, pEdge->weight);
        }
    }

    /// @brief Visita (u, v, peso) de cada arista no dirigida una sola vez (u < v).
    template <typename Visitor>
    void forEachEdge(Visitor visit) const
    {
        for (std::size_t u = 0; u < m_vertices.size(); ++u)
        {
            for (const Edge* pEdge = m_vertices[u].pHead; pEdge != nullptr; pEdge = pEdge->pNext)
            {
                if (u < pEdge->to)
                {
                    visit(u, pEdge->to, pEdge->weight);
                }
            }
        }
    }

    /// @brief Búsqueda lineal del primer vértice que cumple @p matches. O(V).
    template <typename Predicate>
    std::optional<std::size_t> findVertex(Predicate matches) const
    {
        std::optional<std::size_t> found;
        for (std::size_t i = 0; i < m_vertices.size() && !found.has_value(); ++i)
        {
            if (matches(m_vertices[i].data))
            {
                found = i;
            }
        }
        return found;
    }

    const VertexData& vertex(const std::size_t index) const
    {
        return m_vertices[index].data;
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
        VertexData  data;
        Edge*       pHead;
        Edge*       pTail;
        std::size_t degree;
    };

    static void append(Vertex& vertex, const std::size_t to, const double weight)
    {
        Edge* pEdge = new Edge {to, weight, nullptr};
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
    std::size_t          m_edgeCount {0};
};

} // namespace hubflow::ds

#endif // HUBFLOW_DATASTRUCTURES_GRAPH_HPP
