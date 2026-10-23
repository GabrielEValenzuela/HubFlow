import { api } from "../js/api.js";
import { drawBst, drawBPlus } from "./tree.js";
import { h, subHead, loading, errorBox, field, busy, setFieldError, tabs, shipmentTable, toast, zoneTag, statusBadge, serviceBadge, fmtKg } from "../js/ui.js";
import { icon } from "../js/icons.js";

const CODE = /^(PKG-\d{4}|HF-[0-9A-Z]{9})$/;

export default async function indexes(root) {
  const slot = h("div", {}, loading());
  let active = "bst";
  const bar = tabs([["bst", "Árbol binario de búsqueda"], ["bplus", "Árbol B+ (orden 4)"]], active, (id) => { active = id; draw(); });
  root.append(subHead("Índices: BST y B+", "Índice por código de seguimiento: árbol binario de búsqueda (BST) y árbol B+ de orden 4. Búsqueda puntual con nodos visitados, recorrido in-order, altura y búsqueda por rango sobre hojas enlazadas."), bar, slot);

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
      if (!CODE.test(c)) return setFieldError(form, "code", "Usá PKG-0000 o HF- seguido de 9 caracteres.");
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
        ? h("div", { class: "alert info" }, h("div", {}, h("strong", {}, `${code} encontrado; nodos visitados: ${r.visited}.`), h("div", {}, `Camino: ${r.path.map((p) => (p.direction === "encontrado" ? p.code : `${p.code} (${p.direction})`)).join(" → ")}`)))
        : h("div", { class: "alert warn" }, `${code} no está en el índice; nodos visitados: ${r.visited}.`));
    });
    return h("div", { class: "flow" },
      h("div", { class: "tiles" }, tile("Envíos indexados", t.size), tile("Altura", t.height, "niveles"), h("div", { class: "tile", style: "grid-column: 1 / -1" }, h("div", { class: "k" }, "Recorrido in-order (claves en orden ascendente)"), h("div", { class: "chips", style: "margin-top:6px" }, ...t.inOrder.map((c) => h("span", { class: "chip" }, c))))),
      h("div", { class: "card" }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Buscar un código"), h("p", {}, "En cada nodo se compara la clave: menor, hijo izquierdo; mayor, hijo derecho."))), form, info),
      scroll, h("div", { class: "legend" }, h("span", {}, h("i"), "Nodo"), h("span", {}, h("i", { class: "path" }), "Nodos visitados"), h("span", {}, h("i", { class: "found" }), "Clave encontrada")),
      t.height > 4 ? h("p", { class: "muted small" }, "Las claves se insertaron en orden creciente y el BST no se autobalancea: degenera en una lista y la búsqueda pasa de O(log n) a O(n).") : null);
  }

  const tile = (k, v, u) => h("div", { class: "tile" }, h("div", { class: "k" }, k), h("div", { class: "v" }, String(v), u ? h("span", { class: "u" }, " " + u) : null));

  async function bplusPanel() {
    const t = await api.get("/indexes/bplus");
    const scroll = h("div", { class: "tree-scroll" });
    const info = h("div", { style: "margin-top:12px" });
    const paint = (hits) => scroll.replaceChildren(drawBPlus(t.root, { hits: new Set(hits) }));
    paint([]);
    const single = codeForm("Buscar clave", async (code) => {
      const r = await api.get("/indexes/bplus/entries/" + code);
      paint(r.found ? [code] : []);
      info.replaceChildren(r.found
        ? h("div", { class: "alert info" }, `${code} está en el índice; bloques visitados: ${r.visited} (de la raíz a la hoja).`)
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
      if (!CODE.test(a)) { setFieldError(rangeForm, "from", "Usá PKG-0000 o HF- seguido de 9 caracteres."); ok = false; } else setFieldError(rangeForm, "from", "");
      if (!CODE.test(b)) { setFieldError(rangeForm, "to", "Usá PKG-0000 o HF- seguido de 9 caracteres."); ok = false; } else setFieldError(rangeForm, "to", "");
      if (ok && a > b) { setFieldError(rangeForm, "from", "El inicio no puede ser mayor al final."); ok = false; }
      if (!ok) return;
      try {
        const r = await busy(rangeBtn, () => api.get("/indexes/bplus/entries", { from: a, to: b }));
        paint(r.items.map((s) => s.code));
        rangeOut.replaceChildren(h("p", { class: "muted small", style: "margin-bottom:8px" }, `${r.items.length} envíos entre ${a} y ${b}. Se desciende una vez hasta la hoja que contiene el inicio y se avanza por los enlaces entre hojas.`), r.items.length ? shipmentTable(r.items) : null);
      } catch (err) {
        rangeOut.replaceChildren(errorBox(err));
      }
    } }, field("from", "Desde", from), field("to", "Hasta", to), rangeBtn);
    return h("div", { class: "flow" },
      h("div", { class: "tiles" }, tile("Envíos indexados", t.size), tile("Niveles", t.levels), tile("Orden", t.order, "máx. 3 claves por nodo"), tile("Hojas a igual profundidad", t.leavesAtSameDepth ? "Sí" : "No")),
      h("div", { class: "grid-2" },
        h("div", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Un código")), single, info),
        h("div", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Búsqueda por rango")), rangeForm, rangeOut)),
      scroll, h("div", { class: "legend" }, h("span", {}, h("i"), "Bloque"), h("span", {}, h("i", { class: "path" }), "Bloque con claves del resultado"), h("span", {}, h("i", { class: "found" }), "Clave buscada"), h("span", {}, "Las flechas punteadas unen una hoja con la siguiente")));
  }

  draw();
}
