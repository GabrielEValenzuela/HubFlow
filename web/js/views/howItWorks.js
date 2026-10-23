import { api } from "../api.js";
import { roadMap } from "../data.js";
import { mountPlayer } from "../lib/player.js";
import { ALGORITHM, ALGORITHM_INFO, SEARCH_FORMULATION, ALGO_NOTES } from "../lib/algoInfo.js";
import { optionLabel } from "../lib/routeOptions.js";
import simulator from "./simulator.js";
import { h, pageHead, loading, errorBox, empty, field, busy, toast, tabs, fmtDate } from "../ui.js";
import { icon } from "../icons.js";

export default async function howItWorks(root, { id } = {}) {
  let player = null;
  let simCleanup = null;
  const body = h("div", { class: "stack", style: "margin-top:var(--s-4)" });
  const cell = (label, v) => h("td", { "data-label": label }, v);
  const rowOf = (label, f) => h("tr", {}, h("th", { scope: "row" }, label), ...ALGORITHM_INFO.map((a) => cell(a.name, f(a))));
  const cards = h("section", { "aria-labelledby": "algo-h" },
    h("h2", { id: "algo-h", style: "margin-bottom:var(--s-3)" }, "Búsqueda en grafo: BFS, DFS, Greedy y A*"),
    h("div", { class: "card" }, h("h3", {}, "Formulación común"),
      h("dl", { class: "algo-def" }, ...SEARCH_FORMULATION.flatMap(([k, v]) => [h("dt", {}, k), h("dd", {}, v)]))),
    h("div", { class: "table-wrap algo-table-wrap", style: "margin-top:var(--s-4)" }, h("table", { class: "algo-table" },
      h("thead", {}, h("tr", {}, h("th", {}, "Propiedad"), ...ALGORITHM_INFO.map((a) => h("th", {}, a.name)))),
      h("tbody", {},
        rowOf("Frontera", (a) => h("span", {}, a.frontier, h("br"), h("span", { class: "code small" }, a.file))),
        rowOf("Desempate", (a) => a.tie),
        rowOf("Orden de expansión", (a) => a.order),
        rowOf("Completo", (a) => a.complete),
        rowOf("Óptimo", (a) => (a.key === "ASTAR" ? "Sí, en km (h admisible y consistente; ver detalle abajo)." : a.optimal)),
        rowOf("Tiempo (grafo)", (a) => a.time),
        rowOf("Espacio (grafo)", (a) => a.space),
        rowOf("Referencia: búsqueda en árbol", (a) => a.tree)))),
    h("ul", { class: "small muted algo-notes" }, ...ALGO_NOTES.map((n) => h("li", {}, n))),
    h("div", { class: "algo-grid", style: "margin-top:var(--s-4)" }, ...ALGORITHM_INFO.map((a) => h("article", { class: "card" },
      h("h3", {}, a.name),
      h("p", { class: "small", style: "margin:6px 0" }, h("strong", {}, "Optimalidad: "), a.optimal),
      h("p", { class: "small", style: "margin:6px 0" }, h("strong", {}, "Qué observar: "), a.watch),
      h("p", { class: "small", style: "margin:6px 0" }, h("strong", {}, "Métrica a comparar: "), a.metric),
      h("p", { class: "small muted" }, "En la asignación: ", h("strong", {}, a.business))))));
  root.append(pageHead("Cómo decide el PUM", "Agente de búsqueda en grafo sobre el mapa: formulación del problema, comparación de BFS, DFS, Greedy best-first y A*, reproducción de la traza de una ruta asignada y descarga de initial_state.json y sequence.json."), cards,
    tabs([["ruta", "Una ruta asignada"], ["comparar", "Comparar algoritmos"]], "ruta", (t) => (t === "ruta" ? showRoute() : showCompare())), body);

  function stopAll() {
    player?.destroy();
    player = null;
    if (typeof simCleanup === "function") simCleanup();
    simCleanup = null;
  }

  async function showCompare() {
    stopAll();
    body.replaceChildren(h("p", { class: "muted" }, "Elegí un tramo y corré los cuatro algoritmos sobre el mismo mapa. Compará nodes_explored, states_visited, nodes_in_frontier, max_depth y cost_total."));
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
              h("p", {}, "Reproducción de la traza: nodo expandido, frontera y explorados en cada paso.")),
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
