import { api } from "../api.js";
import { navigate } from "../router.js";
import { h, svg, pageHead, loading, empty, errorBox, field, busy, toast, statusBadge, zoneTag, serviceBadge, fmtKg, ZONE, ALGORITHM, ALGORITHMS, setFieldError } from "../ui.js";
import { icon } from "../icons.js";

const ORDER_LABEL = { NONE: "Sin ordenar", CODE_ASC: "Por código (A→Z)", WEIGHT_DESC: "Por peso (mayor primero)" };

export default async function sorting(root) {
  let batch = null;
  let stats = null;
  let search = null;
  const slot = h("div", {}, loading());
  root.append(pageHead("Mesa de clasificación", "Armá un lote por zona, ordenalo, prepará la cola de salida y entregáselo a un repartidor."), slot);

  async function load() {
    try {
      batch = await api.get("/sorting-batches/current");
    } catch (e) {
      if (e.status === 404) batch = null;
      else return slot.replaceChildren(errorBox(e, load));
    }
    draw();
  }

  function draw() {
    slot.replaceChildren(batch ? board() : opener());
  }

  function opener() {
    const zone = h("select", { class: "input" }, h("option", { value: "" }, "Elegí una zona"), ...Object.entries(ZONE).map(([v, l]) => h("option", { value: v }, l)));
    const btn = h("button", { class: "btn primary", type: "submit" }, "Abrir lote");
    const form = h("form", { novalidate: true, class: "card", style: "max-width:520px" },
      h("h2", {}, "No hay un lote abierto"),
      h("p", { class: "muted", style: "margin:6px 0 16px" }, "Un lote reúne los pendientes de una zona para ordenarlos y despacharlos juntos."),
      field("zone", "Zona del lote", zone), h("div", { class: "form-error" }), h("div", { class: "form-actions" }, btn));
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      if (!zone.value) return setFieldError(form, "zone", "Elegí una zona para abrir el lote.");
      try {
        batch = await busy(btn, () => api.post("/sorting-batches", { zone: zone.value }));
        stats = null; search = null;
        toast(`Lote #${batch.id} abierto en ${ZONE[zone.value]}.`);
        draw();
      } catch (err) {
        setFieldError(form, "zone", err.fields?.[0]?.message ?? err.message);
      }
    });
    return form;
  }

  async function act(btn, fn, okMsg) {
    try {
      await busy(btn, fn);
      if (okMsg) toast(okMsg);
      draw();
    } catch (e) {
      toast(e.message, "error");
    }
  }

  function board() {
    const sortedByCode = batch.sortedBy === "CODE_ASC";
    const id = batch.id;
    const sortBtns = [["CODE_ASC", "Por código"], ["WEIGHT_DESC", "Por peso"]].map(([v, l]) =>
      h("button", { "aria-pressed": String(batch.sortedBy === v), onClick: (e) => act(e.currentTarget, async () => {
        const r = await api.patch("/sorting-batches/" + id, { sortBy: v });
        batch = r.batch; stats = r.stats; search = null;
      }) }, l));

    const processed = new Set(batch.processedStack);
    const itemsTable = h("div", { class: "table-wrap" }, h("table", {},
      h("thead", {}, h("tr", {}, ...["Posición", "Código", "Destinatario", "Servicio", "Peso", "Estado"].map((t) => h("th", { class: t === "Peso" ? "num" : "" }, t)))),
      h("tbody", {}, ...batch.items.map((s, i) => h("tr", {},
        h("td", { class: "code" }, String(i + 1)), h("td", { class: "code" }, s.code), h("td", {}, s.recipient), h("td", {}, serviceBadge(s.service)),
        h("td", { class: "num" }, fmtKg(s.weightKg)),
        h("td", {}, processed.has(s.code) ? h("span", { class: "badge s-CLASIFICADO" }, "Procesado") : statusBadge(s.status)))))));

    const prepare = h("button", { class: "btn", onClick: (e) => act(e.currentTarget, async () => { batch = await api.put(`/sorting-batches/${id}/preparation-queue`); }, "Cola de preparación lista.") }, "Preparar cola");
    const next = h("button", { class: "btn primary", disabled: !batch.preparationQueue.length, onClick: (e) => act(e.currentTarget, async () => {
      const r = await api.post(`/sorting-batches/${id}/processed-items`, {});
      batch = r.batch;
      toast(`Procesado ${r.shipment.code}.`);
    }) }, icon("check"), "Procesar siguiente");
    const top = batch.processedStack[0];
    const back = h("button", { class: "btn", disabled: !top, onClick: (e) => act(e.currentTarget, async () => {
      const r = await api.del(`/sorting-batches/${id}/processed-items/${top}`);
      batch = r.batch;
      toast(`${top} salió de los procesados y quedó sin procesar.`);
    }) }, icon("undo"), "Devolver el último");

    return h("div", { class: "stack" },
      h("div", { class: "tiles" },
        tile("Lote", "#" + id), h("div", { class: "tile" }, h("div", { class: "k" }, "Zona"), h("div", { style: "font-size:1.25rem;font-weight:700" }, zoneTag(batch.zone))),
        tile("Envíos", batch.size), h("div", { class: "tile" }, h("div", { class: "k" }, "Orden actual"), h("div", { style: "font-size:1.05rem;font-weight:650;margin-top:6px" }, ORDER_LABEL[batch.sortedBy] ?? batch.sortedBy))),

      h("section", { class: "card" },
        h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "1. Ordenar el lote"), h("p", {}, "Se ordena por inserción; abajo ves cuánto trabajo costó.")), h("div", { class: "seg" }, ...sortBtns)),
        stats ? h("div", { class: "tiles", style: "margin-bottom:12px" }, tile("Comparaciones", stats.comparisons), tile("Desplazamientos", stats.shifts), h("div", { class: "tile" }, h("div", { class: "k" }, "Método"), h("div", { style: "font-weight:650;margin-top:6px" }, "Inserción"))) : null,
        itemsTable),

      h("section", { class: "card" },
        h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "2. Preparar y procesar"), h("p", {}, "La cola sale en el orden del lote; lo procesado se apila y el último en entrar es el primero en poder volver.")), h("div", { class: "actions" }, prepare, next, back)),
        h("div", { class: "lanes" }, queueLane(batch.preparationQueue), stackLane(batch.processedStack))),

      searchCard(sortedByCode),
      assignCard());
  }

  const tile = (k, v) => h("div", { class: "tile" }, h("div", { class: "k" }, k), h("div", { class: "v" }, String(v)));

  function queueLane(q) {
    return h("div", { class: "lane" }, h("h3", {}, "Cola de preparación ", h("span", { class: "tag" }, "FIFO · el primero en entrar sale primero")),
      q.length ? h("ol", { class: "queue", "aria-label": "Cola de preparación" }, ...q.map((c, i) => h("li", { class: i === 0 ? "front" : "" }, h("span", { class: "code" }, c), i === 0 ? h("small", {}, "sale ahora") : null)))
        : h("p", { class: "muted lane-empty" }, "La cola está vacía. Tocá “Preparar cola” para cargar el lote."));
  }

  function stackLane(s) {
    const rev = s;
    return h("div", { class: "lane" }, h("h3", {}, "Procesados ", h("span", { class: "tag" }, "LIFO · el último en entrar sale primero")),
      rev.length ? h("ol", { class: "pile", "aria-label": "Pila de procesados, tope arriba" }, ...rev.map((c, i) => h("li", { class: i === 0 ? "top" : "" }, h("span", { class: "code" }, c), i === 0 ? h("small", {}, "tope") : null)))
        : h("p", { class: "muted lane-empty" }, "Todavía no se procesó ningún envío."));
  }

  function searchCard(sortedByCode) {
    const input = h("input", { class: "input", placeholder: "PKG-1005", style: "max-width:200px", autocomplete: "off" });
    const btn = h("button", { class: "btn primary", type: "submit" }, icon("search"), "Buscar");
    const out = h("div", { style: "margin-top:16px" });
    const form = h("form", { novalidate: true, class: "actions", onSubmit: async (e) => {
      e.preventDefault();
      const code = input.value.trim().toUpperCase();
      if (!/^PKG-\d{4}$/.test(code)) return setFieldError(wrap, "code", "Usá el formato PKG-0000.");
      setFieldError(wrap, "code", "");
      try {
        search = await busy(btn, () => api.get(`/sorting-batches/${batch.id}/searches`, { code, algorithm: sortedByCode ? "both" : "linear" }));
        out.replaceChildren(searchResult(search, sortedByCode));
      } catch (err) {
        out.replaceChildren(errorBox(err));
      }
    } }, field("code", "Código a buscar", input), btn);
    const wrap = form;
    form.style.alignItems = "flex-end";
    if (search && search.code) { input.value = search.code; out.replaceChildren(searchResult(search, sortedByCode)); }
    return h("section", { class: "card" },
      h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "3. Buscar un envío en el lote"), h("p", {}, "Compará cuántos envíos hay que mirar recorriendo uno por uno contra partir el lote por la mitad."))),
      form, out);
  }

  function searchResult(r, sortedByCode) {
    const max = Math.max(r.linear.comparisons, r.binary?.comparisons ?? 0, 1);
    const bar = (label, res, kind) => h("div", { class: "cmp-row" }, h("span", { class: "cmp-label" }, label),
      h("div", { class: "cmp-bar", role: "img", "aria-label": `${res.comparisons} comparaciones` }, h("span", { class: kind, style: `width:${Math.max(4, (res.comparisons / max) * 100)}%` })),
      h("strong", { class: "code" }, `${res.comparisons}`));
    const found = r.linear.found;
    const parts = [h("div", { class: "alert " + (found ? "info" : "warn") }, found ? `${r.code} está en la posición ${r.linear.position + 1} del lote.` : `${r.code} no está en este lote.`),
      h("div", { class: "cmp" }, bar("Recorrido uno por uno", r.linear, "linear"), r.binary ? bar("Búsqueda por mitades", r.binary, "binary") : null)];
    if (r.binary?.steps?.length) {
      parts.push(h("h3", { style: "margin:16px 0 6px" }, "Cómo se acotó la búsqueda por mitades"),
        h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, h("tr", {}, ...["Paso", "izq", "mid", "der", "Código en el medio", "Decisión"].map((t) => h("th", {}, t)))),
          h("tbody", {}, ...r.binary.steps.map((s, i) => h("tr", {}, h("td", {}, String(i + 1)), h("td", { class: "code" }, String(s.left)), h("td", { class: "code" }, String(s.middle)), h("td", { class: "code" }, String(s.right)),
            h("td", { class: "code" }, s.codeAtMiddle),
            h("td", {}, s.codeAtMiddle === r.code ? "Encontrado" : r.code < s.codeAtMiddle ? "Seguir por la izquierda" : "Seguir por la derecha")))))));
    }
    if (!sortedByCode) {
      parts.push(h("div", { class: "alert warn", style: "margin-top:12px" }, h("div", {}, h("strong", {}, "Búsqueda por mitades no disponible. "), r.binaryUnavailableReason || "El lote tiene que estar ordenado por código (A→Z) para poder partirlo por la mitad.")));
    }
    return h("div", {}, ...parts);
  }

  function assignCard() {
    const courier = h("select", { class: "input" }, h("option", { value: "" }, "Cargando repartidores…"));
    const algo = h("select", { class: "input" }, ...ALGORITHMS.map((a) => h("option", { value: a, selected: a === "ASTAR" }, ALGORITHM[a])));
    const btn = h("button", { class: "btn primary", type: "submit", disabled: !batch.processedStack.length }, icon("route"), "Asignar ruta");
    api.get("/couriers").then((r) => {
      courier.replaceChildren(h("option", { value: "" }, "Elegí un repartidor"), ...r.items.filter((u) => u.active).map((u) => h("option", { value: u.id }, u.name)));
    }).catch(() => courier.replaceChildren(h("option", { value: "" }, "No se pudo cargar")));
    const form = h("form", { novalidate: true, onSubmit: async (e) => {
      e.preventDefault();
      if (!courier.value) return setFieldError(form, "courierId", "Elegí quién va a repartir.");
      try {
        const r = await busy(btn, () => api.post("/routes", { batchId: batch.id, courierId: Number(courier.value), algorithm: algo.value }));
        toast(`Ruta #${r.id} asignada a ${r.courier.name}.`);
        navigate("/rutas/" + r.id);
      } catch (err) {
        toast(err.message, "error");
      }
    } }, h("div", { class: "form-grid" }, field("courierId", "Repartidor (PUM)", courier), field("algorithm", "Cómo busca el camino", algo, { hint: "A* encuentra el más corto en km; los otros sirven para comparar." })),
      h("div", { class: "form-error" }), h("div", { class: "form-actions" }, btn,
        batch.processedStack.length ? h("span", { class: "muted small" }, `Se reparten ${batch.processedStack.length} envíos procesados, en el orden de la pila.`) : h("span", { class: "muted small" }, "Procesá al menos un envío para asignar la ruta.")));
    return h("section", { class: "card" }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "4. Asignar la ruta"), h("p", {}, "El repartidor recibe las paradas y el recorrido calculado sobre el mapa de la ciudad."))), form);
  }

  load();
}
