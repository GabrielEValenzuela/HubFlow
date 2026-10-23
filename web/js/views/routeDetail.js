import { api } from "../api.js";
import { roadMap } from "../data.js";
import { MapView } from "../lib/mapview.js";
import { optionLabel, estimatedMinutes, fmtDuration, TRAVEL_ESTIMATE } from "../lib/routeOptions.js";
import { h, pageHead, loading, errorBox, statusBadge, fmtKm, STATUS, toast, promptNote, busy } from "../ui.js";
import { icon } from "../icons.js";
import { reassignRoute } from "./reassign.js";

export default async function routeDetail(root, { id }) {
  const slot = h("div", {}, loading());
  let view = null;
  let animTimer = null;
  root.append(slot);

  async function load() {
    try {
      const [route, map] = await Promise.all([api.get("/routes/" + id), roadMap()]);
      draw(route, map);
    } catch (e) {
      slot.replaceChildren(pageHead("Reparto #" + id), errorBox(e, load), h("p", {}, h("a", { href: "#/repartos" }, "Volver a los repartos")));
    }
  }

  function draw(route, map) {
    view?.destroy();
    clearTimeout(animTimer);
    const labels = new Map(map.nodes.map((n) => [n.id, n.label]));
    const segs = route.sequence.segments;
    const legKm = (stop) => segs[stop.order - 1]?.metrics?.cost_total ?? 0;
    const next = route.status === "EN_CURSO" ? route.stops.find((s) => s.outcome === "PENDIENTE") : null;
    const travelMinutes = estimatedMinutes(route.totalCostKm, route.stopCount);
    const pct = route.stopCount ? Math.round((route.resolvedStops / route.stopCount) * 100) : 0;

    // Mapa: solo el camino planificado, lo ya recorrido y la posición del repartidor.
    const leading = route.stops.findIndex((s) => s.outcome === "PENDIENTE");
    const doneCount = leading === -1 ? route.stops.length + 1 : leading;
    const position = doneCount === 0 ? "HUB" : doneCount > route.stops.length ? "HUB" : route.stops[doneCount - 1].nodeId;
    view = new MapView(map, { stops: route.stops, title: "Mapa con el recorrido del reparto" });
    const drawState = () => view.render({
      done: segs.slice(0, doneCount).map((s) => s.path),
      path: segs.slice(doneCount).flatMap((s) => s.path),
      courier: position,
    });
    drawState();
    const replayBtn = h("button", { class: "btn small", onClick: () => {
      if (matchMedia("(prefers-reduced-motion: reduce)").matches) return toast("Se muestra el recorrido completo; tenés el movimiento reducido en tu sistema.");
      replayBtn.disabled = true;
      view.render({ path: route.sequence.path, courier: "HUB", animate: true, speed: 0.1 });
      announce.textContent = "Mostrando el recorrido completo de la jornada.";
      animTimer = setTimeout(() => { drawState(); replayBtn.disabled = false; announce.textContent = ""; }, 8200);
    } }, icon("play", 16), "Ver el recorrido");
    const announce = h("p", { class: "sr-only", "aria-live": "polite" });

    const stopsEl = h("ol", { class: "stops", "aria-label": "Paradas" },
      ...route.stops.map((s) => {
        const pending = s.outcome === "PENDIENTE";
        const actions = s.outcome === "REPROGRAMADO" ? h("span", { class: "badge s-REPROGRAMADO" }, "No entregado") : statusBadge(s.outcome);
        return h("li", { class: "stop " + s.outcome + (next && next.order === s.order ? " is-next" : "") },
          h("span", { class: "n", "aria-hidden": "true" }, s.outcome === "ENTREGADO" ? "✓" : s.outcome === "REPROGRAMADO" ? "!" : String(s.order)),
          h("div", {}, h("div", {}, h("strong", {}, labels.get(s.nodeId) ?? "Parada"), " · ", h("span", { class: "code muted" }, s.shipmentCodes.join(", ")), s.shipmentCodes.length > 1 ? h("span", { class: "together" }, "viajan juntos") : null),
            h("div", { class: "small muted" }, `Parada ${s.order} · ${fmtKm(legKm(s))} desde la anterior${pending ? "" : " · " + (s.outcome === "REPROGRAMADO" ? "No entregado" : STATUS[s.outcome])}`)), actions);
      }));

    const tile = (k, v, sub) => h("div", { class: "tile" }, h("div", { class: "k" }, k), h("div", { style: "font-size:1.25rem;font-weight:700;margin-top:4px" }, v), sub ?? null);
    const finished = route.status !== "EN_CURSO";
    slot.replaceChildren(...[
      h("p", { class: "small", style: "margin-bottom:8px" }, h("a", { href: "#/repartos" }, "← Repartos")),
      pageHead(`Reparto #${route.id}`, `${route.courier.name} · ${optionLabel(route.algorithm)}`, statusBadge(route.status),
        finished ? null : h("button", { class: "btn", onClick: () => reassignRoute(route, load) }, "Pasar a otro chofer")),
      h("div", { class: "tiles", style: "margin-bottom:var(--s-4)" },
        tile("Prioridad", optionLabel(route.algorithm)),
        tile("Distancia", fmtKm(route.totalCostKm)),
        tile("Tiempo estimado", "≈ " + fmtDuration(travelMinutes), h("div", { class: "small muted" }, TRAVEL_ESTIMATE.note)),
        tile("Paradas hechas", `${route.resolvedStops} de ${route.stopCount}`, h("div", { class: "progress", role: "progressbar", "aria-valuemin": "0", "aria-valuemax": String(route.stopCount), "aria-valuenow": String(route.resolvedStops), "aria-label": "Paradas hechas", style: "margin-top:8px" }, h("span", { style: `width:${pct}%` })))),
      next ? h("div", { class: "next-stop", role: "status" }, icon("flag", 22),
        h("div", {}, h("div", { class: "big" }, `Próxima parada: ${labels.get(next.nodeId) ?? "—"}, ${fmtKm(legKm(next))}`))) : null,
      finished ? h("div", { class: "alert info", style: "margin-bottom:var(--s-4)" }, h("div", {}, route.status === "COMPLETADA" ? "El chofer terminó todas las paradas." : "Esta ruta ya no está en curso.")) : null,
      h("div", { class: "route-layout" },
        h("section", { class: "card" }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Paradas"), h("p", {}, "En el orden en que se entregan. El recorrido termina en el Centro de Distribución."))), stopsEl),
        h("section", { class: "card" },
          h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Recorrido"), h("p", {}, "Camino previsto sobre el mapa.")), replayBtn),
          view.el, announce,
          h("div", { class: "legend map-legend" },
            h("span", {}, h("i", { class: "l-path" }), "Falta recorrer"), h("span", {}, h("i", { class: "l-done" }), "Ya recorrido"),
            h("span", {}, h("i", { class: "l-stop" }), "Parada"), h("span", {}, h("i", { class: "l-courier" }), "Chofer"))))].filter(Boolean));
  }

  load();
  return () => { clearTimeout(animTimer); view?.destroy(); };
}
