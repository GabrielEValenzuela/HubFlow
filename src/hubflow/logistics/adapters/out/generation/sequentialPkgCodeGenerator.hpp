#ifndef HUBFLOW_LOGISTICS_ADAPTERS_OUT_GENERATION_SEQUENTIAL_PKG_CODE_GENERATOR_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_OUT_GENERATION_SEQUENTIAL_PKG_CODE_GENERATOR_HPP

#include "hubflow/logistics/application/ports/out/trackingCodeGenerator.hpp"

namespace hubflow::logistics::adapters
{

    /**
     * @brief Genera códigos correlativos @c PKG-NNNN, como los del dataset del trabajo práctico.
     *
     * Avanza desde el primer número indicado y salta los que ya usa algún envío, así que sigue correctamente después de
     * reiniciar el sistema con envíos ya cargados.
     */
    class SequentialPkgCodeGenerator final : public application::TrackingCodeGenerator
    {
    public:
        static constexpr unsigned FIRST_NUMBER {1001};
        static constexpr unsigned LAST_NUMBER {9999};

        explicit SequentialPkgCodeGenerator(unsigned firstNumber = FIRST_NUMBER);

    private:
        std::string next(const domain::ShipmentCatalog& catalog) override;

        unsigned m_next;
    };

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_OUT_GENERATION_SEQUENTIAL_PKG_CODE_GENERATOR_HPP
