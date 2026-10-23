import { api } from "../api.js";
import { roadMap } from "../data.js";
import { MapView } from "../lib/mapview.js";
import { buildFrames, Replay } from "../lib/replay.js";
import { transport } from "../lib/player.js";
import { h, pageHead, loading, errorBox, field, busy, ALGORITHM, ALGORITHMS, fmtKm, toast, empty } from "../ui.js";
import { icon } from "../icons.js";

export default async function simulator(root) {
  const slot = h("div", {}, loading());
  const out = h("div", { style: "margin-top:20px" });
  let replay = null;
  let views = [];
  root.append(pageHead("Simular PUM", "Elegí un tramo y mirá cómo cada algoritmo recorre el mismo mapa: cuánto explora y qué camino encuentra."), slot, out);
  try {
    const map = await roadMap();
    const opts = (sel) => map.nodes.map((n) => h("option", { value: n.id, selected: n.id === sel }, `${n.id} · ${n.label}`));
    const from = h("select", { class: "input" }, ...opts("HUB"));
    const to = h("select", { class: "input" }, ...opts("N5"));
    const checks = ALGORITHMS.map((a) => h("label", { class: "check" }, h("input", { type: "checkbox", value: a, checked: true }), ALGORITHM[a]));
    const btn = h("button", { class: "btn primary", type: "submit" }, icon("play"), "Comparar algoritmos");
    const form = h("form", { class: "card", novalidate: true }, h("div", { class: "form-grid" }, field("from", "Desde", from), field("to", "Hasta", to)),
      h("fieldset", { class: "crit", style: "margin-top:12px" }, h("legend", {}, "Algoritmos a comparar"), h("div", { class: "actions" }, ...checks)),
      h("div", { class: "form-error" }), h("div", { class: "form-actions" }, btn));
    slot.replaceChildren(form);
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      const algos = checks.map((c) => c.firstChild).filter((c) => c.checked).map((c) => c.value);
      form.querySelector(".form-error").replaceChildren();
      if (from.value === to.value) return void form.querySelector(".form-error").replaceChildren(errorBox(new Error("Elegí un origen y un destino distintos.")));
      if (!algos.length) return void form.querySelector(".form-error").replaceChildren(errorBox(new Error("Marcá al menos un algoritmo.")));
      try {
        const results = await busy(btn, () => Promise.all(algos.map((a) => api.get("/route-previews", { from: from.value, to: to.value, algorithm: a }))));
        show(results, map);
      } catch (err) {
        out.replaceChildren(errorBox(err));
      }
    });
  } catch (e) {
    slot.replaceChildren(errorBox(e));
  }

  function show(results, map) {
    replay?.destroy();
    views.forEach((v) => v.destroy());
    const frames = results.map((r) => buildFrames({ segments: [r] }));
    const longest = Math.max(...frames.map((f) => f.length));
    const best = Math.min(...results.map((r) => r.metrics.cost_total));
    const cards = results.map((r, i) => {
      const view = new MapView(map, { compact: true, title: ALGORITHM[r.algorithm] });
      views.push(view);
      const status = h("p", { class: "small muted", "aria-live": "off" });
      const live = h("dl", { class: "kv small" });
      return { r, view, status, live, el: h("section", { class: "card" + (r.metrics.cost_total === best ? " winner" : "") },
        h("div", { class: "card-head" }, h("h3", {}, ALGORITHM[r.algorithm]), r.metrics.cost_total === best ? h("span", { class: "badge s-ENTREGADO" }, "Camino más corto") : null),
        view.el, status, h("dl", { class: "kv small", style: "margin-top:8px" },
          h("dt", {}, "Camino"), h("dd", { class: "code" }, r.path.join(" → ")),
          h("dt", {}, "cost_total"), h("dd", { class: "code" }, fmtKm(r.metrics.cost_total)),
          h("dt", {}, "nodes_explored"), h("dd", { class: "code" }, String(r.metrics.nodes_explored)),
          h("dt", {}, "max_depth"), h("dd", { class: "code" }, String(r.metrics.max_depth)),
          h("dt", {}, "nodes_in_frontier"), h("dd", { class: "code" }, String(r.metrics.nodes_in_frontier)))) };
    });
    replay = new Replay(longest, (idx) => {
      cards.forEach((c, i) => {
        const fr = frames[i];
        const f = idx < 0 ? null : fr[Math.min(idx, fr.length - 1)];
        if (!f) { c.view.render({ courier: c.r.from, from: c.r.from, to: c.r.to }); c.status.textContent = "Sin empezar."; return; }
        if (f.type === "step") {
          c.view.render({ expanded: f.expanded, frontier: f.frontier, explored: f.explored, from: f.from, to: f.to, courier: f.from });
          c.status.textContent = `Paso ${f.index + 1} de ${f.count}: expande ${f.expanded}`;
        } else {
          c.view.render({ explored: f.explored, path: f.path, from: f.from, to: f.to, courier: f.to, animate: idx <= fr.length - 1, speed: replay.speed });
          c.status.textContent = `Terminó: ${f.path.length - 1} tramos de calle.`;
        }
      });
    });
    replay.seek(-1);
    out.replaceChildren(h("div", { class: "stack" }, transport(replay, "Controles de la comparación"), h("div", { class: "compare" }, ...cards.map((c) => c.el))));
  }
  return () => {
    replay?.destroy();
    views.forEach((v) => v.destroy());
  };
}
