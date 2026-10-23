-- Mapa de referencia de la ciudad. Coordenadas en km; el costo de cada arista es mayor o igual
-- a la distancia en línea recta, por lo que la heurística euclídea de A* es admisible.

INSERT INTO road_nodes (id, position, label, is_hub, zone, x, y) VALUES
  ('HUB', 1, 'Centro de Distribución', 1, 'CENTRO', 10.0, 10.0),
  ('C1', 2, 'Plaza Mayor', 0, 'CENTRO', 7.0, 10.0),
  ('C2', 3, 'Mercado Central', 0, 'CENTRO', 13.0, 11.0),
  ('C3', 4, 'Estación Norte', 0, 'CENTRO', 10.0, 7.0),
  ('C4', 5, 'Barrio Universitario', 0, 'CENTRO', 5.0, 7.0),
  ('N1', 6, 'Av. Libertador', 0, 'NORTE', 9.0, 5.0),
  ('N2', 7, 'Parque Norte', 0, 'NORTE', 13.0, 4.0),
  ('N3', 8, 'Villa Alta', 0, 'NORTE', 6.0, 2.0),
  ('N4', 9, 'Las Lomas', 0, 'NORTE', 11.0, 1.0),
  ('N5', 10, 'Puerto Norte', 0, 'NORTE', 16.0, 2.0),
  ('S1', 11, 'Av. del Sur', 0, 'SUR', 9.0, 14.0),
  ('S2', 12, 'Barrio Industrial', 0, 'SUR', 13.0, 15.0),
  ('S3', 13, 'Villa Sur', 0, 'SUR', 6.0, 17.0),
  ('S4', 14, 'Costanera', 0, 'SUR', 11.0, 18.0),
  ('S5', 15, 'Polo Sur', 0, 'SUR', 16.0, 17.0);

INSERT INTO road_edges (from_id, to_id, cost_km) VALUES
  ('HUB', 'C1', 3.0), ('HUB', 'C2', 3.7), ('HUB', 'C3', 3.9), ('HUB', 'S1', 6.0), ('C1', 'C3', 4.3),
  ('C2', 'C3', 5.8), ('C1', 'C4', 4.7), ('C3', 'N1', 3.3), ('C4', 'N3', 5.1), ('C4', 'N1', 5.2),
  ('N1', 'N3', 5.6), ('N1', 'N2', 6.0), ('N1', 'N4', 4.5), ('N3', 'N4', 5.9), ('N2', 'N4', 4.7),
  ('N2', 'N5', 5.3), ('N4', 'N5', 5.1), ('C2', 'N2', 8.1), ('C2', 'S2', 5.2), ('C1', 'S1', 6.5),
  ('S1', 'S2', 4.2), ('S1', 'S3', 4.9), ('S1', 'S4', 5.9), ('S2', 'S4', 5.3), ('S2', 'S5', 3.7),
  ('S3', 'S4', 5.9), ('S4', 'S5', 6.7), ('C1', 'S3', 10.3);
