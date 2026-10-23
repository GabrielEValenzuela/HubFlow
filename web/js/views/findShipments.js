import { api } from "../api.js";
import { h, pageHead, errorBox, field, busy, setFieldError, shipmentTable, zoneTag, serviceBadge, statusBadge, fmtKg, empty } from "../ui.js";
import { icon } from "../icons.js";
import { openShipment } from "./shipmentDetail.js";
import filters from "./filters.js";

const CODE = /^PKG-\d{4}$/;

export default function findShipments(root) {
  const byCodeOut = h("div", { style: "margin-top:16px", "aria-live": "polite" });
  const rangeOut = h("div", { style: "margin-top:16px", "aria-live": "polite" });

  const code = h("input", { class: "input", placeholder: "PKG-1003", autocomplete: "off", style: "width:170px" });
  const codeBtn = h("button", { class: "btn primary", type: "submit" }, icon("search"), "Buscar");
  const codeForm = h("form", { novalidate: true, class: "actions", style: "align-items:flex-end" }, field("code", "Código de seguimiento", code), codeBtn);
  codeForm.addEventListener("submit", async (e) => {
    e.preventDefault();
    const c = code.value.trim().toUpperCase();
    if (!CODE.test(c)) return setFieldError(codeForm, "code", "Usá el formato PKG-0000.");
    setFieldError(codeForm, "code", "");
    try {
      const r = await busy(codeBtn, () => api.get("/indexes/bplus/entries/" + c));
      if (!r.found) return byCodeOut.replaceChildren(h("div", { class: "alert warn" }, h("div", {}, `No hay un paquete con el código ${c}.`)));
      const s = r.shipment;
      byCodeOut.replaceChildren(h("article", { class: "found-card" },
        h("div", {}, h("div", { class: "code found-code" }, s.code), h("div", {}, s.recipient)),
        h("div", { class: "found-meta" }, zoneTag(s.zone), serviceBadge(s.service), statusBadge(s.status), h("span", { class: "muted" }, fmtKg(s.weightKg))),
        h("button", { class: "btn small", onClick: () => openShipment(s.code) }, "Ver paquete")));
    } catch (err) {
      byCodeOut.replaceChildren(errorBox(err));
    }
  });

  const from = h("input", { class: "input", placeholder: "PKG-1002", autocomplete: "off", style: "width:150px" });
  const to = h("input", { class: "input", placeholder: "PKG-1005", autocomplete: "off", style: "width:150px" });
  const rangeBtn = h("button", { class: "btn primary", type: "submit" }, icon("search"), "Buscar");
  const rangeForm = h("form", { novalidate: true, class: "actions", style: "align-items:flex-end" }, field("from", "Desde el código", from), field("to", "Hasta el código", to), rangeBtn);
  rangeForm.addEventListener("submit", async (e) => {
    e.preventDefault();
    const a = from.value.trim().toUpperCase(), b = to.value.trim().toUpperCase();
    let ok = true;
    if (!CODE.test(a)) { setFieldError(rangeForm, "from", "Usá el formato PKG-0000."); ok = false; } else setFieldError(rangeForm, "from", "");
    if (!CODE.test(b)) { setFieldError(rangeForm, "to", "Usá el formato PKG-0000."); ok = false; } else setFieldError(rangeForm, "to", "");
    if (ok && a > b) { setFieldError(rangeForm, "from", "El código inicial no puede ser mayor que el final."); ok = false; }
    if (!ok) return;
    try {
      const r = await busy(rangeBtn, () => api.get("/indexes/bplus/entries", { from: a, to: b }));
      rangeOut.replaceChildren(r.items.length
        ? h("div", {}, h("p", { class: "muted small", style: "margin-bottom:8px" }, `${r.items.length} ${r.items.length === 1 ? "paquete" : "paquetes"} entre ${a} y ${b}.`),
          shipmentTable(r.items, { extra: (s) => h("button", { class: "btn small", onClick: () => openShipment(s.code) }, "Ver paquete") }))
        : empty("No hay paquetes en ese rango", `No se registró ningún código entre ${a} y ${b}.`));
    } catch (err) {
      rangeOut.replaceChildren(errorBox(err));
    }
  });

  const filterSection = h("section", { style: "margin-top:var(--s-6)" });
  filters(filterSection);
  root.append(pageHead("Buscar paquete", "Encontrá un paquete por su código o listá los de un tramo de códigos."),
    h("div", { class: "grid-2" },
      h("section", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Por código")), codeForm, byCodeOut),
      h("section", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Por tramo de códigos")), rangeForm, rangeOut)),
    filterSection);
}
