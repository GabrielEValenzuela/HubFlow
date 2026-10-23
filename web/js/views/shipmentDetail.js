import { api } from "../api.js";
import { roadMap } from "../data.js";
import { h, dialog, loading, errorBox, statusBadge, serviceBadge, zoneTag, journeyChips, fmtDate, fmtKg, STATUS, FEATURE, promptNote, toast } from "../ui.js";

const ACTIONS = [
  ["CLASIFICADO", "Clasificar", "Anotá en qué lugar de la planta quedó el paquete.", "Clasificar paquete"],
  ["REPROGRAMADO", "Reprogramar", "Anotá por qué no se entregó y cuándo se vuelve a intentar.", "Reprogramar paquete"],
  ["ENTREGADO", "Entregar", "Anotá quién recibió el paquete.", "Registrar entrega"],
];

export function openShipment(code, onChange) {
  const body = h("div", {}, loading());
  const dlg = dialog({ title: code, sheet: true, body });
  let order = "asc";

  async function draw() {
    try {
      const [s, mv, map] = await Promise.all([api.get("/shipments/" + code), api.get(`/shipments/${code}/movements`, { order }), roadMap()]);
      const deliveryLabel = (id) => map.nodes.find((n) => n.id === id)?.label ?? id;
      body.replaceChildren(
        h("div", { class: "flow" },
          h("div", {}, statusBadge(s.status), " ", serviceBadge(s.service)),
          h("dl", { class: "kv" },
            h("dt", {}, "Destinatario"), h("dd", {}, s.recipient),
            h("dt", {}, "Zona"), h("dd", {}, zoneTag(s.zone)),
            h("dt", {}, "Lugar de entrega"), h("dd", {}, deliveryLabel(s.deliveryPoint)),
            h("dt", {}, "Peso"), h("dd", {}, fmtKg(s.weightKg)),
            s.features?.length ? h("dt", {}, "Cuidados") : null, s.features?.length ? h("dd", {}, s.features.map((f) => FEATURE[f] ?? f).join(", ")) : null,
            h("dt", {}, "Intentos"), h("dd", {}, String(s.attempts))),
          h("section", {}, h("h3", {}, "Recorrido del paquete"),
            journeyChips(s.stateHistory),
            s.stateHistory.everRescheduled ? h("p", { class: "small", style: "margin-top:6px" }, "Este paquete ya fue reprogramado alguna vez.") : null),
          h("section", {},
            h("div", { class: "card-head" }, h("h3", {}, `Historial (${s.movementCount})`),
              h("div", { class: "seg" },
                ...[["asc", "Más antiguos primero"], ["desc", "Más recientes primero"]].map(([v, l]) =>
                  h("button", { "aria-pressed": String(order === v), onClick: () => { order = v; draw(); } }, l)))),
            h("ol", { class: "timeline" }, ...mv.items.map((m) =>
              h("li", {}, h("div", {}, statusBadge(m.status)), h("div", {}, m.note), h("div", { class: "when" }, `${m.actor} · ${fmtDate(m.occurredAt)}`))))),
          actions(s)));
    } catch (e) {
      body.replaceChildren(errorBox(e, draw));
    }
  }

  function actions(s) {
    if (s.status === "ENTREGADO") return h("p", { class: "muted" }, "El paquete ya fue entregado. No se pueden agregar más anotaciones.");
    return h("section", {}, h("h3", {}, "Qué pasó con el paquete"),
      h("div", { class: "actions", style: "margin-top:8px" }, ...ACTIONS.map(([status, label, hint, title]) =>
        h("button", { class: "btn", onClick: async () => {
          const note = await promptNote({ title, label: "Nota", hint, confirmLabel: label });
          if (!note) return;
          try {
            await api.post(`/shipments/${code}/movements`, { status, note });
            toast(`${code}: ${STATUS[status].toLowerCase()}`);
            await draw();
            onChange?.();
          } catch (e) {
            toast(e.message, "error");
          }
        } }, label))));
  }
  draw();
  return dlg;
}
