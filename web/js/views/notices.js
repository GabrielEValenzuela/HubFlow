import { api } from "../api.js";
import { h, pageHead, loading, errorBox, empty, fmtDate } from "../ui.js";

const LIMIT = 20;

export default async function notices(root) {
  let offset = 0;
  const slot = h("div", {}, loading());
  root.append(
    pageHead("Avisos a clientes", "Los mensajes que el sistema le manda a quien espera un paquete, del más reciente al más antiguo. Por ahora no se envían de verdad: acá se ve lo que se habría enviado."),
    slot);

  async function load() {
    try {
      const d = await api.get("/customer-notices", { limit: LIMIT, offset });
      if (!d.items.length && offset === 0) return slot.replaceChildren(empty("Todavía no se avisó a ningún cliente", "Cuando ingrese, salga a reparto o se entregue un paquete, el aviso va a aparecer acá."));
      const from = offset + 1, to = offset + d.items.length;
      slot.replaceChildren(
        h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, h("tr", {}, h("th", {}, "Cuándo"), h("th", {}, "Paquete"), h("th", {}, "A quién"), h("th", {}, "Qué se le dijo"))),
          h("tbody", {}, ...d.items.map((n) => h("tr", {},
            h("td", { style: "white-space:nowrap" }, fmtDate(n.sentAt)),
            h("td", { class: "code" }, n.shipmentCode),
            h("td", {}, `Mensaje de texto al teléfono terminado en ${n.recipient.replace(/\*/g, "")}`),
            h("td", {}, n.message.replace(/^Paquete [A-Z0-9-]+: /, ""))))))),
        h("nav", { class: "pager", "aria-label": "Páginas" }, h("span", { class: "muted small" }, `Se ven del ${from} al ${to}, de ${d.total}`),
          h("div", { class: "actions" }, h("button", { class: "btn small", disabled: offset === 0, onClick: () => { offset = Math.max(0, offset - LIMIT); load(); } }, "Más recientes"),
            h("button", { class: "btn small", disabled: to >= d.total, onClick: () => { offset += LIMIT; load(); } }, "Más antiguos"))));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }
  load();
}
