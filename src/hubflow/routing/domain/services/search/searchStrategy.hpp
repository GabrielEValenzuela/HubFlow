#ifndef HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_STRATEGY_HPP
#define HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_STRATEGY_HPP

#include "hubflow/routing/domain/entities/roadMap.hpp"
#include "hubflow/routing/domain/services/search/frontier.hpp"
#include "hubflow/routing/domain/services/search/searchNodePool.hpp"
#include "hubflow/routing/domain/services/search/searchTrace.hpp"

namespace hubflow::routing::domain
{

/// @brief Registro de la búsqueda en curso, indexado por vértice.
struct SearchBookkeeping
{
    ds::DynamicArray<bool>   explored;
    ds::DynamicArray<bool>   inFrontier;
    ds::DynamicArray<double> bestCost;
};

/**
 * @brief Búsqueda en grafo del agente PUM (Template Method + Strategy).
 *
 * Formulación del problema: el estado es el nodo donde está el PUM, el estado inicial es
 * @c start, el objetivo @c goal, las acciones mueven al PUM a un nodo adyacente y el costo de
 * cada acción son los km de la arista.
 *
 * El esqueleto es común: se extrae un nodo de la frontera, se descarta si ya fue explorado, se
 * aplica el test de objetivo al expandir y se generan los sucesores. Las subclases deciden la
 * frontera, la prioridad de cada nodo y cuándo admitir un sucesor.
 *
 * Los SearchNode se crean con @c new en un pool propio de cada búsqueda y se liberan al terminar.
 */
class SearchStrategy
{
public:
    virtual ~SearchStrategy() = default;

    /// @brief Ejecuta la búsqueda y devuelve su traza completa.
    SearchTrace search(const RoadMap& map, std::size_t start, std::size_t goal) const;

    virtual SearchAlgorithm algorithm() const noexcept = 0;

    /// @brief Estrategia sin estado correspondiente a @p algorithm.
    static const SearchStrategy& of(SearchAlgorithm algorithm);

protected:
    /// @brief Nueva frontera vacía; ownership al llamador.
    virtual Frontier* createFrontier() const = 0;

    /// @brief Prioridad del nodo a partir de g(n) y h(n). Menor se expande antes.
    virtual double priority(double cost, double heuristic) const noexcept = 0;

    /// @brief Decide si un sucesor no explorado entra a la frontera.
    virtual bool admits(std::size_t vertex, double cost, const SearchBookkeeping& bookkeeping) const noexcept;

private:
    void expand(const RoadMap&     map,
                SearchNode*        pNode,
                std::size_t        goal,
                Frontier&          frontier,
                SearchBookkeeping& bookkeeping,
                SearchNodePool&    pool) const;
};

/// @brief BFS: frontera FIFO; óptimo en cantidad de saltos, no en km.
class BreadthFirstSearch final : public SearchStrategy
{
private:
    SearchAlgorithm algorithm() const noexcept override;
    Frontier*       createFrontier() const override;
    double          priority(double cost, double heuristic) const noexcept override;
};

/// @brief DFS: frontera LIFO; completo en grafos finitos con explorados, no óptimo.
class DepthFirstSearch final : public SearchStrategy
{
private:
    SearchAlgorithm algorithm() const noexcept override;
    Frontier*       createFrontier() const override;
    double          priority(double cost, double heuristic) const noexcept override;
};

/// @brief Greedy best-first: prioriza h(n); suele expandir poco pero no es óptimo.
class GreedyBestFirstSearch final : public SearchStrategy
{
private:
    SearchAlgorithm algorithm() const noexcept override;
    Frontier*       createFrontier() const override;
    double          priority(double cost, double heuristic) const noexcept override;
};

/// @brief A*: prioriza f(n) = g(n) + h(n); óptimo en km porque h es admisible.
class AStarSearch final : public SearchStrategy
{
private:
    SearchAlgorithm algorithm() const noexcept override;
    Frontier*       createFrontier() const override;
    double          priority(double cost, double heuristic) const noexcept override;
    bool admits(std::size_t vertex, double cost, const SearchBookkeeping& bookkeeping) const noexcept override;
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_SERVICES_SEARCH_SEARCH_STRATEGY_HPP
