import { api } from "../api.js";
import { h, svg, pageHead, loading, errorBox, field, busy, setFieldError, shipmentTable, bitChips, ZONE, SERVICE, toast, tabs, empty } from "../ui.js";
import { icon } from "../icons.js";

const OPS = { UNION: "Unión · está en A o en B", INTERSECTION: "Intersección · está en A y en B", DIFFERENCE: "Diferencia · está en A pero no en B" };

export default async function queries(root) {
  let mode = "sets";
  const slot = h("div", {});
  const bar = tabs([["sets", "Set de códigos"], ["bits", "BitSet de estados"]], mode, (id) => { mode = id; draw(); });
  root.append(pageHead("Consultas", "Operaciones de conjuntos sobre códigos de envío (Set sobre lista ordenada sin repetidos; unión, intersección y diferencia por merge, O(n + m)) y comparación del historial de estados de dos envíos con BitSet de 5 bits (AND, OR, XOR)."), bar, slot);
  const draw = () => slot.replaceChildren(mode === "sets" ? sets() : bits());

  function criterion(prefix, label, defType, defVal) {
    const type = h("select", { class: "input", "aria-label": `${label}: criterio` }, h("option", { value: "zone" }, "Zona"), h("option", { value: "service" }, "Servicio"));
    const val = h("select", { class: "input", "aria-label": `${label}: valor` });
    const fill = () => {
      const src = type.value === "zone" ? ZONE : SERVICE;
      val.replaceChildren(...Object.entries(src).map(([v, l]) => h("option", { value: v }, l)));
    };
    type.value = defType;
    fill();
    val.value = defVal;
    type.addEventListener("change", fill);
    return { type, val, el: h("fieldset", { class: "crit" }, h("legend", {}, label), h("div", { class: "form-grid", style: "grid-template-columns:1fr 1fr" }, type, val)), get: () => `${type.value}:${val.value}` };
  }

  function sets() {
    const a = criterion("a", "Grupo A", "zone", "NORTE");
    const b = criterion("b", "Grupo B", "service", "EXPRESS");
    const op = h("select", { class: "input" }, ...Object.entries(OPS).map(([v, l]) => h("option", { value: v }, l)));
    const btn = h("button", { class: "btn primary", type: "submit" }, "Calcular");
    const out = h("div", { style: "margin-top:20px" });
    const form = h("form", { class: "card", novalidate: true }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Construir y combinar conjuntos"), h("p", {}, "Cada conjunto A o B contiene los códigos de los envíos de una zona o de un servicio."))),
      h("div", { class: "grid-2", style: "gap:12px" }, a.el, b.el), h("div", { style: "margin-top:12px;max-width:420px" }, field("operation", "Operación", op)), h("div", { class: "form-actions" }, btn));
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      if (a.get() === b.get()) { out.replaceChildren(h("div", { class: "alert warn" }, "Elegí dos conjuntos distintos.")); return; }
      out.replaceChildren(loading(3));
      try {
        const r = await busy(btn, () => api.get("/shipment-set-operations", { operation: op.value, first: a.get(), second: b.get() }));
        out.replaceChildren(result(r, op.value, a, b));
      } catch (err) {
        out.replaceChildren(errorBox(err));
      }
    });
    return h("div", {}, form, out);
  }

  function result(r, op, a, b) {
    const A = new Set(r.first.codes), B = new Set(r.second.codes);
    const onlyA = r.first.codes.filter((c) => !B.has(c));
    const both = r.first.codes.filter((c) => B.has(c));
    const onlyB = r.second.codes.filter((c) => !A.has(c));
    const lbl = (c) => `${c.type.selectedOptions[0].text} ${c.val.selectedOptions[0].text}`;
    const nameA = lbl(a), nameB = lbl(b);
    return h("div", { class: "stack" },
      h("div", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, OPS[op].split(" · ")[0]), h("p", {}, `${r.result.size} ${r.result.size === 1 ? "envío" : "envíos"} en el resultado`)),
        h("div", { class: "venn-wrap" }, venn(op, onlyA.length, both.length, onlyB.length, nameA, nameB),
          h("div", { class: "venn-lists" }, listBlock("Solo en A", onlyA, op !== "INTERSECTION"), listBlock("En A y en B", both, true), listBlock("Solo en B", onlyB, op === "UNION")))),
      r.shipments.length ? shipmentTable(r.shipments) : empty("Ningún envío cumple", "Probá con otra combinación de conjuntos."));
  }

  const listBlock = (title, codes, inResult) => h("div", {}, h("h3", {}, title, " ", h("span", { class: "tag" }, inResult ? "forma parte del resultado" : "queda afuera")),
    codes.length ? h("div", { class: "chips", style: "margin-top:6px" }, ...codes.map((c) => h("span", { class: "chip" }, c))) : h("p", { class: "muted small" }, "Ninguno"));

  function venn(op, nA, nI, nB, nameA, nameB) {
    const inA = op !== "INTERSECTION";
    const inI = true;
    const inB = op === "UNION";
    return svg("svg", { class: "venn", viewBox: "0 0 360 220", role: "img", "aria-label": `Diagrama de conjuntos: solo A ${nA}, ambos ${nI}, solo B ${nB}` },
      svg("defs", {}, svg("clipPath", { id: "clipA" }, svg("circle", { cx: 130, cy: 105, r: 80 })), svg("clipPath", { id: "clipB" }, svg("circle", { cx: 230, cy: 105, r: 80 }))),
      svg("circle", { cx: 130, cy: 105, r: 80, class: "vc" + (inA ? " on" : "") }), svg("circle", { cx: 230, cy: 105, r: 80, class: "vc" + (inB ? " on" : "") }),
      svg("g", { "clip-path": "url(#clipA)" }, svg("circle", { cx: 230, cy: 105, r: 80, class: "vi" + (inI ? " on" : "") })),
      svg("text", { x: 90, y: 112, class: "vn", "text-anchor": "middle" }, String(nA)), svg("text", { x: 180, y: 112, class: "vn", "text-anchor": "middle" }, String(nI)), svg("text", { x: 270, y: 112, class: "vn", "text-anchor": "middle" }, String(nB)),
      svg("text", { x: 100, y: 20, class: "vl", "text-anchor": "middle" }, "A · " + nameA), svg("text", { x: 260, y: 20, class: "vl", "text-anchor": "middle" }, "B · " + nameB));
  }

  function bits() {
    const first = h("input", { class: "input", placeholder: "PKG-1001", autocomplete: "off" });
    const second = h("input", { class: "input", placeholder: "PKG-1002", autocomplete: "off" });
    const btn = h("button", { class: "btn primary", type: "submit" }, "Comparar");
    const out = h("div", { style: "margin-top:20px" });
    const form = h("form", { class: "card", novalidate: true }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Comparar dos historiales"), h("p", {}, "Cada envío guarda un BitSet de 5 bits, uno por estado alcanzado. Se combinan con AND (comunes), OR (de cualquiera) y XOR (exclusivos)."))),
      h("div", { class: "form-grid" }, field("first", "Primer envío", first), field("second", "Segundo envío", second)), h("div", { class: "form-actions" }, btn));
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      let ok = true;
      for (const [n, i] of [["first", first], ["second", second]]) {
        const bad = !/^PKG-\d{4}$/.test(i.value.trim().toUpperCase());
        setFieldError(form, n, bad ? "Usá el formato PKG-0000." : "");
        if (bad) ok = false;
      }
      if (!ok) return;
      try {
        const f = first.value.trim().toUpperCase(), sc = second.value.trim().toUpperCase();
        const [r, sf, ss] = await busy(btn, () => Promise.all([api.get("/state-history-comparisons", { first: f, second: sc }), api.get(`/shipments/${f}/state-history`), api.get(`/shipments/${sc}/state-history`)]));
        const row = (t, d, s) => h("tr", {}, h("td", {}, h("strong", {}, t), h("div", { class: "muted small" }, d)), h("td", {}, bitChips(s)));
        out.replaceChildren(h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, h("tr", {}, h("th", {}, "Qué muestra"), h("th", {}, "Estados"))), h("tbody", {},
          row(r.first, "Primer envío", sf), row(r.second, "Segundo envío", ss),
          row("Estados en común", "AND: ambos lo alcanzaron", r.common), row("Estados de cualquiera", "OR: al menos uno lo alcanzó", r.either),
          row("Solo el primero", "Primero AND NOT segundo", r.onlyFirst), row("Solo el segundo", "Segundo AND NOT primero", r.onlySecond), row("Distintos entre sí", "XOR: lo alcanzó solo uno de los dos", r.exclusive)))));
      } catch (err) {
        out.replaceChildren(errorBox(err));
      }
    });
    return h("div", {}, form, out);
  }
  draw();
}
