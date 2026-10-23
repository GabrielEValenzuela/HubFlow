// Opciones de ruta en lenguaje de negocio. La API sigue usando los valores técnicos (clave `algorithm`).
import { api } from "../api.js";

// Supuestos de la estimación de tiempo (un solo lugar). Son una estimación, no una medición.
export const ETA = { avgKmh: 25, minutesPerStop: 4, note: "Estimación: 25 km/h promedio urbano + 4 min por parada." };

export const ROUTE_OPTIONS = [
  { algorithm: "ASTAR", label: "Menos kilómetros", short: "La ruta más corta, garantizada", recommended: true },
  { algorithm: "BFS", label: "Menos cruces", short: "Pasa por menos intersecciones; puede sumar algunos km" },
  { algorithm: "GREEDY", label: "Salida inmediata", short: "Prioriza el punto más cercano al destino en línea recta; calcula rápido y puede sumar km" },
];
export const DEFAULT_OPTION = "ASTAR";

/** Nombre de la opción elegida; DFS no se ofrece, pero una ruta antigua puede traerlo. */
export const optionLabel = (algorithm) => ROUTE_OPTIONS.find((o) => o.algorithm === algorithm)?.label ?? "Recorrido de prueba";

export const etaMinutes = (km, stops) => Math.round((km / ETA.avgKmh) * 60 + stops * ETA.minutesPerStop);

export function fmtDuration(min) {
  if (min < 60) return `${min} min`;
  return `${Math.floor(min / 60)} h ${String(min % 60).padStart(2, "0")} min`;
}

/** Tramos HUB -> paradas -> HUB, en el orden de descarga (el de la pila del lote). Omite tramos de largo cero. */
export function legsFor(stopNodes) {
  const nodes = ["HUB", ...stopNodes, "HUB"];
  const legs = [];
  for (let i = 1; i < nodes.length; i++) if (nodes[i - 1] !== nodes[i]) legs.push([nodes[i - 1], nodes[i]]);
  return legs;
}

/** Calcula km, cruces, paradas y tiempo de cada opción para la carga actual. */
export async function compareOptions(stopNodes) {
  const legs = legsFor(stopNodes);
  const results = await Promise.all(ROUTE_OPTIONS.map(async (o) => {
    const parts = await Promise.all(legs.map(([from, to]) => api.get("/route-previews", { from, to, algorithm: o.algorithm })));
    const km = parts.reduce((s, p) => s + p.metrics.cost_total, 0);
    const crossings = parts.reduce((s, p) => s + Math.max(0, p.path.length - 2), 0);
    return { ...o, km, crossings, stops: stopNodes.length, minutes: etaMinutes(km, stopNodes.length) };
  }));
  const best = Math.min(...results.map((r) => r.km));
  const most = results.reduce((a, b) => (b.km > a.km ? b : a));
  return results.map((r) => ({ ...r, reason: reasonFor(r, results, best, most) }));
}

const pct = (a, b) => Math.round(((a - b) / b) * 100);

function reasonFor(r, all, best, most) {
  if (r.km - best > 0.05) {
    const extra = pct(r.km, best);
    const fewer = Math.min(...all.map((o) => o.crossings));
    const base = `${Math.max(extra, 1)}% más km que Menos kilómetros`;
    return r.crossings === fewer && fewer < all.find((o) => o.algorithm === "ASTAR").crossings ? `${base}, pero con menos cruces` : base;
  }
  if (r.recommended && most.km - best > 0.05) return `${Math.max(Math.round(((most.km - best) / most.km) * 100), 1)}% menos km que ${most.label}`;
  return r.recommended ? "Igual de corta que las demás para esta carga" : "Mismos km que la opción recomendada";
}
