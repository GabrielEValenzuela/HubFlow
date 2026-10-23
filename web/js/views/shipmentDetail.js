import { api } from "../api.js";
import { roadMap, trackingLink } from "../data.js";
import { h, dialog, loading, errorBox, statusBadge, serviceBadge, zoneTag, journeyChips, fmtDate, fmtKg, STATUS, FEATURE, promptNote, toast, confirmDialog, parcelsText } from "../ui.js";
import { printLabels } from "../lib/labels.js";
import { icon } from "../icons.js";

const ACTIONS = [
  ["CLASIFICADO", "Clasificar", "Anotá en qué lugar de la planta quedó el paquete.", "Clasificar paquete"],
  ["REPROGRAMADO", "Reprogramar", "Anotá por qué no se entregó y cuándo se vuelve a intentar.", "Reprogramar paquete"],
  ["ENTREGADO", "Entregar", "Anotá quién recibió el paquete. Se dan por entregados todos sus bultos.", "Registrar entrega"],
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
            h("dt", {}, "Teléfono"), h("dd", {}, s.recipientPhone),
            h("dt", {}, "Código postal"), h("dd", {}, s.recipientPostalCode),
            h("dt", {}, "Zona"), h("dd", {}, zoneTag(s.zone)),
            h("dt", {}, "Lugar de entrega"), h("dd", {}, deliveryLabel(s.deliveryPoint)),
            h("dt", {}, "Código de seguimiento"), h("dd", {}, h("span", { class: "code" }, s.code), " ",
              h("button", { class: "btn small", onClick: () => copyTrackingLink(s.code) }, icon("link", 16), "Copiar enlace de seguimiento")),
            h("dt", {}, "Bultos"), h("dd", {}, parcelsText(s.parcelCount) + (s.pendingParcels && s.pendingParcels < s.parcelCount ? ` (faltan entregar ${s.pendingParcels})` : "")),
            h("dt", {}, "Peso"), h("dd", {}, `${fmtKg(s.realWeightKg)} · se factura ${fmtKg(s.billableWeightKg)}`),
            s.features?.length ? h("dt", {}, "Cuidados") : null, s.features?.length ? h("dd", {}, s.features.map((f) => FEATURE[f] ?? f).join(", ")) : null,
            h("dt", {}, "Intentos"), h("dd", {}, String(s.attempts))),
          parcelsSection(s),
          togetherSection(s),
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

  async function copyTrackingLink(code) {
    try {
      const link = await trackingLink(code);
      await navigator.clipboard.writeText(link);
      toast("Enlace copiado. El cliente puede seguir su paquete desde ahí.");
    } catch {
      toast("No se pudo copiar el enlace. Probá de nuevo.", "error");
    }
  }

  function parcelsSection(s) {
    return h("section", {}, h("div", { class: "card-head" }, h("h3", {}, "Bultos"), h("button", { class: "btn small", onClick: () => printLabels(s) }, icon("printer", 16), "Imprimir etiquetas")),
      h("ul", { class: "parcel-list" }, ...s.parcels.map((p) => h("li", {},
        h("span", {}, h("strong", { class: "code" }, p.label), ` · ${p.lengthCm} × ${p.widthCm} × ${p.heightCm} cm`),
        h("span", { class: "muted" }, `${fmtKg(p.weightKg)} · se factura ${fmtKg(p.billableWeightKg)}`, p.delivered ? " · entregado" : "")))));
  }

  function togetherSection(s) {
    if (!s.group) return null;
    return h("section", {}, h("h3", {}, "Viaja junto con"),
      h("p", { class: "muted small" }, "Estos paquetes se cargan, se reparten en una sola parada y se reprograman juntos."),
      h("div", { class: "actions", style: "margin-top:8px" },
        ...s.group.travelsWith.map((c) => h("button", { class: "btn small", onClick: () => { dlg.close(); openShipment(c, onChange); } }, h("span", { class: "code" }, c))),
        s.status === "EN_REPARTO" ? null : h("button", { class: "btn small", onClick: async () => {
          if (!(await confirmDialog({ title: "Dejar de viajar juntos", text: "Los paquetes se van a poder cargar y repartir por separado.", confirmLabel: "Separar paquetes" }))) return;
          try {
            await api.del("/shipment-groups/" + s.group.id);
            toast("Los paquetes se pueden cargar por separado.");
            await draw();
            onChange?.();
          } catch (e) {
            toast(e.message, "error");
          }
        } }, "Separar paquetes")));
  }

  function actions(s) {
    if (s.status === "ENTREGADO") return h("p", { class: "muted" }, "El paquete ya fue entregado. No se pueden agregar más anotaciones.");
    return h("section", {}, h("h3", {}, "Qué pasó con el paquete"),
      h("div", { class: "actions", style: "margin-top:8px" }, ...ACTIONS.map(([status, label, hint, title]) =>
        h("button", { class: "btn", onClick: async () => {
          const note = await promptNote({ title, label: "Nota", hint, confirmLabel: label });
          if (!note) return;
          try {
            await api.post(`/shipments/${code}/movements`, { status, note, confirmAllParcels: status === "ENTREGADO" });
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
