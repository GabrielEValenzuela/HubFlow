import { api } from "../api.js";
import { h, pageHead, loading, errorBox, empty, fmtDate, ROLE } from "../ui.js";

const LIMIT = 20;
const code = (e) => e.subject;
const route = (e) => e.subject.replace("Ruta ", "reparto ");

/** Cada evento se cuenta como una oración corta: quién hizo qué. */
const SENTENCE = {
  "shipment.registered": (e) => `registró el paquete ${code(e)}`,
  "shipment.classified": (e) => `clasificó el paquete ${code(e)}`,
  "shipment.dispatched": (e) => `despachó el paquete ${code(e)}`,
  "shipment.delivered": (e) => `entregó el paquete ${code(e)}`,
  "shipment.rescheduled": (e) => `dejó para reintentar el paquete ${code(e)}`,
  "route.assigned": (e) => {
    const [courier, , stops] = e.detail.split(" · ");
    return `asignó el ${route(e)} a ${courier}${stops ? ` (${stops})` : ""}`;
  },
  "route.stop_resolved": (e) => {
    const [pkg, outcome] = e.detail.split(" · ");
    return outcome === "ENTREGADO" ? `entregó el paquete ${pkg} en el ${route(e)}` : `no pudo entregar el paquete ${pkg} en el ${route(e)}`;
  },
  "user.registered": (e) => `dio de alta a ${e.subject} como ${ROLE[e.detail] ?? e.detail}`,
  "user.changed": (e) => {
    const [role, status] = e.detail.split(" · ");
    return `actualizó a ${e.subject}: ${ROLE[role] ?? role}, ${status === "activo" ? "con acceso" : "sin acceso"}`;
  },
};

export default async function activity(root) {
  let offset = 0;
  const slot = h("div", {}, loading());
  root.append(pageHead("Actividad", "Lo que hizo cada persona, del más reciente al más antiguo."), slot);

  async function load() {
    try {
      const d = await api.get("/audit-events", { limit: LIMIT, offset });
      if (!d.items.length && offset === 0) return slot.replaceChildren(empty("Todavía no hay actividad", "Lo que haga el personal va a aparecer acá."));
      const resolved = d.items.filter((e) => e.type === "route.stop_resolved");
      // Resolver una parada también deja su propio aviso de paquete: se muestra una sola oración.
      const duplicate = (e) => ["shipment.delivered", "shipment.rescheduled"].includes(e.type) &&
        resolved.some((r) => r.detail.startsWith(e.subject + " ") && r.actor.name === e.actor.name && Math.abs(r.occurredAt - e.occurredAt) <= 2);
      const items = d.items.filter((e) => !duplicate(e));
      const from = offset + 1, to = offset + d.items.length;
      slot.replaceChildren(
        h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, h("tr", {}, h("th", {}, "Cuándo"), h("th", {}, "Qué pasó"))),
          h("tbody", {}, ...items.map((e) => h("tr", {}, h("td", { style: "white-space:nowrap" }, fmtDate(e.occurredAt)),
            h("td", {}, h("strong", {}, e.actor.name), " ", (SENTENCE[e.type]?.(e) ?? e.detail ?? "hizo un cambio") + "."))))),
        ),
        h("nav", { class: "pager", "aria-label": "Páginas" }, h("span", { class: "muted small" }, `Se ven del ${from} al ${to}, de ${d.total}`),
          h("div", { class: "actions" }, h("button", { class: "btn small", disabled: offset === 0, onClick: () => { offset = Math.max(0, offset - LIMIT); load(); } }, "Más recientes"),
            h("button", { class: "btn small", disabled: to >= d.total, onClick: () => { offset += LIMIT; load(); } }, "Más antiguos"))));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }
  load();
}
