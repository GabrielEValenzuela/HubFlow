import { api } from "../js/api.js";
import { h, subHead, loading, errorBox, empty, serviceBadge, zoneTag, fmtKg, ZONE } from "../js/ui.js";

const RANK = { EXPRESS: 0, PRIORITARIO: 1, ESTANDAR: 2 };

/** Resumen de una zona por recursión sobre la cadena de nodos; réplica en el cliente de summarizeZone. */
function summarize(items, i, zone) {
  if (i >= items.length) return { count: 0, express: 0, kg: 0 };
  const rest = summarize(items, i + 1, zone);
  if (items[i].zone !== zone) return rest;
  return { count: rest.count + 1, express: rest.express + (items[i].service === "EXPRESS" ? 1 : 0), kg: rest.kg + items[i].weightKg };
}

export default async function pendingList(root) {
  const slot = h("div", {}, loading());
  root.append(subHead("Lista de pendientes",
    "Lista simplemente enlazada con prioridad estable: cada envío pendiente es un nodo con puntero al siguiente; se despacha por la cabeza."), slot);

  async function load() {
    try {
      const data = await api.get("/pending-shipments", { limit: 200 });
      const zones = Object.keys(ZONE);
      const server = await Promise.all(zones.map((z) => api.get(`/zones/${z}/summary`)));
      slot.replaceChildren(view(data.items, zones, server));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }

  function view(items, zones, server) {
    let stable = true;
    for (let i = 1; i < items.length; i++) if (RANK[items[i - 1].service] > RANK[items[i].service]) stable = false;
    const chain = items.length
      ? h("ol", { class: "chain", "aria-label": "Nodos de la lista, de la cabeza a la cola" },
        ...items.map((s, i) => h("li", { class: "node-card" },
          h("span", { class: "code" }, s.code), serviceBadge(s.service),
          h("small", { class: "muted" }, i === 0 ? "cabeza (sale primero)" : i === items.length - 1 ? "cola (puntero a null)" : "next →"))))
      : empty("La lista está vacía", "No hay nodos: la cabeza y la cola son null.");
    return h("div", { class: "flow" },
      h("section", { class: "card" },
        h("div", { class: "card-head" }, h("div", {}, h("h3", {}, "Cadena de nodos"), h("p", {}, `${items.length} nodos. Orden de la cadena: prioridad (Express, Prioritario, Estándar) y, dentro de la misma prioridad, orden de alta.`))),
        chain,
        h("div", { class: stable ? "alert info" : "alert warn", style: "margin-top:12px" }, h("div", {}, stable
          ? "Verificación en el cliente: la prioridad no decrece a lo largo de la cadena."
          : "La prioridad decrece en algún punto de la cadena: la inserción no es estable."))),
      h("section", { class: "card" },
        h("h3", {}, "Inserción estable"),
        h("p", { class: "small", style: "margin:6px 0" }, "PendingShipments::insert recorre la cadena hasta el último nodo con prioridad mayor o igual a la del nuevo y lo enlaza a continuación. Con prioridades iguales el nuevo queda detrás, por eso se conserva el orden de alta."),
        h("p", { class: "small muted" }, "Costos: insertar O(1) en el mejor caso (cabeza) y O(n) en el peor; despachar (popFront) O(1); quitar un nodo concreto O(n); listar O(n). Se mantiene puntero a la cola.")),
      h("section", { class: "card" },
        h("div", { class: "card-head" }, h("div", {}, h("h3", {}, "Resumen recursivo por zona"), h("p", {}, "summarizeZone(nodo): caso base, nodo nulo, devuelve (0, 0, 0). Paso recursivo: resume el resto de la cadena y suma el nodo actual si es de la zona. O(n) en tiempo y O(n) de pila de llamadas."))),
        h("div", { class: "table-wrap" }, h("table", {},
          h("thead", {}, h("tr", {}, ...["Zona", "Cantidad (API)", "Express (API)", "Peso (API)", "Recursión en el cliente", "Coincide"].map((t) => h("th", {}, t)))),
          h("tbody", {}, ...zones.map((z, i) => {
            const r = summarize(items, 0, z), s = server[i];
            const ok = r.count === s.count && r.express === s.expressCount && Math.abs(r.kg - s.totalWeightKg) < 0.01;
            return h("tr", {}, h("td", {}, zoneTag(z)), h("td", { class: "code" }, String(s.count)), h("td", { class: "code" }, String(s.expressCount)), h("td", { class: "code" }, fmtKg(s.totalWeightKg)),
              h("td", { class: "code" }, `${r.count} · ${r.express} · ${fmtKg(Math.round(r.kg * 100) / 100)}`), h("td", {}, h("span", { class: "badge " + (ok ? "s-ENTREGADO" : "s-REPROGRAMADO") }, ok ? "Sí" : "No")));
          })))),
        h("p", { class: "muted small", style: "margin-top:8px" }, "El envío más pesado de cada zona usa la misma recursión (heaviestInZone): compara el nodo actual con el máximo del resto.")));
  }
  load();
}
