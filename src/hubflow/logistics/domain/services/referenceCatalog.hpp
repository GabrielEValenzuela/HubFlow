#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_REFERENCE_CATALOG_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_REFERENCE_CATALOG_HPP

#include "datastructures/singlyLinkedList.hpp"
#include "hubflow/logistics/domain/entities/serviceLevel.hpp"
#include "hubflow/logistics/domain/entities/shipmentFeature.hpp"
#include "hubflow/logistics/domain/entities/zone.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <string>
#include <utility>

namespace hubflow::logistics::domain
{

/// @brief Cómo se nombra un tipo de dato maestro en los mensajes de error.
struct CatalogNouns
{
    std::string kind;
    std::string definite;
    std::string indefinite;
};

/**
 * @brief Colección en memoria de un tipo de dato maestro (zonas, niveles de servicio o características).
 *
 * Es la única dueña de los objetos: los recibe por @c adopt y los destruye en su destructor. Los datos maestros
 * nunca se eliminan, sólo se desactivan, así que ninguna referencia queda colgando.
 *
 * | Operación | Complejidad |
 * |-----------|-------------|
 * | adopt     | O(n)        |
 * | find      | O(n)        |
 *
 * @tparam Entry agregado con una función @c code().
 */
template <typename Entry>
class ReferenceCatalog final
{
public:
    explicit ReferenceCatalog(CatalogNouns nouns)
        : m_nouns {std::move(nouns)}
    {
    }

    ~ReferenceCatalog()
    {
        m_entries.forEach(
            [](Entry* pEntry)
            {
                delete pEntry;
            });
    }

    ReferenceCatalog(const ReferenceCatalog&)            = delete;
    ReferenceCatalog& operator=(const ReferenceCatalog&) = delete;

    /**
     * @brief Toma ownership de @p pEntry.
     * @throws business_error (Conflict) si ya existe uno con ese código; en ese caso libera @p pEntry.
     */
    Entry& adopt(Entry* pEntry)
    {
        if (contains(pEntry->code()))
        {
            delete pEntry;
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                  m_nouns.kind + ".duplicate",
                                                  "Ya existe " + m_nouns.indefinite + " con ese código"};
        }
        m_entries.pushBack(pEntry);
        return *pEntry;
    }

    /// @brief nullptr si no existe.
    Entry* find(const std::string& code) const
    {
        Entry* const* ppFound = m_entries.findFirst(
            [&code](const Entry* pEntry)
            {
                return pEntry->code() == code;
            });
        return ppFound == nullptr ? nullptr : *ppFound;
    }

    bool contains(const std::string& code) const
    {
        return find(code) != nullptr;
    }

    /// @throws business_error (NotFound) si no existe.
    Entry& require(const std::string& code) const
    {
        Entry* pEntry = find(code);
        if (pEntry == nullptr)
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::NotFound, m_nouns.kind + ".not_found", "No existe " + m_nouns.definite};
        }
        return *pEntry;
    }

    /// @brief Visita los datos en orden de alta.
    template <typename Visitor>
    void forEach(Visitor visit) const
    {
        m_entries.forEach(
            [&visit](Entry* pEntry)
            {
                visit(*pEntry);
            });
    }

    std::size_t size() const noexcept
    {
        return m_entries.size();
    }

private:
    CatalogNouns                 m_nouns;
    ds::SinglyLinkedList<Entry*> m_entries;
};

using ZoneCatalog         = ReferenceCatalog<Zone>;
using ServiceLevelCatalog = ReferenceCatalog<ServiceLevel>;
using FeatureCatalog      = ReferenceCatalog<ShipmentFeature>;

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_REFERENCE_CATALOG_HPP
