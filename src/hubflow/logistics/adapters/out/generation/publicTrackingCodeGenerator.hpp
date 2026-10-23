#ifndef HUBFLOW_LOGISTICS_ADAPTERS_OUT_GENERATION_PUBLIC_TRACKING_CODE_GENERATOR_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_OUT_GENERATION_PUBLIC_TRACKING_CODE_GENERATOR_HPP

#include "hubflow/logistics/application/ports/out/trackingCodeGenerator.hpp"

#include <random>

namespace hubflow::logistics::adapters
{

/**
 * @brief Genera códigos públicos no adivinables: @c HF-, ocho símbolos al azar y un verificador.
 *
 * Si el código sorteado ya lo usa otro envío, o su verificador no tiene símbolo, sortea otro. El orden de los
 * códigos no tiene relación con el orden de alta.
 */
class PublicTrackingCodeGenerator final : public application::TrackingCodeGenerator
{
public:
    static constexpr unsigned MAX_ATTEMPTS {64};

    /// @brief Semilla tomada del sistema.
    PublicTrackingCodeGenerator();

    /// @brief Semilla fija, para pruebas reproducibles.
    explicit PublicTrackingCodeGenerator(std::uint64_t seed);

private:
    std::string next(const domain::ShipmentCatalog& catalog) override;

    std::mt19937_64 m_random;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_OUT_GENERATION_PUBLIC_TRACKING_CODE_GENERATOR_HPP
