import { api } from "../api.js";
import * as session from "../session.js";
import { h, pageHead, loading, empty, errorBox, statusBadge, fmtDate, fmtKm } from "../ui.js";
import { optionLabel } from "../lib/routeOptions.js";
import { reassignRoute } from "./reassign.js";

/** Repartos: avance de cada chofer, con la posibilidad de pasar un reparto en curso a otro chofer. */
export default async function deliveries(root) {
  const admin = session.role() === "ADMIN";
  const slot = h("div", {}, loading());
  root.append(pageHead(admin ? "Repartos" : "Repartos en curso",
    "Qué está haciendo cada chofer ahora: paradas hechas, no entregadas y paquetes que vuelven a la planta. Si un chofer no puede seguir, pasá su reparto a otro."), slot);

  async function load() {
    try {
      const list = await api.get("/routes", { limit: 100 });
      const routes = await Promise.all(list.items.map((r) => api.get("/routes/" + r.id)));
      const active = routes.filter((r) => r.status === "EN_CURSO").sort((a, b) => b.id - a.id);
      const done = routes.filter((r) => r.status !== "EN_CURSO").sort((a, b) => b.id - a.id);
      if (!routes.length) {
        return slot.replaceChildren(empty("Todavía no hay repartos", admin ? "Aparecen cuando el jefe de planta asigna un reparto a un chofer." : "Armá una carga y asignala a un chofer para que salga a repartir.",
          admin ? null : h("a", { class: "btn primary", href: "#/carga" }, "Armar carga")));
      }
      slot.replaceChildren(h("div", { class: "flow" },
        h("section", {}, h("h2", { style: "margin-bottom:12px" }, "En curso"),
          active.length ? h("div", { class: "grid-3" }, ...active.map(card)) : empty("No hay repartos en curso", "Los choferes no tienen paradas pendientes en este momento.")),
        done.length ? h("section", {}, h("h2", { style: "margin-bottom:12px" }, "Terminados"), h("div", { class: "grid-3" }, ...done.map(card))) : null));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }

  function card(r) {
    const ok = r.stops.filter((s) => s.outcome === "ENTREGADO").length;
    const back = r.stops.filter((s) => s.outcome === "REPROGRAMADO");
    const waiting = r.stops.length - ok - back.length;
    const pct = r.stopCount ? Math.round((r.resolvedStops / r.stopCount) * 100) : 0;
    return h("article", { class: "card delivery" },
      h("div", { class: "card-head" }, h("div", {}, h("h3", {}, r.courier.name), h("p", {}, `Reparto #${r.id} · ${fmtKm(r.totalCostKm)} · ${optionLabel(r.algorithm)}`)), statusBadge(r.status)),
      h("div", { class: "progress", role: "progressbar", "aria-valuemin": "0", "aria-valuemax": String(r.stopCount), "aria-valuenow": String(r.resolvedStops), "aria-label": `Paradas hechas por ${r.courier.name}` }, h("span", { style: `width:${pct}%` })),
      h("p", { class: "small", style: "margin-top:6px" }, `${r.resolvedStops} de ${r.stopCount} paradas hechas`),
      h("dl", { class: "kv", style: "margin-top:10px" },
        h("dt", {}, "Entregados"), h("dd", {}, String(ok)),
        h("dt", {}, "No entregados"), h("dd", {}, String(back.length)),
        h("dt", {}, "Faltan"), h("dd", {}, String(waiting)),
        h("dt", {}, "Salió"), h("dd", {}, fmtDate(r.createdAt))),
      back.length ? h("div", { class: "alert warn", style: "margin-top:10px" }, h("div", {}, h("strong", {}, "Vuelven a la planta: "), back.map((s) => s.shipmentCode).join(", "))) : null,
      h("div", { class: "actions", style: "margin-top:12px" }, h("a", { class: "btn small", href: "#/repartos/" + r.id }, "Ver detalle"),
        r.status === "EN_CURSO" ? h("button", { class: "btn small", onClick: () => reassignRoute(r, load) }, "Pasar a otro chofer") : null));
  }
  load();
}
