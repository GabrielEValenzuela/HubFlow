import { api } from "../api.js";
import { navigate } from "../router.js";
import { h, pageHead, loading, errorBox, field, busy, toast, statusBadge, zoneTag, serviceBadge, fmtKg, ZONE, fmtKm, setFieldError } from "../ui.js";
import { ROUTE_OPTIONS, DEFAULT_OPTION, TRAVEL_ESTIMATE, compareOptions, fmtDuration } from "../lib/routeOptions.js";
import { icon } from "../icons.js";

const ORDER_LABEL = { NONE: "Sin ordenar", CODE_ASC: "Por código", WEIGHT_DESC: "Más pesados primero" };

export default async function loadPlan(root) {
  let load = null;
  let hit = null;
  const slot = h("div", {}, loading());
  root.append(pageHead("Armar carga", "Elegí una zona, ordená los paquetes, cargalos en el vehículo y asigná el chofer que sale a entregar."), slot);

  async function fetchLoad() {
    try {
      load = await api.get("/sorting-batches/current");
    } catch (e) {
      if (e.status === 404) load = null;
      else return slot.replaceChildren(errorBox(e, fetchLoad));
    }
    draw();
  }

  const draw = () => slot.replaceChildren(load ? board() : opener());

  function opener() {
    const zone = h("select", { class: "input" }, h("option", { value: "" }, "Elegí una zona"), ...Object.entries(ZONE).map(([v, l]) => h("option", { value: v }, l)));
    const btn = h("button", { class: "btn primary", type: "submit" }, "Armar carga de esta zona");
    const form = h("form", { novalidate: true, class: "card", style: "max-width:520px" },
      h("h2", {}, "No hay una carga en preparación"),
      h("p", { class: "muted", style: "margin:6px 0 16px" }, "Una carga reúne los paquetes pendientes de una zona para ordenarlos, subirlos al vehículo y asignarlos a un chofer."),
      field("zone", "Zona", zone), h("div", { class: "form-error" }), h("div", { class: "form-actions" }, btn));
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      if (!zone.value) return setFieldError(form, "zone", "Elegí una zona para armar la carga.");
      try {
        load = await busy(btn, () => api.post("/sorting-batches", { zone: zone.value }));
        hit = null;
        toast(`Carga #${load.id} armada en ${ZONE[zone.value]}.`);
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

  const tile = (k, v) => h("div", { class: "tile" }, h("div", { class: "k" }, k), h("div", { class: "v" }, String(v)));

  function board() {
    const id = load.id;
    const toLoad = load.preparationQueue;
    const onVehicle = load.processedStack;
    const placed = new Set(onVehicle);
    const waiting = new Set(toLoad);

    const sortBtns = [["CODE_ASC", "Por código"], ["WEIGHT_DESC", "Más pesados primero"]].map(([v, l]) =>
      h("button", { "aria-pressed": String(load.sortedBy === v), onClick: (e) => act(e.currentTarget, async () => {
        const r = await api.patch("/sorting-batches/" + id, { sortBy: v });
        load = r.batch; hit = null;
      }, `Carga ordenada: ${l.toLowerCase()}.`) }, l));

    const prepare = h("button", { class: "btn", disabled: onVehicle.length > 0, title: onVehicle.length ? "Ya empezaste a cargar. Para volver a preparar, bajá primero todos los paquetes del vehículo." : null, onClick: (e) => act(e.currentTarget, async () => { load = await api.put(`/sorting-batches/${id}/preparation-queue`); }, "Lista “Por cargar” preparada.") }, "Preparar para cargar");
    const next = h("button", { class: "btn primary", disabled: !toLoad.length, onClick: (e) => act(e.currentTarget, async () => {
      const r = await api.post(`/sorting-batches/${id}/processed-items`, {});
      load = r.batch;
      toast(`${r.shipment.code} subió al vehículo.`);
    }) }, icon("check"), "Cargar siguiente");
    const last = onVehicle[0];
    const back = h("button", { class: "btn", disabled: !last, onClick: (e) => act(e.currentTarget, async () => {
      const r = await api.del(`/sorting-batches/${id}/processed-items/${last}`);
      load = r.batch;
      toast(`${last} bajó del vehículo y quedó sin cargar.`);
    }) }, icon("undo"), "Bajar del vehículo");

    const tableHolder = h("div", {});
    const paintTable = () => tableHolder.replaceChildren(h("div", { class: "table-wrap" }, h("table", {},
      h("thead", {}, h("tr", {}, ...["N.º", "Código", "Destinatario", "Servicio", "Peso", "Estado"].map((t) => h("th", { class: t === "Peso" ? "num" : "" }, t)))),
      h("tbody", {}, ...load.items.map((s, i) => h("tr", { class: hit === s.code ? "is-hit" : "", "aria-current": hit === s.code ? "true" : null },
        h("td", { class: "code" }, String(i + 1)), h("td", { class: "code" }, s.code), h("td", {}, s.recipient), h("td", {}, serviceBadge(s.service)),
        h("td", { class: "num" }, fmtKg(s.weightKg)),
        h("td", {}, placed.has(s.code) ? h("span", { class: "badge s-CLASIFICADO" }, "En el vehículo") : waiting.has(s.code) ? h("span", { class: "badge s-PENDIENTE" }, "Por cargar") : statusBadge(s.status))))))));
    paintTable();

    return h("div", { class: "flow" },
      h("div", { class: "tiles" },
        tile("Carga", "#" + id), h("div", { class: "tile" }, h("div", { class: "k" }, "Zona"), h("div", { style: "font-size:1.25rem;font-weight:700" }, zoneTag(load.zone))),
        tile("Paquetes", load.size), h("div", { class: "tile" }, h("div", { class: "k" }, "Orden actual"), h("div", { style: "font-size:1.05rem;font-weight:650;margin-top:6px" }, ORDER_LABEL[load.sortedBy] ?? load.sortedBy))),

      h("section", { class: "card" },
        h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "1. Ordenar los paquetes"), h("p", {}, "Elegí con qué orden querés cargarlos.")), h("div", { class: "seg", role: "group", "aria-label": "Orden de la carga" }, ...sortBtns)),
        tableHolder,
        search(paintTable)),

      h("section", { class: "card" },
        h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "2. Preparar y cargar"),
          h("p", {}, "Preparar para cargar arma la lista “Por cargar” en el orden elegido. Cargar siguiente sube el primero al vehículo. Lo último que subió es lo primero que se entrega. Si querés volver a preparar la lista, primero bajá del vehículo todo lo que cargaste.")),
          h("div", { class: "actions" }, prepare, next, back)),
        h("div", { class: "lanes" }, toLoadLane(toLoad), vehicleLane(onVehicle))),

      assign());
  }

  function toLoadLane(codes) {
    return h("div", { class: "lane" }, h("h3", {}, "Por cargar"),
      codes.length ? h("ol", { class: "todo-list", "aria-label": "Por cargar, en orden" }, ...codes.map((c, i) => h("li", { class: i === 0 ? "front" : "" }, h("span", { class: "code" }, c), i === 0 ? h("small", {}, "sigue") : null)))
        : h("p", { class: "muted lane-empty" }, "No hay paquetes esperando. Tocá “Preparar para cargar” para armar la lista."));
  }

  function vehicleLane(codes) {
    return h("div", { class: "lane" }, h("h3", {}, "En el vehículo"),
      codes.length ? h("ol", { class: "on-board", "aria-label": "En el vehículo, el último que subió primero" }, ...codes.map((c, i) => h("li", { class: i === 0 ? "top" : "" }, h("span", { class: "code" }, c), i === 0 ? h("small", {}, "último en subir") : null)))
        : h("p", { class: "muted lane-empty" }, "Todavía no subió ningún paquete."));
  }

  function search(paintTable) {
    const input = h("input", { class: "input", placeholder: "PKG-1005", style: "max-width:200px", autocomplete: "off" });
    const btn = h("button", { class: "btn", type: "submit" }, icon("search"), "Buscar en esta carga");
    const out = h("div", { style: "margin-top:12px", "aria-live": "polite" });
    const form = h("form", { novalidate: true, class: "actions", style: "align-items:flex-end;margin-top:16px", onSubmit: async (e) => {
      e.preventDefault();
      const code = input.value.trim().toUpperCase();
      if (!/^PKG-\d{4}$/.test(code)) return setFieldError(form, "code", "Usá el formato PKG-0000.");
      setFieldError(form, "code", "");
      try {
        const r = await busy(btn, () => api.get(`/sorting-batches/${load.id}/searches`, { code, algorithm: "linear" }));
        hit = r.linear.found ? code : null;
        paintTable();
        if (!r.linear.found) return out.replaceChildren(h("div", { class: "alert warn" }, h("div", {}, `${code} no está en esta carga.`)));
        const where = load.processedStack.includes(code) ? "ya está en el vehículo" : load.preparationQueue.includes(code) ? "está en la lista “Por cargar”" : "todavía no se preparó para cargar";
        out.replaceChildren(h("div", { class: "alert info" }, h("div", {}, `${code} está en esta carga y ${where}. Quedó resaltado en la tabla.`)));
        document.querySelector("tr.is-hit")?.scrollIntoView({ block: "nearest" });
      } catch (err) {
        out.replaceChildren(errorBox(err));
      }
    } }, field("code", "Código a buscar", input), btn);
    if (hit) input.value = hit;
    return h("div", {}, form, out);
  }

  function assign() {
    const onVehicle = load.processedStack;
    const courier = h("select", { class: "input" }, h("option", { value: "" }, "Cargando choferes…"));
    const btn = h("button", { class: "btn primary", type: "submit", disabled: !onVehicle.length }, icon("route"), "Asignar chofer");
    api.get("/couriers").then((r) => {
      courier.replaceChildren(h("option", { value: "" }, "Elegí un chofer"), ...r.items.filter((u) => u.active).map((u) => h("option", { value: u.id }, u.name)));
    }).catch(() => courier.replaceChildren(h("option", { value: "" }, "No se pudo cargar")));

    // Orden de entrega: el último que subió al vehículo es el primero en bajar (igual que el servidor).
    const pointOf = new Map(load.items.map((s) => [s.code, s.deliveryPoint]));
    const stopNodes = onVehicle.map((c) => pointOf.get(c)).filter(Boolean);
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
    const note = h("p", { class: "muted small", style: "margin-top:8px" }, TRAVEL_ESTIMATE.note);
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
        const r = await busy(btn, () => api.post("/routes", { batchId: load.id, courierId: Number(courier.value), algorithm: chosen }));
        toast(`Reparto #${r.id} asignado a ${r.courier.name}.`);
        navigate("/repartos/" + r.id);
      } catch (err) {
        toast(err.message, "error");
      }
    } }, h("div", { class: "form-grid" }, field("courierId", "Chofer", courier)),
      h("fieldset", { class: "crit", style: "margin-top:12px" }, h("legend", { id: "opt-legend" }, "¿Qué priorizar en esta ruta?"), optionsBox, note, failure),
      h("div", { class: "form-error" }), h("div", { class: "form-actions" }, btn,
        onVehicle.length ? h("span", { class: "muted small" }, `Se reparten ${onVehicle.length} paquetes cargados; el último que subió se entrega primero.`) : h("span", { class: "muted small" }, "Cargá al menos un paquete en el vehículo para asignar el chofer.")));
    return h("section", { class: "card" }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "3. Asignar chofer"), h("p", {}, "Cada opción se calcula con los paquetes que están en el vehículo; elegí cuál recibe el chofer."))), form);
  }

  fetchLoad();
}
