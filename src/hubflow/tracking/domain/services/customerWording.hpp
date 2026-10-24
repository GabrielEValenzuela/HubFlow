#ifndef HUBFLOW_TRACKING_DOMAIN_SERVICES_CUSTOMER_WORDING_HPP
#define HUBFLOW_TRACKING_DOMAIN_SERVICES_CUSTOMER_WORDING_HPP

#include "hubflow/tracking/domain/valueobjects/milestone.hpp"

#include <cstddef>
#include <string>

namespace hubflow::tracking::domain
{

    /**
     * @brief Cómo se le cuenta cada hito al cliente: frases cálidas y precisas, sin jerga de la operación.
     *
     * Es la única fuente de las frases: la línea de tiempo de la página de seguimiento y los avisos que se mandan al
     * cliente dicen lo mismo con las mismas palabras.
     */
    class CustomerWording final
    {
    public:
        /// @brief Frase del hito, por ejemplo "Está en camino".
        static const char* sentence(Milestone milestone) noexcept;

        /// @brief Identificador estable del hito para que las pantallas lo usen como estilo (RECEIVED, ON_THE_WAY...).
        static const char* stage(Milestone milestone) noexcept;

        /// @brief Aviso para el cliente: el código del paquete seguido de la frase del hito.
        static std::string notice(Milestone milestone, const std::string& code);

        /// @brief Aviso cuando sólo se entregaron algunos de los bultos y el resto queda para un nuevo intento.
        static std::string partialNotice(const std::string& code, std::size_t delivered, std::size_t total);
    };

} // namespace hubflow::tracking::domain

#endif // HUBFLOW_TRACKING_DOMAIN_SERVICES_CUSTOMER_WORDING_HPP
