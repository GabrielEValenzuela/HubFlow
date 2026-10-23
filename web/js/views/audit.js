import { api } from "../api.js";
import { h, pageHead, loading, errorBox, empty, fmtDate } from "../ui.js";

const LIMIT = 20;
const TYPES = {
  "route.assigned": "Ruta asignada",
  "route.stop_resolved": "Parada resuelta",
  "shipment.classified": "Envío clasificado",
  "shipment.delivered": "Envío entregado",
  "shipment.dispatched": "Envío despachado",
  "shipment.registered": "Envío registrado",
  "shipment.rescheduled": "Envío reprogramado",
  "user.changed": "Usuario modificado",
  "user.registered": "Usuario creado",
};

export default async function audit(root) {
  let offset = 0;
  const slot = h("div", {}, loading());
  root.append(pageHead("Auditoría", "Registro de eventos con usuario y fecha, del más reciente al más antiguo."), slot);

  async function load() {
    try {
      const d = await api.get("/audit-events", { limit: LIMIT, offset });
      if (!d.items.length && offset === 0) return slot.replaceChildren(empty("Todavía no hay actividad", "Los eventos se registran a medida que se opera."));
      const from = offset + 1, to = offset + d.items.length;
      slot.replaceChildren(
        h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, h("tr", {}, ...["Cuándo", "Quién", "Qué", "Sobre", "Detalle"].map((t) => h("th", {}, t)))),
          h("tbody", {}, ...d.items.map((e) => h("tr", {}, h("td", { style: "white-space:nowrap" }, fmtDate(e.occurredAt)), h("td", {}, e.actor.name), h("td", {}, TYPES[e.type] ?? e.type), h("td", { class: "code" }, e.subject), h("td", { class: "muted" }, e.detail)))))),
        h("nav", { class: "pager", "aria-label": "Paginación" }, h("span", { class: "muted small" }, `Mostrando ${from}–${to} de ${d.total}`),
          h("div", { class: "actions" }, h("button", { class: "btn small", disabled: offset === 0, onClick: () => { offset = Math.max(0, offset - LIMIT); load(); } }, "Anterior"),
            h("button", { class: "btn small", disabled: to >= d.total, onClick: () => { offset += LIMIT; load(); } }, "Siguiente"))));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }
  load();
}
