#ifndef HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_FEATURE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_FEATURE_HPP

#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <optional>
#include <string>

namespace hubflow::logistics::domain
{

/// @brief Estado persistible de una característica de envío.
struct ShipmentFeatureState
{
    std::string code;
    std::string name;
    bool        active {true};
};

/**
 * @brief Característica con la que se puede etiquetar un envío (frágil, refrigerado, voluminoso, etc.).
 *
 * Un envío puede tener ninguna o varias. Desactivar una característica sólo impide etiquetar envíos nuevos con ella:
 * los envíos que ya la tienen la conservan.
 */
class ShipmentFeature final : public shared::domain::AggregateRoot
{
public:
    /// @brief Alta de una característica activa. Ownership del resultado al llamador.
    static ShipmentFeature*
    open(const std::string& code, const std::string& name, const shared::domain::ActorContext& context);

    /// @brief Reconstituye una característica persistida. Ownership al llamador.
    static ShipmentFeature* restore(const ShipmentFeatureState& state);

    /**
     * @brief Aplica los cambios pedidos (nombre y/o habilitación) como una sola operación. Si no cambia nada no
     * registra eventos, por lo que repetir el pedido deja todo igual.
     * @throws business_error (Validation) si el nombre es inválido.
     */
    void revise(const std::optional<std::string>&   name,
                const std::optional<bool>&          active,
                const shared::domain::ActorContext& context);

    const std::string&          code() const noexcept;
    const std::string&          name() const noexcept;
    bool                        active() const noexcept;
    const ShipmentFeatureState& state() const noexcept;

private:
    explicit ShipmentFeature(ShipmentFeatureState state);

    ShipmentFeatureState m_state;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_FEATURE_HPP
