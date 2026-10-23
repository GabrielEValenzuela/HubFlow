#ifndef HUBFLOW_SHARED_ADAPTERS_OUT_GENERATION_UUID_V7_IDENTITY_GENERATOR_HPP
#define HUBFLOW_SHARED_ADAPTERS_OUT_GENERATION_UUID_V7_IDENTITY_GENERATOR_HPP

#include "hubflow/shared/application/ports/out/identityGenerator.hpp"

#include <cstdint>
#include <functional>
#include <random>
#include <string>

namespace hubflow::shared::adapters
{

/**
 * @brief Genera identidades UUID versión 7 (RFC 9562): 48 bits de hora en milisegundos, versión, un contador de 12
 * bits, variante y 62 bits al azar.
 *
 * Con el contador, las identidades pedidas dentro del mismo milisegundo siguen creciendo en orden de texto. Si el
 * contador se agota se adelanta un milisegundo lógico, y si el reloj retrocede se sigue desde el último valor usado,
 * de modo que el orden nunca se invierte.
 */
class UuidV7IdentityGenerator final : public application::IdentityGenerator
{
public:
    using MillisecondSource = std::function<std::uint64_t()>;

    /// @brief Usa el reloj del sistema.
    UuidV7IdentityGenerator();

    /// @brief Usa @p source como reloj (milisegundos desde epoch), para pruebas.
    explicit UuidV7IdentityGenerator(MillisecondSource source);

private:
    std::string next() override;

    MillisecondSource m_now;
    std::mt19937_64   m_random;
    std::uint64_t     m_lastMillis {0};
    std::uint32_t     m_counter {0};
};

} // namespace hubflow::shared::adapters

#endif // HUBFLOW_SHARED_ADAPTERS_OUT_GENERATION_UUID_V7_IDENTITY_GENERATOR_HPP
