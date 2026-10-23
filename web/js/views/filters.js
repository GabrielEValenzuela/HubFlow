import { api } from "../api.js";
import { h, subHead, loading, errorBox, field, busy, shipmentTable, ZONE, SERVICE, empty } from "../ui.js";
import { openShipment } from "./shipmentDetail.js";

const JOIN = {
  INTERSECTION: ["y", "y", "Cumplen las dos condiciones."],
  UNION: ["o", "o", "Cumplen al menos una de las dos."],
  DIFFERENCE: ["pero no", "pero no", "Cumplen la primera y no la segunda."],
};

export default function filters(root) {
  function condition(label, defType, defVal) {
    const type = h("select", { class: "input", "aria-label": `${label}: tipo` }, h("option", { value: "zone" }, "Zona"), h("option", { value: "service" }, "Servicio"));
    const val = h("select", { class: "input", "aria-label": `${label}: valor` });
    const fill = () => val.replaceChildren(...Object.entries(type.value === "zone" ? ZONE : SERVICE).map(([v, l]) => h("option", { value: v }, l)));
    type.value = defType;
    fill();
    val.value = defVal;
    type.addEventListener("change", fill);
    return {
      el: h("fieldset", { class: "crit" }, h("legend", {}, label), h("div", { class: "form-grid", style: "grid-template-columns:1fr 1fr" }, type, val)),
      key: () => `${type.value}:${val.value}`,
      text: () => `${type.selectedOptions[0].text} ${val.selectedOptions[0].text}`,
    };
  }

  const a = condition("Primera condición", "zone", "NORTE");
  const b = condition("Segunda condición", "service", "EXPRESS");
  const join = h("select", { class: "input" }, ...Object.entries(JOIN).map(([v, [, l]]) => h("option", { value: v }, l)));
  const joinHint = h("span", {}, JOIN.INTERSECTION[2]);
  join.addEventListener("change", () => { joinHint.textContent = JOIN[join.value][2]; });
  const btn = h("button", { class: "btn primary", type: "submit" }, "Mostrar paquetes");
  const out = h("div", { style: "margin-top:20px", "aria-live": "polite" });
  const form = h("form", { class: "card", novalidate: true },
    
    h("div", { class: "filter-row" }, a.el, h("div", { class: "join-box" }, field("join", "Combinar con", join, { hint: joinHint })), b.el),
    h("div", { class: "form-actions" }, btn));
  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    if (a.key() === b.key()) return out.replaceChildren(h("div", { class: "alert warn" }, h("div", {}, "Elegí dos condiciones distintas.")));
    out.replaceChildren(loading(3));
    try {
      const op = join.value;
      const r = await busy(btn, () => api.get("/shipment-set-operations", { operation: op, first: a.key(), second: b.key() }));
      const sentence = `${a.text()} ${JOIN[op][0]} ${b.text()}`;
      out.replaceChildren(h("section", { class: "card" },
        h("div", { class: "card-head" }, h("h2", {}, sentence), h("p", {}, `${r.shipments.length} ${r.shipments.length === 1 ? "paquete" : "paquetes"}`)),
        r.shipments.length
          ? shipmentTable(r.shipments, { extra: (s) => h("button", { class: "btn small", onClick: () => openShipment(s.code) }, "Ver paquete") })
          : empty("Ningún paquete cumple", "Probá con otra combinación de condiciones.")));
    } catch (err) {
      out.replaceChildren(errorBox(err));
    }
  });
  root.append(subHead("Lista por zona y servicio", "Combiná dos condiciones para ver solo los paquetes que te interesan."), form, out);
}
