SET NAMES utf8mb4;
-- El destinatario suma teléfono y código postal (para avisarle y confirmar su identidad al consultar el paquete).
-- Los avisos que se mandan a los clientes quedan registrados en `notifications` con el contacto enmascarado.
-- Docker sólo corre las migraciones sobre un volumen nuevo: para aplicarla sobre una base existente usar `make reset-db`.

ALTER TABLE shipments
    ADD COLUMN recipient_phone       VARCHAR(20) NOT NULL DEFAULT '' AFTER recipient,
    ADD COLUMN recipient_postal_code VARCHAR(8)  NOT NULL DEFAULT '' AFTER recipient_phone;

UPDATE shipments SET recipient_phone = '1100000000', recipient_postal_code = '1000' WHERE recipient_phone = '';

CREATE TABLE IF NOT EXISTS notifications (
    id               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    shipment_code    VARCHAR(16)     NOT NULL,
    channel          VARCHAR(16)     NOT NULL,
    recipient_masked VARCHAR(40)     NOT NULL,
    message          VARCHAR(400)    NOT NULL,
    sent_at          BIGINT          NOT NULL,
    KEY ix_notification_shipment (shipment_code)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;
