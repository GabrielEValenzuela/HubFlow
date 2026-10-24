#ifndef HUBFLOW_SHARED_APPLICATION_PORTS_OUT_IDEMPOTENCY_STORE_HPP
#define HUBFLOW_SHARED_APPLICATION_PORTS_OUT_IDEMPOTENCY_STORE_HPP

#include <cstdint>
#include <optional>
#include <string>

namespace hubflow::shared::application
{

    /// @brief Respuesta HTTP ya entregada para una clave de idempotencia.
    struct StoredResponse
    {
        std::string method;
        std::string path;
        int status {0};
        std::string body;
    };

    /**
     * @brief Memoria de respuestas por usuario y clave de idempotencia.
     *
     * Permite que un reintento de una operación no idempotente (mismo usuario, misma clave) reciba la
     * respuesta original en lugar de repetir el efecto.
     */
    class IdempotencyStore
    {
    public:
        virtual ~IdempotencyStore() = default;

        /// @brief Respuesta guardada para @p userId y @p key, o vacío si la clave no se usó antes.
        virtual std::optional<StoredResponse> find(std::uint64_t userId, const std::string& key) const = 0;

        /// @brief Recuerda la respuesta entregada para @p userId y @p key.
        virtual void save(std::uint64_t userId, const std::string& key, const StoredResponse& response) = 0;
    };

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_PORTS_OUT_IDEMPOTENCY_STORE_HPP
