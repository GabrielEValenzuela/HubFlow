import { h } from "../ui.js";
import { icon } from "../icons.js";
import { MapView } from "./mapview.js";
import { Replay, buildFrames } from "./replay.js";

const GLOSS = {
  solution_found: "¿Se encontró camino?",
  nodes_explored: "Nodos expandidos",
  states_visited: "Estados distintos visitados",
  nodes_in_frontier: "Nodos pendientes en la frontera",
  max_depth: "Profundidad de la solución",
  cost_total: "Costo total",
};
const SPEEDS = [0.5, 1, 2, 4];

export function metricsPanel(live) {
  const body = h("tbody", {});
  const render = (m, extra = {}) => {
    const rows = ["solution_found", "nodes_explored", "states_visited", "nodes_in_frontier", "max_depth", "cost_total"].map((k) => {
      let v = extra[k] ?? m?.[k];
      if (v === undefined || v === null) v = "—";
      else if (k === "solution_found") v = v ? "sí" : "no";
      else if (k === "cost_total") v = `${Number(v).toLocaleString("es-AR", { maximumFractionDigits: 1 })} km`;
      return h("tr", {}, h("td", {}, h("code", {}, k)), h("td", { class: "muted small" }, GLOSS[k]), h("td", { class: "num code" }, String(v)));
    });
    body.replaceChildren(...rows);
  };
  render(null);
  return { el: h("div", { class: "table-wrap metrics-panel" }, h("table", {}, h("thead", {}, h("tr", {}, h("th", {}, "Métrica"), h("th", {}, "Qué mide"), h("th", { class: "num" }, "Valor"))), body)), render };
}

export function transport(replay, label) {
  const play = h("button", { class: "btn primary icon-btn", "aria-label": "Reproducir", onClick: () => replay.toggle() }, icon("play"));
  const reset = h("button", { class: "btn icon-btn", "aria-label": "Volver al inicio", onClick: () => replay.reset() }, icon("reset"));
  const prev = h("button", { class: "btn icon-btn", "aria-label": "Paso anterior", onClick: () => replay.prev() }, icon("back"));
  const next = h("button", { class: "btn icon-btn", "aria-label": "Paso siguiente", onClick: () => { replay.pause(); replay.next(); } }, icon("step"));
  const speed = h("select", { class: "input", "aria-label": "Velocidad", style: "width:auto;min-height:40px", onChange: (e) => replay.setSpeed(Number(e.target.value)) },
    ...SPEEDS.map((s) => h("option", { value: s, selected: s === 1 }, `${s}x`)));
  const scrub = h("input", { type: "range", min: "-1", max: String(replay.length - 1), value: "-1", "aria-label": "Posición en la animación", class: "scrub",
    onInput: (e) => { replay.pause(); replay.seek(Number(e.target.value)); } });
  const state = () => {
    play.replaceChildren(icon(replay.playing ? "pause" : "play"));
    play.setAttribute("aria-label", replay.playing ? "Pausar" : "Reproducir");
    scrub.value = String(replay.index);
    scrub.setAttribute("aria-valuetext", `Paso ${replay.index + 1} de ${replay.length}`);
    prev.disabled = reset.disabled = replay.index < 0;
    next.disabled = replay.index >= replay.length - 1;
  };
  replay.onState = state;
  state();
  return h("div", { class: "transport", role: "group", "aria-label": label ?? "Controles de la animación" }, reset, prev, play, next, scrub, h("label", { class: "speed" }, h("span", { class: "sr-only" }, "Velocidad"), speed));
}

export function mountPlayer({ map, sequence, stops = [] }) {
  const frames = buildFrames(sequence);
  const view = new MapView(map, { stops, title: "Mapa de la ciudad con la búsqueda del repartidor" });
  const status = h("p", { class: "player-status", "aria-live": "polite" });
  const frontierBox = h("div", { class: "chips" });
  const exploredBox = h("div", { class: "chips" });
  const mp = metricsPanel();
  const totals = h("p", { class: "muted small" }, `Recorrido completo: ${sequence.metrics.nodes_explored} nodos expandidos · ${Number(sequence.metrics.cost_total).toLocaleString("es-AR", { maximumFractionDigits: 1 })} km.`);
  const doneSegs = [];

  const replay = new Replay(frames.length, (i) => {
    const f = frames[i];
    exploredBox.replaceChildren();
    frontierBox.replaceChildren();
    if (!f) {
      view.render({ courier: sequence.segments[0]?.from });
      status.textContent = "Listo para empezar. Tocá reproducir o avanzá paso a paso.";
      mp.render(null);
      return;
    }
    const prior = sequence.segments.slice(0, f.seg).map((s) => s.path);
    if (f.type === "step") {
      view.render({ expanded: f.expanded, frontier: f.frontier, explored: f.explored, done: prior, from: f.from, to: f.to, courier: f.from });
      status.textContent = `Tramo ${f.seg + 1} de ${sequence.segments.length} · ${f.from} → ${f.to} · paso ${f.index + 1} de ${f.count}: expande ${f.expanded}`;
      mp.render(null, { nodes_explored: f.explored.length, states_visited: f.explored.length, nodes_in_frontier: f.frontier.length });
    } else {
      view.render({ explored: f.explored, path: f.path, done: prior, from: f.from, to: f.to, courier: f.to, animate: true, speed: replay.speed });
      status.textContent = `Tramo ${f.seg + 1} de ${sequence.segments.length} · ${f.from} → ${f.to} · camino encontrado: ${f.path.join(" → ")}`;
      mp.render(f.metrics);
    }
    exploredBox.append(...f.explored.map((n) => h("span", { class: "chip" + (n === f.expanded ? " strong" : "") }, n)));
    frontierBox.append(...(f.frontier.length ? f.frontier : []).map((n) => h("span", { class: "chip front" }, n)));
    if (!f.frontier.length) frontierBox.append(h("span", { class: "muted small" }, f.type === "path" ? "El tramo terminó." : "Vacía"));
  });
  replay.seek(-1);

  const el = h("div", { class: "player" },
    h("div", { class: "player-map card" }, view.el,
      h("div", { class: "legend map-legend" },
        h("span", {}, h("i", { class: "l-explored" }), "Explorado"), h("span", {}, h("i", { class: "l-front" }), "En la frontera"), h("span", {}, h("i", { class: "l-exp" }), "Expandiendo ahora"),
        h("span", {}, h("i", { class: "l-path" }), "Camino elegido"), h("span", {}, h("i", { class: "l-stop" }), "Parada"))),
    h("div", { class: "player-side" }, transport(replay), status,
      h("div", { class: "card" }, h("h3", {}, "Frontera"), h("p", { class: "muted small", style: "margin:2px 0 8px" }, "Nodos que el agente conoce y todavía no visitó, en orden de turno."), frontierBox),
      h("div", { class: "card" }, h("h3", {}, "Explorados"), h("p", { class: "muted small", style: "margin:2px 0 8px" }, "Nodos ya expandidos en este tramo."), exploredBox),
      mp.el, totals));
  return { el, view, replay, destroy: () => { replay.destroy(); view.destroy(); } };
}
