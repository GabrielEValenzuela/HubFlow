import { api } from "../api.js";
import { roadMap } from "../data.js";
import { mountPlayer } from "../lib/player.js";
import { ALGORITHM, ALGORITHM_INFO } from "../lib/algoInfo.js";
import { optionLabel } from "../lib/routeOptions.js";
import simulator from "./simulator.js";
import { h, pageHead, loading, errorBox, empty, field, busy, toast, tabs, fmtDate } from "../ui.js";
import { icon } from "../icons.js";

// "Cómo decide el PUM": única pantalla donde se usan los nombres técnicos de los algoritmos.
export default async function howItWorks(root, { id } = {}) {
  let player = null;
  let simCleanup = null;
  const body = h("div", { class: "stack", style: "margin-top:var(--s-4)" });
  const cards = h("section", { "aria-labelledby": "algo-h" },
    h("h2", { id: "algo-h", style: "margin-bottom:var(--s-3)" }, "Cuatro formas de buscar el camino"),
    h("div", { class: "algo-grid" }, ...ALGORITHM_INFO.map((a) => h("article", { class: "card" },
      h("h3", {}, a.name), h("p", { class: "small", style: "margin:6px 0" }, a.text),
      h("p", { class: "small muted" }, "En la asignación: ", h("strong", {}, a.business))))));
  root.append(pageHead("Cómo decide el PUM", "Así busca el repartidor su camino en el mapa. Acá sí aparecen los nombres técnicos: sirve para entender, comparar y descargar las trazas."), cards,
    tabs([["ruta", "Una ruta asignada"], ["comparar", "Comparar algoritmos"]], "ruta", (t) => (t === "ruta" ? showRoute() : showCompare())), body);

  function stopAll() {
    player?.destroy();
    player = null;
    if (typeof simCleanup === "function") simCleanup();
    simCleanup = null;
  }

  async function showCompare() {
    stopAll();
    body.replaceChildren(h("p", { class: "muted" }, "Elegí un tramo y mirá cómo cada algoritmo recorre el mismo mapa: cuánto explora y qué camino encuentra."));
    const holder = h("div", {});
    body.append(holder);
    simCleanup = await simulator(holder);
  }

  async function showRoute() {
    stopAll();
    body.replaceChildren(loading());
    try {
      const list = await api.get("/routes", { limit: 100 });
      if (!list.items.length) return body.replaceChildren(empty("Todavía no hay rutas asignadas", "Asigná una ruta desde la Mesa de clasificación para ver paso a paso cómo se calculó.", h("a", { class: "btn primary", href: "#/mesa" }, "Ir a la mesa")));
      const wanted = id && list.items.some((r) => String(r.id) === String(id)) ? String(id) : String(list.items[0].id);
      const select = h("select", { class: "input", onChange: (e) => loadRoute(e.target.value) },
        ...list.items.map((r) => h("option", { value: r.id, selected: String(r.id) === wanted }, `Ruta #${r.id} · ${r.courier.name} · ${optionLabel(r.algorithm)} · ${fmtDate(r.createdAt)}`)));
      const out = h("div", { class: "stack" });
      body.replaceChildren(h("div", { class: "card" }, field("route", "Ruta a reproducir", select)), out);
      await loadRoute(wanted);

      async function loadRoute(rid) {
        player?.destroy();
        out.replaceChildren(loading());
        try {
          const [route, map] = await Promise.all([api.get("/routes/" + rid), roadMap()]);
          player = mountPlayer({ map, sequence: route.sequence, stops: route.stops });
          const download = (path, name) => h("button", { class: "btn small", onClick: (e) => busy(e.currentTarget, () => api.download(path, name)).catch((err) => toast(err.message, "error")) }, icon("download", 16), name);
          out.replaceChildren(
            h("div", { class: "card-head" }, h("div", {}, h("h2", {}, `Ruta #${route.id}: ${ALGORITHM[route.algorithm] ?? route.algorithm}`),
              h("p", {}, "Reproducí cómo el agente fue descartando calles hasta dar con el camino.")),
              h("div", { class: "actions" }, download(`/routes/${rid}/initial-state`, "initial_state.json"), download(`/routes/${rid}/sequence`, "sequence.json"))),
            player.el);
        } catch (e) {
          out.replaceChildren(errorBox(e, () => loadRoute(rid)));
        }
      }
    } catch (e) {
      body.replaceChildren(errorBox(e, showRoute));
    }
  }

  showRoute();
  return stopAll;
}
