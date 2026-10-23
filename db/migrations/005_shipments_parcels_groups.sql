SET NAMES utf8mb4;
-- Cada envío pasa a tener una identidad interna (UUID versión 7) y un código de seguimiento público aparte.
-- Un envío se compone de uno o más bultos y varios envíos pueden formar un grupo que viaja junto.
-- Docker sólo corre las migraciones sobre un volumen nuevo: para aplicarla sobre una base existente usar `make reset-db`.

ALTER TABLE movements DROP FOREIGN KEY fk_movement_shipment;
ALTER TABLE route_stops DROP FOREIGN KEY fk_stop_shipment;
ALTER TABLE shipment_feature_links DROP FOREIGN KEY fk_link_shipment;

ALTER TABLE shipments DROP PRIMARY KEY;
ALTER TABLE shipments
    CHANGE code tracking_code VARCHAR(16) NOT NULL,
    ADD COLUMN id CHAR(36) NOT NULL FIRST;

UPDATE shipments
SET id = LOWER(CONCAT(
    SUBSTR(LPAD(HEX(UNIX_TIMESTAMP(created_at) * 1000), 12, '0'), 1, 8), '-',
    SUBSTR(LPAD(HEX(UNIX_TIMESTAMP(created_at) * 1000), 12, '0'), 9, 4), '-7',
    SUBSTR(HEX(RANDOM_BYTES(10)), 1, 3), '-8', SUBSTR(HEX(RANDOM_BYTES(10)), 1, 3), '-',
    SUBSTR(HEX(RANDOM_BYTES(10)), 1, 12)));

ALTER TABLE shipments MODIFY weight_kg DECIMAL(8, 2) NOT NULL;

ALTER TABLE shipments
    ADD PRIMARY KEY (id),
    ADD UNIQUE KEY uq_shipment_tracking_code (tracking_code);

ALTER TABLE movements MODIFY shipment_code VARCHAR(16) NOT NULL;
ALTER TABLE route_stops MODIFY shipment_code VARCHAR(16) NOT NULL;
ALTER TABLE shipment_feature_links MODIFY shipment_code VARCHAR(16) NOT NULL;

ALTER TABLE movements
    ADD CONSTRAINT fk_movement_shipment FOREIGN KEY (shipment_code) REFERENCES shipments (tracking_code);
ALTER TABLE route_stops
    ADD CONSTRAINT fk_stop_shipment FOREIGN KEY (shipment_code) REFERENCES shipments (tracking_code);
ALTER TABLE shipment_feature_links
    ADD CONSTRAINT fk_link_shipment FOREIGN KEY (shipment_code) REFERENCES shipments (tracking_code);

CREATE TABLE IF NOT EXISTS parcels (
    shipment_id CHAR(36)      NOT NULL,
    ordinal     INT UNSIGNED  NOT NULL,
    weight_kg   DECIMAL(6, 3) NOT NULL,
    length_cm   DECIMAL(6, 1) NOT NULL,
    width_cm    DECIMAL(6, 1) NOT NULL,
    height_cm   DECIMAL(6, 1) NOT NULL,
    delivered   TINYINT(1)    NOT NULL DEFAULT 0,
    PRIMARY KEY (shipment_id, ordinal),
    CONSTRAINT fk_parcel_shipment FOREIGN KEY (shipment_id) REFERENCES shipments (id)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

INSERT INTO parcels (shipment_id, ordinal, weight_kg, length_cm, width_cm, height_cm, delivered)
SELECT id, 1, weight_kg, 10, 10, 10, IF(status = 'ENTREGADO', 1, 0) FROM shipments;

CREATE TABLE IF NOT EXISTS shipment_groups (
    id             CHAR(36)    NOT NULL PRIMARY KEY,
    delivery_point VARCHAR(16) NOT NULL,
    created_at     TIMESTAMP   NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS shipment_group_members (
    group_id      CHAR(36)    NOT NULL,
    shipment_code VARCHAR(16) NOT NULL,
    position      INT UNSIGNED NOT NULL,
    PRIMARY KEY (group_id, shipment_code),
    UNIQUE KEY uq_group_member_shipment (shipment_code),
    CONSTRAINT fk_group_member_group FOREIGN KEY (group_id) REFERENCES shipment_groups (id) ON DELETE CASCADE,
    CONSTRAINT fk_group_member_shipment FOREIGN KEY (shipment_code) REFERENCES shipments (tracking_code)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS route_stop_companions (
    route_id      BIGINT UNSIGNED NOT NULL,
    stop_order    INT UNSIGNED    NOT NULL,
    shipment_code VARCHAR(16)     NOT NULL,
    PRIMARY KEY (route_id, stop_order, shipment_code),
    CONSTRAINT fk_companion_stop FOREIGN KEY (route_id, stop_order) REFERENCES route_stops (route_id, stop_order),
    CONSTRAINT fk_companion_shipment FOREIGN KEY (shipment_code) REFERENCES shipments (tracking_code)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;
