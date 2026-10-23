import { api } from "../api.js";
import { h, dialog, loading, errorBox, statusBadge, serviceBadge, zoneTag, bitChips, fmtDate, fmtKg, STATUS, promptNote, toast, tabs } from "../ui.js";

const ACTIONS = [
  ["CLASIFICADO", "Clasificar", "Registrá la ubicación del paquete en el centro de distribución.", "Clasificar envío"],
  ["REPROGRAMADO", "Reprogramar", "Registrá el motivo de la no entrega y cuándo se reintenta.", "Reprogramar envío"],
  ["ENTREGADO", "Entregar", "Registrá quién recibió el paquete.", "Registrar entrega"],
];

export function openShipment(code, onChange) {
  const body = h("div", {}, loading());
  const dlg = dialog({ title: code, sheet: true, body });
  let order = "asc";

  async function draw() {
    try {
      const [s, mv] = await Promise.all([api.get("/shipments/" + code), api.get(`/shipments/${code}/movements`, { order })]);
      body.replaceChildren(
        h("div", { class: "stack" },
          h("div", {}, statusBadge(s.status), " ", serviceBadge(s.service)),
          h("dl", { class: "kv" },
            h("dt", {}, "Destinatario"), h("dd", {}, s.recipient),
            h("dt", {}, "Zona"), h("dd", {}, zoneTag(s.zone)),
            h("dt", {}, "Punto de entrega"), h("dd", { class: "code" }, s.deliveryPoint),
            h("dt", {}, "Peso"), h("dd", {}, fmtKg(s.weightKg)),
            h("dt", {}, "Intentos"), h("dd", {}, String(s.attempts))),
          h("section", {}, h("h3", {}, "Estados por los que pasó"),
            h("p", { class: "muted small", style: "margin:4px 0 8px" }, `BitSet de 5 bits: cada bit marca un estado alcanzado (${s.stateHistory.count} de 5).`),
            bitChips(s.stateHistory),
            s.stateHistory.everRescheduled ? h("p", { class: "small", style: "margin-top:6px" }, "Este envío ya fue reprogramado alguna vez.") : null),
          h("section", {},
            h("div", { class: "card-head" }, h("h3", {}, `Movimientos (${s.movementCount})`),
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
    if (s.status === "ENTREGADO") return h("p", { class: "muted" }, "El envío ya fue entregado: no admite más movimientos.");
    return h("section", {}, h("h3", {}, "Registrar un movimiento"),
      h("div", { class: "actions", style: "margin-top:8px" }, ...ACTIONS.map(([status, label, hint, title]) =>
        h("button", { class: "btn", onClick: async () => {
          const note = await promptNote({ title, label: "Nota del movimiento", hint, confirmLabel: label });
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
