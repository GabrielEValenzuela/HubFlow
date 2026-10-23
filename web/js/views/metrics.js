import { api } from "../api.js";
import { h, pageHead, loading, errorBox, STATUS, BIT_ORDER, zoneTag, serviceBadge, statusBadge, fmtKg, ZONE, empty } from "../ui.js";
import { icon } from "../icons.js";

export default async function metrics(root) {
  const slot = h("div", {}, loading());
  const refresh = h("button", { class: "btn", onClick: load }, icon("reset"), "Actualizar");
  root.append(pageHead("Métricas", "Cómo viene el día: en qué estado está cada envío y qué pesa cada zona.", refresh), slot);

  async function load() {
    try {
      const m = await api.get("/metrics");
      const zones = Object.keys(ZONE);
      const [summaries, heavy] = await Promise.all([
        Promise.all(zones.map((z) => api.get(`/zones/${z}/summary`))),
        Promise.all(zones.map((z) => api.get(`/zones/${z}/heaviest-shipment`).catch(() => null))),
      ]);
      const max = Math.max(1, ...Object.values(m.byStatus));
      slot.replaceChildren(h("div", { class: "stack" },
        h("div", { class: "tiles" }, tile("Envíos totales", m.totalShipments), tile("Intentos de entrega", m.totalAttempts), tile("Pendientes de salir", m.pendingByZone.reduce((n, z) => n + z.count, 0))),
        h("section", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Envíos por estado")),
          h("div", { class: "bars" }, ...BIT_ORDER.map((s) => h("div", { class: "bar-row" }, statusBadge(s), h("div", { class: "bar-track", role: "img", "aria-label": `${STATUS[s]}: ${m.byStatus[s]}` }, h("span", { class: "s-" + s, style: `width:${(m.byStatus[s] / max) * 100}%` })), h("strong", { class: "code" }, String(m.byStatus[s])))))),
        h("section", {}, h("h2", { style: "margin-bottom:12px" }, "Por zona"),
          h("div", { class: "grid-3" }, ...zones.map((z, i) => {
            const s = summaries[i], hv = heavy[i];
            return h("div", { class: "card" }, h("div", { class: "card-head" }, h("h3", {}, zoneTag(z))),
              h("dl", { class: "kv" }, h("dt", {}, "Pendientes"), h("dd", {}, String(s.count)), h("dt", {}, "Express"), h("dd", {}, String(s.expressCount)), h("dt", {}, "Peso total"), h("dd", {}, fmtKg(s.totalWeightKg))),
              h("div", { style: "margin-top:12px;padding-top:12px;border-top:1px solid var(--line)" }, h("div", { class: "muted small" }, "Envío más pesado"),
                hv ? h("div", {}, h("span", { class: "code" }, hv.code), " · ", fmtKg(hv.weightKg), h("div", { class: "muted small" }, `${hv.recipient} · `, serviceBadge(hv.service))) : h("div", { class: "muted" }, "Sin envíos pendientes")));
          })))));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }
  const tile = (k, v) => h("div", { class: "tile" }, h("div", { class: "k" }, k), h("div", { class: "v" }, String(v)));
  load();
}
