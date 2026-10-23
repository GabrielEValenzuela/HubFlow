import { api } from "../api.js";
import { roadMap } from "../data.js";
import { MapView } from "../lib/mapview.js";
import { h, pageHead, loading, errorBox, empty, field, busy, toast, dialog, fmtKm } from "../ui.js";
import { icon } from "../icons.js";

const REASONS = ["No había nadie", "La dirección está cerrada o no existe", "No quisieron recibirlo", "Otro motivo"];
const OUTCOME = { PENDIENTE: ["Pendiente", "s-PENDIENTE"], ENTREGADO: ["Entregado", "s-ENTREGADO"], REPROGRAMADO: ["No se pudo entregar", "s-REPROGRAMADO"] };

/** Pantalla del chofer: su ruta del día, la próxima parada y dos botones grandes. */
export default async function myRoute(root) {
  const slot = h("div", {}, loading());
  let view = null;
  root.append(pageHead("Mi ruta"), slot);

  async function load() {
    try {
      const list = await api.get("/routes", { limit: 100 });
      if (!list.items.length) {
        return slot.replaceChildren(empty("Hoy no tenés una ruta", "Cuando el jefe de planta te asigne paquetes, tu ruta aparece acá."));
      }
      const active = list.items.filter((r) => r.status === "EN_CURSO").sort((a, b) => a.id - b.id)[0];
      const chosen = active ?? list.items.slice().sort((a, b) => b.id - a.id)[0];
      const [route, map] = await Promise.all([api.get("/routes/" + chosen.id), roadMap()]);
      draw(route, map);
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }

  function draw(route, map) {
    view?.destroy();
    const labels = new Map(map.nodes.map((n) => [n.id, n.label]));
    const segs = route.sequence.segments;
    const legKm = (stop) => segs[stop.order - 1]?.metrics?.cost_total ?? 0;
    const finished = route.status !== "EN_CURSO";
    const next = finished ? null : route.stops.find((s) => s.outcome === "PENDIENTE");
    const delivered = route.stops.filter((s) => s.outcome === "ENTREGADO").length;
    const failed = route.stops.filter((s) => s.outcome === "REPROGRAMADO").length;
    const pct = route.stopCount ? Math.round((route.resolvedStops / route.stopCount) * 100) : 0;

    const leading = route.stops.findIndex((s) => s.outcome === "PENDIENTE");
    const doneCount = leading === -1 ? route.stops.length + 1 : leading;
    const position = doneCount === 0 || doneCount > route.stops.length ? "HUB" : route.stops[doneCount - 1].nodeId;
    view = new MapView(map, { stops: route.stops, title: "Mapa con tu recorrido" });
    view.render({ done: segs.slice(0, doneCount).map((s) => s.path), path: segs.slice(doneCount).flatMap((s) => s.path), courier: position });

    async function deliver(btn, stop) {
      try {
        const updated = await busy(btn, () => api.patch(`/routes/${route.id}/stops/${stop.order}`, { outcome: "ENTREGADO", note: "Entregado en destino" }));
        toast(`Entregaste ${stop.shipmentCode}.`);
        draw({ ...updated, sequence: updated.sequence ?? route.sequence }, map);
      } catch (e) {
        toast(e.message, "error");
      }
    }

    async function notDelivered(stop) {
      const note = await askReason(stop);
      if (!note) return;
      try {
        const updated = await api.patch(`/routes/${route.id}/stops/${stop.order}`, { outcome: "REPROGRAMADO", note });
        toast(`${stop.shipmentCode} vuelve a la planta.`);
        draw({ ...updated, sequence: updated.sequence ?? route.sequence }, map);
      } catch (e) {
        toast(e.message, "error");
      }
    }

    const nextCard = next
      ? h("section", { class: "driver-next", "aria-labelledby": "next-title" },
        h("p", { class: "driver-kicker", id: "next-title" }, `Próxima parada (${next.order} de ${route.stopCount})`),
        h("p", { class: "driver-place" }, labels.get(next.nodeId) ?? "Parada"),
        next.recipient ? h("p", { class: "driver-recipient" }, "Entregar a ", h("strong", {}, next.recipient)) : null,
        h("p", { class: "driver-meta" }, h("span", {}, "Paquete ", h("strong", { class: "code" }, next.shipmentCode)),
          next.service ? h("span", {}, `${next.service === "EXPRESS" ? "Express" : "Estándar"} · ${next.weightKg} kg`) : null, h("span", {}, `${fmtKm(legKm(next))} desde la parada anterior`)),
        h("div", { class: "driver-actions" },
          h("button", { class: "btn big ok", onClick: (e) => deliver(e.currentTarget, next) }, icon("check", 24), "Entregado"),
          h("button", { class: "btn big", onClick: () => notDelivered(next) }, "No se pudo entregar")))
      : h("section", { class: "driver-next done" }, icon("check", 28), h("div", {}, h("p", { class: "driver-place" }, "Terminaste tu ruta"),
        h("p", { class: "driver-meta" }, `Entregaste ${delivered} ${delivered === 1 ? "paquete" : "paquetes"}.` + (failed ? ` ${failed} vuelve${failed === 1 ? "" : "n"} a la planta.` : ""))));

    const stopList = h("ol", { class: "stops", "aria-label": "Paradas de la ruta" }, ...route.stops.map((s) => {
      const [text, cls] = OUTCOME[s.outcome] ?? [s.outcome, ""];
      return h("li", { class: "stop " + s.outcome + (next && next.order === s.order ? " is-next" : "") },
        h("span", { class: "n", "aria-hidden": "true" }, s.outcome === "ENTREGADO" ? "✓" : s.outcome === "REPROGRAMADO" ? "!" : String(s.order)),
        h("div", {}, h("strong", {}, labels.get(s.nodeId) ?? "Parada"), s.recipient ? h("div", {}, s.recipient) : null, h("div", { class: "small muted" }, h("span", { class: "code" }, s.shipmentCode), ` · ${fmtKm(legKm(s))}`)),
        h("span", { class: "badge " + cls }, text));
    }));

    slot.replaceChildren(
      h("div", { class: "driver" },
        h("div", { class: "driver-progress" },
          h("div", { class: "driver-count" }, h("strong", {}, `${route.resolvedStops} de ${route.stopCount}`), " paradas hechas"),
          h("div", { class: "progress", role: "progressbar", "aria-valuemin": "0", "aria-valuemax": String(route.stopCount), "aria-valuenow": String(route.resolvedStops), "aria-label": "Paradas hechas" }, h("span", { style: `width:${pct}%` }))),
        nextCard,
        h("section", { class: "card" }, h("h2", {}, "Todas las paradas"), h("div", { style: "margin-top:12px" }, stopList)),
        h("section", { class: "card" }, h("h2", { style: "margin-bottom:12px" }, "Recorrido"), view.el,
          h("div", { class: "legend map-legend" }, h("span", {}, h("i", { class: "l-path" }), "Falta recorrer"), h("span", {}, h("i", { class: "l-done" }), "Ya recorrido"), h("span", {}, h("i", { class: "l-stop" }), "Parada"), h("span", {}, h("i", { class: "l-courier" }), "Vos")))));
  }

  function askReason(stop) {
    return new Promise((resolve) => {
      let answer = null;
      const radios = REASONS.map((r, i) => h("label", { class: "reason" }, h("input", { type: "radio", name: "reason", value: r, checked: i === 0 }), h("span", {}, r)));
      const extra = h("textarea", { class: "input", rows: "2", maxlength: "150" });
      const form = h("form", { novalidate: true },
        h("fieldset", { class: "crit" }, h("legend", {}, `¿Por qué no se pudo entregar ${stop.shipmentCode}?`), h("div", { class: "reasons" }, ...radios)),
        field("extra", "Querés agregar algo (opcional)", extra),
        h("div", { class: "form-actions" }, h("button", { class: "btn primary big", type: "submit" }, "Confirmar"), h("button", { class: "btn big", type: "button", onClick: () => dlg.close() }, "Volver")));
      form.addEventListener("submit", (e) => {
        e.preventDefault();
        const reason = form.querySelector("input[name=reason]:checked").value;
        const more = extra.value.trim();
        answer = more ? `${reason}: ${more}` : reason;
        dlg.close();
      });
      const dlg = dialog({ title: "No se pudo entregar", body: form, onClose: () => resolve(answer) });
    });
  }

  load();
  return () => view?.destroy();
}
