import { api } from "../api.js";
import * as session from "../session.js";
import { h, pageHead, loading, empty, errorBox, statusBadge, fmtDate, fmtKm, ALGORITHM } from "../ui.js";
import { navigate } from "../router.js";

export default async function routes(root) {
  const pum = session.role() === "PUM";
  const slot = h("div", {}, loading());
  root.append(pageHead(pum ? "Mi hoja de ruta" : "Rutas", pum ? "Las rutas que te asignaron, con el recorrido calculado y las paradas para marcar." : "Todas las rutas asignadas a repartidores."), slot);

  async function load() {
    try {
      const data = await api.get("/routes", { limit: 100 });
      if (!data.items.length) {
        return slot.replaceChildren(empty(pum ? "Todavía no tenés rutas" : "No hay rutas asignadas", pum ? "Cuando un operador te asigne un lote, la ruta aparece acá." : "Las rutas se asignan desde la Mesa de clasificación.",
          pum ? null : h("a", { class: "btn primary", href: "#/mesa" }, "Ir a la mesa")));
      }
      slot.replaceChildren(h("div", { class: "table-wrap" }, h("table", {},
        h("thead", {}, h("tr", {}, ...["Ruta", pum ? null : "Repartidor", "Cómo busca", "Estado", "Paradas", "Distancia", "Creada", ""].filter((t) => t !== null).map((t) => h("th", {}, t)))),
        h("tbody", {}, ...data.items.map((r) => h("tr", {},
          h("td", { class: "code" }, "#" + r.id), pum ? null : h("td", {}, r.courier.name), h("td", {}, ALGORITHM[r.algorithm] ?? r.algorithm), h("td", {}, statusBadge(r.status)),
          h("td", {}, `${r.resolvedStops} de ${r.stopCount}`), h("td", {}, fmtKm(r.totalCostKm)), h("td", {}, fmtDate(r.createdAt)),
          h("td", {}, h("a", { class: "btn small" + (r.status === "EN_CURSO" ? " primary" : ""), href: "#/rutas/" + r.id }, r.status === "EN_CURSO" ? "Abrir ruta" : "Ver ruta"))))))));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }
  load();
}
