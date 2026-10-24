#ifndef HUBFLOW_TESTS_SUPPORT_HTTP_HARNESS_HPP
#define HUBFLOW_TESTS_SUPPORT_HTTP_HARNESS_HPP

#include "support/applicationHarness.hpp"

#include "hubflow/platform/http/apiRouter.hpp"
#include "hubflow/platform/http/controller.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace hubflow::test
{

    /**
     * @brief Servidor HTTP real en un puerto efímero, conectado a la aplicación con adaptadores en memoria.
     *
     * Sirve para probar el contrato REST completo (autenticación, validación, idempotencia) sin base de datos.
     */
    class HttpHarness
    {
    public:
        HttpHarness();
        ~HttpHarness();

        HttpHarness(const HttpHarness&) = delete;
        HttpHarness& operator=(const HttpHarness&) = delete;

        /// @brief Inicia sesión por HTTP y devuelve el token.
        std::string login(const std::string& email, const std::string& password = "clave-segura");

        /// @brief Cabeceras con el token de @p email.
        httplib::Headers as(const std::string& email, httplib::Headers extra = {});

        /// @brief Cuerpo JSON de una respuesta.
        static nlohmann::json bodyOf(const httplib::Result& result);

        httplib::Client& client();

        Storage storage;
        ApplicationHarness app;
        InMemoryIdempotencyStore idempotency;
        std::uint64_t operadorId {0};
        std::uint64_t pumId {0};
        std::uint64_t otherPumId {0};

    private:
        httplib::Server m_server;
        std::mutex m_mutex;
        std::unique_ptr<platform::ApiRouter> m_spRouter;
        std::vector<std::unique_ptr<platform::Controller>> m_controllers;
        std::thread m_thread;
        std::unique_ptr<httplib::Client> m_spClient;
    };

} // namespace hubflow::test

#endif // HUBFLOW_TESTS_SUPPORT_HTTP_HARNESS_HPP
