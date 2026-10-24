#ifndef HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_PARCEL_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_PARCEL_HPP

#include "hubflow/logistics/domain/valueobjects/dimensions.hpp"
#include "hubflow/logistics/domain/valueobjects/weight.hpp"

#include <cstddef>
#include <string>

namespace hubflow::logistics::domain
{

    /// @brief Datos de un bulto tal como se informan al registrar el envío y como se guardan: peso, medidas y si ya se
    /// entregó.
    struct ParcelData
    {
        double weightKg {0.0};
        double lengthCm {0.0};
        double widthCm {0.0};
        double heightCm {0.0};
        bool delivered {false};
    };

    /**
     * @brief Bulto: cada caja física de un envío, con su propia etiqueta, peso y medidas.
     *
     * La etiqueta es el código de seguimiento del envío seguido del número de bulto y el total, por ejemplo
     * @c HF-7K3M9Q2X8-2/3. Para facturar se usa el mayor entre el peso real y el peso volumétrico. Un bulto entregado
     * no vuelve a quedar pendiente.
     */
    class Parcel final
    {
    public:
        /// @throws BusinessError (Validation) si el peso o las medidas son inválidos.
        Parcel(const std::string& trackingCode, std::size_t ordinal, std::size_t total, const ParcelData& data);

        /// @brief Marca el bulto como entregado.
        void confirmDelivery() noexcept;

        const std::string& label() const noexcept;
        std::size_t ordinal() const noexcept;
        double weightKg() const noexcept;
        const Dimensions& dimensions() const noexcept;
        double volumetricWeightKg() const noexcept;
        double billableWeightKg() const noexcept;
        bool isDelivered() const noexcept;
        ParcelData details() const noexcept;

    private:
        std::string m_label;
        std::size_t m_ordinal;
        Weight m_weight;
        Dimensions m_dimensions;
        bool m_delivered;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_PARCEL_HPP
