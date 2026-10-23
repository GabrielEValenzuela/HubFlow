import { api } from "../api.js";
import { roadMap } from "../data.js";
import { mountPlayer } from "../lib/player.js";
import { h, pageHead, loading, errorBox, statusBadge, fmtKm, ALGORITHM, STATUS, toast, promptNote, busy, tabs } from "../ui.js";
import { icon } from "../icons.js";
import * as session from "../session.js";

export default async function routeDetail(root, { id }) {
  const slot = h("div", {}, loading());
  let player = null;
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
    player?.destroy();
    const pum = session.role() === "PUM";
    const labels = new Map(map.nodes.map((n) => [n.id, n.label]));
    player = mountPlayer({ map, sequence: route.sequence, stops: route.stops });
    const stopsEl = h("ol", { class: "stops", "aria-label": "Paradas" });
    const refreshStops = (r) => {
      stopsEl.replaceChildren(...r.stops.map((s) => {
        const pending = s.outcome === "PENDIENTE";
        const actions = pum && pending && r.status === "EN_CURSO" ? h("div", { class: "actions" },
          h("button", { class: "btn primary small", onClick: (e) => resolve(e.currentTarget, s, "ENTREGADO") }, icon("check", 16), "Entregado"),
          h("button", { class: "btn small", onClick: (e) => resolve(e.currentTarget, s, "REPROGRAMADO") }, "Reprogramar")) : h("div", {}, statusBadge(s.outcome));
        return h("li", { class: "stop " + s.outcome }, h("span", { class: "n", "aria-hidden": "true" }, s.outcome === "ENTREGADO" ? "✓" : s.outcome === "REPROGRAMADO" ? "!" : String(s.order)),
          h("div", {}, h("div", {}, h("strong", { class: "code" }, s.shipmentCode), " · ", h("span", { class: "muted" }, `${s.nodeId} ${labels.get(s.nodeId) ?? ""}`)),
            h("div", { class: "small muted" }, `Parada ${s.order}${pending ? "" : " · " + STATUS[s.outcome]}`)), actions);
      }));
    };
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
    refreshStops(route);
    const download = (path, name) => h("button", { class: "btn small", onClick: (e) => busy(e.currentTarget, () => api.download(path, name)).catch((err) => toast(err.message, "error")) }, icon("download", 16), name);
    slot.replaceChildren(
      h("p", { class: "small", style: "margin-bottom:8px" }, h("a", { href: "#/rutas" }, "← ", pum ? "Mi hoja de ruta" : "Rutas")),
      pageHead(`Ruta #${route.id}`, `${route.courier.name} · ${ALGORITHM[route.algorithm] ?? route.algorithm} · ${fmtKm(route.totalCostKm)} · ${route.resolvedStops} de ${route.stopCount} paradas resueltas`,
        statusBadge(route.status), pum ? null : h("a", { class: "btn small", href: "#/simulador" }, icon("map", 16), "Simular PUM")),
      h("section", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Paradas"), h("p", {}, "En el orden de descarga; al terminar el repartidor vuelve al centro.")), stopsEl),
      h("section", { style: "margin-top:24px" },
        h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Recorrido del agente"), h("p", {}, "Reproducí cómo el agente fue descartando calles hasta dar con el camino.")),
          h("div", { class: "actions" }, download(`/routes/${id}/initial-state`, "initial_state.json"), download(`/routes/${id}/sequence`, "sequence.json"))),
        player.el));
  }

  load();
  return () => player?.destroy();
}
