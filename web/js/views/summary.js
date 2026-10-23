import { api } from "../api.js";
import { h, pageHead, loading, errorBox, STATUS, STATE_ORDER, zoneTag, zoneColor, serviceBadge, statusBadge, fmtKg, ZONE, topServiceName } from "../ui.js";
import { icon } from "../icons.js";

/** Resumen de la operación para el administrador: números grandes y barras simples. */
export default async function summary(root) {
  const slot = h("div", {}, loading());
  const refresh = h("button", { class: "btn", onClick: load }, icon("reset"), "Actualizar");
  root.append(pageHead("Resumen de la operación", "Cómo va el día: dónde están los paquetes y cuánto pesa lo que espera en la planta.", refresh), slot);

  async function load() {
    try {
      const m = await api.get("/metrics");
      const shown = m.pendingByZone.filter((z) => z.active || z.count > 0);
      const zones = shown.map((z) => z.zone);
      const sums = shown;
      const heavy = await Promise.all(zones.map((z) => api.get(`/zones/${z}/heaviest-shipment`).catch(() => null)));
      const inPlant = m.pendingByZone.reduce((n, z) => n + z.count, 0);
      const maxKg = Math.max(0.01, ...sums.map((s) => s.totalWeightKg));
      const busiest = sums.map((s, i) => ({ z: zones[i], kg: s.totalWeightKg })).sort((a, b) => b.kg - a.kg)[0] ?? { z: "", kg: 0 };
      const maxState = Math.max(1, ...Object.values(m.byStatus));
      slot.replaceChildren(h("div", { class: "flow" },
        h("div", { class: "big-tiles" },
          big("Paquetes en planta", inPlant, "esperando salir"),
          big("En reparto", m.byStatus.EN_REPARTO ?? 0, "ya salieron con un chofer"),
          big("Entregados", m.byStatus.ENTREGADO ?? 0, "recibidos por el cliente", "ok"),
          big("No entregados", m.byStatus.REPROGRAMADO ?? 0, `${m.totalAttempts} ${m.totalAttempts === 1 ? "intento" : "intentos"} de entrega en total`, "warn")),
        h("section", { class: "card" }, h("div", { class: "card-head" }, h("div", {}, h("h2", {}, "Kilos en planta por zona"),
          h("p", {}, busiest.kg > 0 ? `La zona más cargada es ${ZONE[busiest.z]}, con ${fmtKg(busiest.kg)}.` : "No hay paquetes esperando en la planta."))),
          h("div", { class: "bars" }, ...zones.map((z, i) => h("div", { class: "bar-row" }, zoneTag(z),
            h("div", { class: "bar-track", role: "img", "aria-label": `${ZONE[z]}: ${fmtKg(sums[i].totalWeightKg)}` }, h("span", { class: "z-fill", style: `--zc:${zoneColor(z)};width:${(sums[i].totalWeightKg / maxKg) * 100}%` })),
            h("strong", { class: "code" }, fmtKg(sums[i].totalWeightKg)))))),
        h("section", { class: "card" }, h("div", { class: "card-head" }, h("h2", {}, "Paquetes según su situación")),
          h("div", { class: "bars" }, ...STATE_ORDER.map((s) => h("div", { class: "bar-row" }, statusBadge(s),
            h("div", { class: "bar-track", role: "img", "aria-label": `${STATUS[s]}: ${m.byStatus[s]}` }, h("span", { class: "s-" + s, style: `width:${(m.byStatus[s] / maxState) * 100}%` })),
            h("strong", { class: "code" }, String(m.byStatus[s])))))),
        h("section", {}, h("h2", { style: "margin-bottom:12px" }, "Cada zona"),
          h("div", { class: "grid-3" }, ...zones.map((z, i) => {
            const s = sums[i], hv = heavy[i];
            return h("div", { class: "card" }, h("div", { class: "card-head" }, h("h3", {}, zoneTag(z))),
              h("dl", { class: "kv" }, h("dt", {}, "Paquetes en planta"), h("dd", {}, String(s.count)), h("dt", {}, topServiceName() || "Urgentes"), h("dd", {}, String(s.urgentCount)), h("dt", {}, "Peso"), h("dd", {}, fmtKg(s.totalWeightKg))),
              h("div", { style: "margin-top:12px;padding-top:12px;border-top:1px solid var(--line)" }, h("div", { class: "muted small" }, "Paquete más pesado"),
                hv ? h("div", {}, h("span", { class: "code" }, hv.code), " · ", fmtKg(hv.weightKg), h("div", { class: "muted small" }, `${hv.recipient} · `, serviceBadge(hv.service))) : h("div", { class: "muted" }, "No hay paquetes esperando")));
          })))));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }
  const big = (k, v, sub, tone) => h("div", { class: "big-tile" + (tone ? " " + tone : "") }, h("div", { class: "k" }, k), h("div", { class: "v" }, String(v)), h("div", { class: "sub" }, sub));
  load();
}
