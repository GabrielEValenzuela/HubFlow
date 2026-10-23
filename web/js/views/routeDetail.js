import { api } from "../api.js";
import { roadMap } from "../data.js";
import { MapView } from "../lib/mapview.js";
import { optionLabel, etaMinutes, fmtDuration, ETA } from "../lib/routeOptions.js";
import { h, pageHead, loading, errorBox, statusBadge, fmtKm, STATUS, toast, promptNote, busy } from "../ui.js";
import { icon } from "../icons.js";
import * as session from "../session.js";

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
      slot.replaceChildren(pageHead("Ruta #" + id), errorBox(e, load), h("p", {}, h("a", { href: "#/rutas" }, "Volver a las rutas")));
    }
  }

  function draw(route, map) {
    view?.destroy();
    clearTimeout(animTimer);
    const pum = session.role() === "PUM";
    const labels = new Map(map.nodes.map((n) => [n.id, n.label]));
    const segs = route.sequence.segments;
    const legKm = (stop) => segs[stop.order - 1]?.metrics?.cost_total ?? 0;
    const next = route.status === "EN_CURSO" ? route.stops.find((s) => s.outcome === "PENDIENTE") : null;
    const eta = etaMinutes(route.totalCostKm, route.stopCount);
    const pct = route.stopCount ? Math.round((route.resolvedStops / route.stopCount) * 100) : 0;

    // Mapa: solo el camino planificado, lo ya recorrido y la posición del repartidor.
    const leading = route.stops.findIndex((s) => s.outcome === "PENDIENTE");
    const doneCount = leading === -1 ? route.stops.length + 1 : leading;
    const position = doneCount === 0 ? "HUB" : doneCount > route.stops.length ? "HUB" : route.stops[doneCount - 1].nodeId;
    view = new MapView(map, { stops: route.stops, title: "Mapa de la ciudad con el recorrido planificado" });
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
    } }, icon("play", 16), "Ver recorrido");
    const announce = h("p", { class: "sr-only", "aria-live": "polite" });

    const stopsEl = h("ol", { class: "stops", "aria-label": "Paradas" },
      ...route.stops.map((s) => {
        const pending = s.outcome === "PENDIENTE";
        const actions = pum && pending && route.status === "EN_CURSO" ? h("div", { class: "actions" },
          h("button", { class: "btn primary small", onClick: (e) => resolve(e.currentTarget, s, "ENTREGADO") }, icon("check", 16), "Entregado"),
          h("button", { class: "btn small", onClick: (e) => resolve(e.currentTarget, s, "REPROGRAMADO") }, "Reprogramar")) : h("div", {}, statusBadge(s.outcome));
        return h("li", { class: "stop " + s.outcome + (next && next.order === s.order ? " is-next" : "") },
          h("span", { class: "n", "aria-hidden": "true" }, s.outcome === "ENTREGADO" ? "✓" : s.outcome === "REPROGRAMADO" ? "!" : String(s.order)),
          h("div", {}, h("div", {}, h("strong", {}, labels.get(s.nodeId) ?? "Parada"), " · ", h("span", { class: "code muted" }, s.shipmentCode)),
            h("div", { class: "small muted" }, `Parada ${s.order} · ${fmtKm(legKm(s))} desde la anterior${pending ? "" : " · " + STATUS[s.outcome]}`)), actions);
      }));

    async function resolve(btn, stop, outcome) {
      let note = "Entregado en destino";
      if (outcome === "REPROGRAMADO") {
        note = await promptNote({ title: `Reprogramar ${stop.shipmentCode}`, label: "Motivo", hint: "Por ejemplo: nadie atendió, dirección cerrada.", confirmLabel: "Reprogramar entrega" });
        if (!note) return;
      }
      try {
        const updated = await busy(btn, () => api.patch(`/routes/${id}/stops/${stop.order}`, { outcome, note }));
        toast(outcome === "ENTREGADO" ? `${stop.shipmentCode} entregado.` : `${stop.shipmentCode} vuelve a pendientes.`);
        draw({ ...updated, sequence: updated.sequence ?? route.sequence }, map);
      } catch (e) {
        toast(e.message, "error");
      }
    }

    const tile = (k, v, sub) => h("div", { class: "tile" }, h("div", { class: "k" }, k), h("div", { style: "font-size:1.25rem;font-weight:700;margin-top:4px" }, v), sub ?? null);
    const finished = route.status !== "EN_CURSO";
    slot.replaceChildren(...[
      h("p", { class: "small", style: "margin-bottom:8px" }, h("a", { href: "#/rutas" }, "← ", pum ? "Mi hoja de ruta" : "Rutas")),
      pageHead(`Ruta #${route.id}`, `${route.courier.name} · ${optionLabel(route.algorithm)}`, statusBadge(route.status),
        pum ? null : h("a", { class: "btn small", href: "#/como-decide/" + route.id }, icon("map", 16), "¿Cómo se calculó?")),
      h("div", { class: "tiles", style: "margin-bottom:var(--s-4)" },
        tile("Prioridad", optionLabel(route.algorithm)),
        tile("Distancia", fmtKm(route.totalCostKm)),
        tile("Tiempo estimado", "≈ " + fmtDuration(eta), h("div", { class: "small muted" }, ETA.note)),
        tile("Entregas", `${route.resolvedStops} de ${route.stopCount}`, h("div", { class: "progress", role: "progressbar", "aria-valuemin": "0", "aria-valuemax": String(route.stopCount), "aria-valuenow": String(route.resolvedStops), "aria-label": "Entregas resueltas", style: "margin-top:8px" }, h("span", { style: `width:${pct}%` })))),
      next ? h("div", { class: "next-stop", role: "status" }, icon("flag", 22),
        h("div", {}, h("div", { class: "big" }, `Próxima parada: ${labels.get(next.nodeId) ?? "—"}, ${fmtKm(legKm(next))}`))) : null,
      finished ? h("div", { class: "alert info", style: "margin-bottom:var(--s-4)" }, h("div", {}, route.status === "COMPLETADA" ? "Todas las paradas están resueltas." : "Esta ruta ya no está en curso.")) : null,
      h("div", { class: "route-layout" },
        h("section", { class: "card" }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Paradas"), h("p", {}, "En el orden de descarga; al terminar se vuelve al centro de distribución."))), stopsEl),
        h("section", { class: "card" },
          h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Recorrido"), h("p", {}, "Camino planificado y avance del repartidor.")), replayBtn),
          view.el, announce,
          h("div", { class: "legend map-legend" },
            h("span", {}, h("i", { class: "l-path" }), "Por recorrer"), h("span", {}, h("i", { class: "l-done" }), "Ya recorrido"),
            h("span", {}, h("i", { class: "l-stop" }), "Parada"), h("span", {}, h("i", { class: "l-courier" }), "Repartidor"))))].filter(Boolean));
  }

  load();
  return () => { clearTimeout(animTimer); view?.destroy(); };
}
