#ifndef HUBFLOW_SHARED_APPLICATION_PORTS_OUT_IDENTITY_GENERATOR_HPP
#define HUBFLOW_SHARED_APPLICATION_PORTS_OUT_IDENTITY_GENERATOR_HPP

#include <string>

namespace hubflow::shared::application
{

    /**
     * @brief Fuente de identidades internas únicas (puerto), reemplazable en tests.
     *
     * Cada identidad es un UUID versión 7 en texto: los primeros 48 bits son la hora de creación en milisegundos, así
     * que las identidades generadas después se ordenan después, también dentro del mismo milisegundo.
     */
    class IdentityGenerator
    {
    public:
        virtual ~IdentityGenerator() = default;

        /// @brief Nueva identidad, distinta de todas las anteriores y posterior a ellas en el orden de texto.
        virtual std::string next() = 0;
    };

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_PORTS_OUT_IDENTITY_GENERATOR_HPP
