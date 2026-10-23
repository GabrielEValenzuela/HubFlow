#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_CUSTOMER_NOTIFIER_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_CUSTOMER_NOTIFIER_HPP

#include <cstdint>
#include <string>

namespace hubflow::tracking::application
{

/**
 * @brief Aviso para un cliente: qué paquete, por qué canal, a qué contacto (siempre enmascarado), qué se le dice y
 * cuándo.
 */
struct CustomerNotice
{
    std::string  shipmentCode;
    std::string  channel;
    std::string  recipient;
    std::string  message;
    std::int64_t sentAt {0};
};

/**
 * @brief Envía avisos a los clientes (puerto).
 *
 * El aviso nunca lleva el contacto completo: el adaptador que de verdad lo envíe y que necesite el número real deberá
 * obtenerlo por su cuenta, para que nada de lo que se registra o se muestra lo revele.
 */
class CustomerNotifier
{
public:
    virtual ~CustomerNotifier() = default;

    virtual void send(const CustomerNotice& notice) = 0;
};

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_CUSTOMER_NOTIFIER_HPP
