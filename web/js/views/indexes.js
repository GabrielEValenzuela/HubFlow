import { api } from "../api.js";
import { drawBst, drawBPlus } from "../lib/tree.js";
import { h, pageHead, loading, errorBox, field, busy, setFieldError, tabs, shipmentTable, toast, zoneTag, statusBadge, serviceBadge, fmtKg } from "../ui.js";
import { icon } from "../icons.js";

const CODE = /^PKG-\d{4}$/;

export default async function indexes(root) {
  const slot = h("div", {}, loading());
  let active = "bst";
  const bar = tabs([["bst", "Árbol binario de búsqueda"], ["bplus", "Árbol B+ (orden 4)"]], active, (id) => { active = id; draw(); });
  root.append(pageHead("Índices", "Dos formas de encontrar un envío por su código sin revisar todos: cada una con su recorrido a la vista."), bar, slot);

  async function draw() {
    slot.replaceChildren(loading());
    try {
      slot.replaceChildren(active === "bst" ? await bstPanel() : await bplusPanel());
    } catch (e) {
      slot.replaceChildren(errorBox(e, draw));
    }
  }

  function codeForm(label, onSubmit, extra) {
    const input = h("input", { class: "input", placeholder: "PKG-1003", autocomplete: "off", style: "width:170px" });
    const btn = h("button", { class: "btn primary", type: "submit" }, icon("search"), label);
    const form = h("form", { novalidate: true, class: "actions", style: "align-items:flex-end" }, field("code", "Código", input), btn, extra);
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      const c = input.value.trim().toUpperCase();
      if (!CODE.test(c)) return setFieldError(form, "code", "Usá el formato PKG-0000.");
      setFieldError(form, "code", "");
      try {
        await busy(btn, () => onSubmit(c));
      } catch (err) {
        toast(err.message, "error");
      }
    });
    return form;
  }

  async function bstPanel() {
    const t = await api.get("/indexes/bst");
    const scroll = h("div", { class: "tree-scroll" });
    const info = h("div", { style: "margin-top:12px" });
    const paint = (opts) => scroll.replaceChildren(drawBst(t.root, opts));
    paint({});
    const form = codeForm("Buscar en el árbol", async (code) => {
      const r = await api.get("/indexes/bst/entries/" + code);
      paint({ path: r.path, found: r.found ? code : null });
      info.replaceChildren(r.found
        ? h("div", { class: "alert info" }, h("div", {}, h("strong", {}, `${code} encontrado mirando ${r.visited} ${r.visited === 1 ? "nodo" : "nodos"}.`), h("div", {}, `Camino: ${r.path.map((p) => (p.direction === "encontrado" ? p.code : `${p.code} (${p.direction})`)).join(" → ")}`)))
        : h("div", { class: "alert warn" }, `${code} no está en el índice. Se miraron ${r.visited} nodos.`));
    });
    return h("div", { class: "stack" },
      h("div", { class: "tiles" }, tile("Envíos indexados", t.size), tile("Altura", t.height, "niveles"), h("div", { class: "tile", style: "grid-column: 1 / -1" }, h("div", { class: "k" }, "Recorrido en orden (de menor a mayor)"), h("div", { class: "chips", style: "margin-top:6px" }, ...t.inOrder.map((c) => h("span", { class: "chip" }, c))))),
      h("div", { class: "card" }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Buscar un código"), h("p", {}, "En cada nodo se decide si seguir por la izquierda (menores) o la derecha (mayores)."))), form, info),
      scroll, h("div", { class: "legend" }, h("span", {}, h("i"), "Nodo"), h("span", {}, h("i", { class: "path" }), "Camino recorrido"), h("span", {}, h("i", { class: "found" }), "Envío encontrado")),
      t.height > 4 ? h("p", { class: "muted small" }, "Los códigos se registraron en orden creciente, por eso el árbol creció hacia un solo lado: la búsqueda se parece a recorrer una lista.") : null);
  }

  const tile = (k, v, u) => h("div", { class: "tile" }, h("div", { class: "k" }, k), h("div", { class: "v" }, String(v), u ? h("span", { class: "u" }, " " + u) : null));

  async function bplusPanel() {
    const t = await api.get("/indexes/bplus");
    const scroll = h("div", { class: "tree-scroll" });
    const info = h("div", { style: "margin-top:12px" });
    const paint = (hits) => scroll.replaceChildren(drawBPlus(t.root, { hits: new Set(hits) }));
    paint([]);
    const single = codeForm("Buscar código", async (code) => {
      const r = await api.get("/indexes/bplus/entries/" + code);
      paint(r.found ? [code] : []);
      info.replaceChildren(r.found
        ? h("div", { class: "alert info" }, `${code} está en el índice; se consultaron ${r.visited} bloques (de la raíz hasta la hoja).`)
        : h("div", { class: "alert warn" }, `${code} no está en el índice.`));
    });
    const from = h("input", { class: "input", placeholder: "PKG-1002", style: "width:150px", autocomplete: "off" });
    const to = h("input", { class: "input", placeholder: "PKG-1005", style: "width:150px", autocomplete: "off" });
    const rangeBtn = h("button", { class: "btn primary", type: "submit" }, icon("search"), "Buscar rango");
    const rangeOut = h("div", { style: "margin-top:12px" });
    const rangeForm = h("form", { novalidate: true, class: "actions", style: "align-items:flex-end", onSubmit: async (e) => {
      e.preventDefault();
      const a = from.value.trim().toUpperCase(), b = to.value.trim().toUpperCase();
      let ok = true;
      if (!CODE.test(a)) { setFieldError(rangeForm, "from", "Formato PKG-0000."); ok = false; } else setFieldError(rangeForm, "from", "");
      if (!CODE.test(b)) { setFieldError(rangeForm, "to", "Formato PKG-0000."); ok = false; } else setFieldError(rangeForm, "to", "");
      if (ok && a > b) { setFieldError(rangeForm, "from", "El inicio no puede ser mayor al final."); ok = false; }
      if (!ok) return;
      try {
        const r = await busy(rangeBtn, () => api.get("/indexes/bplus/entries", { from: a, to: b }));
        paint(r.items.map((s) => s.code));
        rangeOut.replaceChildren(h("p", { class: "muted small", style: "margin-bottom:8px" }, `${r.items.length} envíos entre ${a} y ${b}. Se baja una vez hasta la primera hoja y se avanza por los enlaces entre hojas.`), r.items.length ? shipmentTable(r.items) : null);
      } catch (err) {
        rangeOut.replaceChildren(errorBox(err));
      }
    } }, field("from", "Desde", from), field("to", "Hasta", to), rangeBtn);
    return h("div", { class: "stack" },
      h("div", { class: "tiles" }, tile("Envíos indexados", t.size), tile("Niveles", t.levels), tile("Orden", t.order, "claves máx. 3 por bloque"), tile("Hojas al mismo nivel", t.leavesAtSameDepth ? "Sí" : "No")),
      h("div", { class: "grid-2" },
        h("div", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Un código")), single, info),
        h("div", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Un rango de códigos")), rangeForm, rangeOut)),
      scroll, h("div", { class: "legend" }, h("span", {}, h("i"), "Bloque"), h("span", {}, h("i", { class: "path" }), "Bloque con resultados"), h("span", {}, h("i", { class: "found" }), "Código buscado"), h("span", {}, "Las flechas punteadas unen una hoja con la siguiente")));
  }

  draw();
}
