import { api } from "../api.js";
import { navigate } from "../router.js";
import { h, svg, pageHead, loading, empty, errorBox, field, busy, toast, statusBadge, zoneTag, serviceBadge, fmtKg, ZONE, fmtKm, setFieldError } from "../ui.js";
import { ROUTE_OPTIONS, DEFAULT_OPTION, ETA, compareOptions, fmtDuration } from "../lib/routeOptions.js";
import { icon } from "../icons.js";

const ORDER_LABEL = { NONE: "Sin ordenar", CODE_ASC: "Por código (A→Z)", WEIGHT_DESC: "Por peso (mayor primero)" };

export default async function sorting(root) {
  let batch = null;
  let stats = null;
  let search = null;
  const slot = h("div", {}, loading());
  root.append(pageHead("Mesa de clasificación", "Lote por zona con estructuras propias: ordenamiento por inserción, Queue FIFO de preparación, Stack LIFO de procesados, búsqueda lineal frente a binaria y asignación de ruta."), slot);

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
      h("p", { class: "muted", style: "margin:6px 0 16px" }, "Un lote toma los envíos pendientes de una zona para ordenarlos, procesarlos y asignarlos a un repartidor."),
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
        h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "1. Ordenar el lote"), h("p", {}, "Insertion Sort estable: O(n) en el mejor caso, O(n²) en el peor. Se informan comparaciones y desplazamientos.")), h("div", { class: "seg" }, ...sortBtns)),
        stats ? h("div", { class: "tiles", style: "margin-bottom:12px" }, tile("Comparaciones", stats.comparisons), tile("Desplazamientos", stats.shifts), h("div", { class: "tile" }, h("div", { class: "k" }, "Método"), h("div", { style: "font-weight:650;margin-top:6px" }, "Insertion Sort"))) : null,
        itemsTable),

      h("section", { class: "card" },
        h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "2. Preparar y procesar"), h("p", {}, "Preparar cola carga el lote en una Queue FIFO. Procesar siguiente la desencola y apila el envío en una Stack LIFO; Devolver el último desapila el tope.")), h("div", { class: "actions" }, prepare, next, back)),
        h("div", { class: "lanes" }, queueLane(batch.preparationQueue), stackLane(batch.processedStack))),

      searchCard(sortedByCode),
      assignCard());
  }

  const tile = (k, v) => h("div", { class: "tile" }, h("div", { class: "k" }, k), h("div", { class: "v" }, String(v)));

  function queueLane(q) {
    return h("div", { class: "lane" }, h("h3", {}, "Cola de preparación ", h("span", { class: "tag" }, "Queue FIFO: sale el primero que entró")),
      q.length ? h("ol", { class: "queue", "aria-label": "Cola de preparación" }, ...q.map((c, i) => h("li", { class: i === 0 ? "front" : "" }, h("span", { class: "code" }, c), i === 0 ? h("small", {}, "sale ahora") : null)))
        : h("p", { class: "muted lane-empty" }, "La cola está vacía. Tocá “Preparar cola” para cargar el lote."));
  }

  function stackLane(s) {
    const rev = s;
    return h("div", { class: "lane" }, h("h3", {}, "Procesados ", h("span", { class: "tag" }, "Stack LIFO: sale el último que entró")),
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
      h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "3. Buscar un envío en el lote"), h("p", {}, "Búsqueda lineal O(n) frente a binaria O(log n), con el número de comparaciones de cada una. La binaria exige el lote ordenado por código."))),
      form, out);
  }

  function searchResult(r, sortedByCode) {
    const max = Math.max(r.linear.comparisons, r.binary?.comparisons ?? 0, 1);
    const bar = (label, res, kind) => h("div", { class: "cmp-row" }, h("span", { class: "cmp-label" }, label),
      h("div", { class: "cmp-bar", role: "img", "aria-label": `${res.comparisons} comparaciones` }, h("span", { class: kind, style: `width:${Math.max(4, (res.comparisons / max) * 100)}%` })),
      h("strong", { class: "code" }, `${res.comparisons}`));
    const found = r.linear.found;
    const parts = [h("div", { class: "alert " + (found ? "info" : "warn") }, found ? `${r.code} está en la posición ${r.linear.position + 1} del lote.` : `${r.code} no está en este lote.`),
      h("div", { class: "cmp" }, bar("Búsqueda lineal", r.linear, "linear"), r.binary ? bar("Búsqueda binaria", r.binary, "binary") : null)];
    if (r.binary?.steps?.length) {
      parts.push(h("h3", { style: "margin:16px 0 6px" }, "Iteraciones de la búsqueda binaria"),
        h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, h("tr", {}, ...["Paso", "izq", "mid", "der", "Código en mid", "Decisión"].map((t) => h("th", {}, t)))),
          h("tbody", {}, ...r.binary.steps.map((s, i) => h("tr", {}, h("td", {}, String(i + 1)), h("td", { class: "code" }, String(s.left)), h("td", { class: "code" }, String(s.middle)), h("td", { class: "code" }, String(s.right)),
            h("td", { class: "code" }, s.codeAtMiddle),
            h("td", {}, s.codeAtMiddle === r.code ? "Encontrado" : r.code < s.codeAtMiddle ? "Descartar la mitad derecha" : "Descartar la mitad izquierda")))))));
    }
    if (!sortedByCode) {
      parts.push(h("div", { class: "alert warn", style: "margin-top:12px" }, h("div", {}, h("strong", {}, "Búsqueda binaria no disponible. "), r.binaryUnavailableReason || "Precondición: lote ordenado por código ascendente.")));
    }
    return h("div", {}, ...parts);
  }

  function assignCard() {
    const courier = h("select", { class: "input" }, h("option", { value: "" }, "Cargando repartidores…"));
    const btn = h("button", { class: "btn primary", type: "submit", disabled: !batch.processedStack.length }, icon("route"), "Asignar ruta");
    api.get("/couriers").then((r) => {
      courier.replaceChildren(h("option", { value: "" }, "Elegí un repartidor"), ...r.items.filter((u) => u.active).map((u) => h("option", { value: u.id }, u.name)));
    }).catch(() => courier.replaceChildren(h("option", { value: "" }, "No se pudo cargar")));

    // Orden de descarga = orden de la pila (el último procesado baja primero), igual que el servidor.
    const pointOf = new Map(batch.items.map((s) => [s.code, s.deliveryPoint]));
    const stopNodes = batch.processedStack.map((c) => pointOf.get(c)).filter(Boolean);
    let chosen = DEFAULT_OPTION;
    const optionsBox = h("div", { class: "route-options", role: "radiogroup", "aria-labelledby": "opt-legend" });
    const drawOptions = (stats, failed) => {
      optionsBox.replaceChildren(...ROUTE_OPTIONS.map((o) => {
        const st = stats?.find((x) => x.algorithm === o.algorithm);
        const input = h("input", { type: "radio", name: "routeOption", value: o.algorithm, checked: chosen === o.algorithm, onChange: () => { chosen = o.algorithm; mark(); } });
        const body = h("span", { class: "opt-body" },
          h("span", { class: "opt-title" }, h("strong", {}, o.label), o.recommended ? h("span", { class: "badge s-ENTREGADO" }, "Recomendada") : null),
          h("span", { class: "muted small" }, o.short),
          st ? h("dl", { class: "opt-stats" },
            h("div", {}, h("dt", {}, "Kilómetros"), h("dd", {}, fmtKm(st.km))),
            h("div", {}, h("dt", {}, "Paradas"), h("dd", {}, String(st.stops))),
            h("div", {}, h("dt", {}, "Tiempo estimado"), h("dd", {}, "≈ " + fmtDuration(st.minutes))),
            h("div", {}, h("dt", {}, "Cruces"), h("dd", {}, String(st.crossings)))) : null,
          st ? h("span", { class: "opt-reason" }, st.reason) : stats === undefined && !failed ? h("span", { class: "muted small" }, "Calculando…") : null);
        return h("label", { class: "route-option" + (chosen === o.algorithm ? " on" : "") }, input, body);
      }));
    };
    const mark = () => optionsBox.querySelectorAll(".route-option").forEach((l) => l.classList.toggle("on", l.querySelector("input").checked));
    drawOptions(undefined, false);
    const note = h("p", { class: "muted small", style: "margin-top:8px" }, ETA.note);
    const failure = h("div", {});
    const compute = () => {
      failure.replaceChildren();
      if (!stopNodes.length) return drawOptions(null, true);
      drawOptions(undefined, false);
      compareOptions(stopNodes).then((stats) => drawOptions(stats, false))
        .catch((e) => { drawOptions(null, true); failure.replaceChildren(errorBox(e, compute)); });
    };
    compute();

    const form = h("form", { novalidate: true, onSubmit: async (e) => {
      e.preventDefault();
      if (!courier.value) return setFieldError(form, "courierId", "Elegí quién va a repartir.");
      try {
        const r = await busy(btn, () => api.post("/routes", { batchId: batch.id, courierId: Number(courier.value), algorithm: chosen }));
        toast(`Ruta #${r.id} asignada a ${r.courier.name}.`);
        navigate("/rutas/" + r.id);
      } catch (err) {
        toast(err.message, "error");
      }
    } }, h("div", { class: "form-grid" }, field("courierId", "Repartidor (PUM)", courier)),
      h("fieldset", { class: "crit", style: "margin-top:12px" }, h("legend", { id: "opt-legend" }, "¿Qué priorizar en esta ruta?"), optionsBox, note, failure),
      h("div", { class: "form-error" }), h("div", { class: "form-actions" }, btn,
        batch.processedStack.length ? h("span", { class: "muted small" }, `Se reparten ${batch.processedStack.length} envíos procesados, en el orden de la pila.`) : h("span", { class: "muted small" }, "Procesá al menos un envío para asignar la ruta.")));
    return h("section", { class: "card" }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "4. Asignar la ruta"), h("p", {}, "Cada opción se calcula sobre la carga procesada; elegí cuál recibe el repartidor."))), form);
  }

  load();
}
