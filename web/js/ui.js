import { icon } from "./icons.js";

export function h(tag, attrs, ...children) {
  const el = document.createElement(tag);
  for (const [k, v] of Object.entries(attrs ?? {})) {
    if (v === undefined || v === null || v === false) continue;
    if (k === "class") el.className = v;
    else if (k.startsWith("on") && typeof v === "function") el.addEventListener(k.slice(2).toLowerCase(), v);
    else if (k === "dataset") Object.assign(el.dataset, v);
    else if (k === "html") el.innerHTML = v;
    else if (v === true) el.setAttribute(k, "");
    else el.setAttribute(k, v);
  }
  append(el, children);
  return el;
}

export function append(el, children) {
  for (const c of children.flat(Infinity)) {
    if (c === null || c === undefined || c === false) continue;
    el.append(c instanceof Node ? c : document.createTextNode(String(c)));
  }
  return el;
}

export function svg(tag, attrs, ...children) {
  const el = document.createElementNS("http://www.w3.org/2000/svg", tag);
  for (const [k, v] of Object.entries(attrs ?? {})) {
    if (v === undefined || v === null || v === false) continue;
    if (k.startsWith("on") && typeof v === "function") el.addEventListener(k.slice(2).toLowerCase(), v);
    else el.setAttribute(k, v);
  }
  append(el, children);
  return el;
}

export const clear = (el) => {
  el.replaceChildren();
  return el;
};

export const STATUS = {
  RECIBIDO: "Recibido",
  CLASIFICADO: "Clasificado",
  EN_REPARTO: "En reparto",
  REPROGRAMADO: "Reprogramado",
  ENTREGADO: "Entregado",
  PENDIENTE: "Pendiente",
  EN_CURSO: "En curso",
  COMPLETADA: "Completada",
};

/** Zonas, servicios y características vienen de la API (se cargan al iniciar sesión); estos objetos se rellenan en el lugar. */
export const ZONE = {};
export const SERVICE = {};
export const FEATURE = {};
export const CATALOG = { zones: [], services: [], features: [] };
const SERVICE_RANK = new Map();
const ZONE_POSITION = new Map();
const ZONE_SHAPES = 3;
const ZONE_COLORS = 6;

function refill(target, items) {
  for (const k of Object.keys(target)) delete target[k];
  for (const it of items) target[it.code] = it.name;
}

export function setCatalogs({ zones = [], services = [], features = [] }) {
  CATALOG.zones = zones;
  CATALOG.services = [...services].sort((a, b) => b.priority - a.priority);
  CATALOG.features = features;
  refill(ZONE, zones);
  refill(SERVICE, CATALOG.services);
  refill(FEATURE, features);
  ZONE_POSITION.clear();
  zones.forEach((z, i) => ZONE_POSITION.set(z.code, i));
  SERVICE_RANK.clear();
  CATALOG.services.forEach((sv, i) => SERVICE_RANK.set(sv.code, i));
}

/** Opciones [código, nombre] para elegir; por defecto solo las habilitadas. */
export const zoneChoices = ({ all = false } = {}) => CATALOG.zones.filter((z) => all || z.active).map((z) => [z.code, z.name]);
export const serviceChoices = ({ all = false } = {}) => CATALOG.services.filter((sv) => all || sv.active).map((sv) => [sv.code, sv.name]);
export const featureChoices = ({ all = false } = {}) => CATALOG.features.filter((f) => all || f.active).map((f) => [f.code, f.name]);

/** Servicios habilitados en el orden en que salen de la planta: "Express, Prioritario y Estándar". */
export const serviceOrderSentence = () => {
  const names = CATALOG.services.filter((sv) => sv.active).map((sv) => sv.name);
  return names.length > 1 ? `${names.slice(0, -1).join(", ")} y ${names[names.length - 1]}` : names.join("");
};
export const topServiceName = () => CATALOG.services.find((sv) => sv.active)?.name ?? "";
export const zoneColor = (code) => `var(--zone-${((ZONE_POSITION.get(code) ?? 0) % ZONE_COLORS) + 1})`;
export const zoneShape = (code) => (ZONE_POSITION.get(code) ?? 0) % ZONE_SHAPES;
export const serviceTone = (code) => {
  const rank = SERVICE_RANK.get(code) ?? 2;
  return rank === 0 ? "tone-top" : rank === 1 ? "tone-mid" : "tone-low";
};
export const ROLE = { ADMIN: "Administrador", OPERADOR: "Jefe de planta", PUM: "Chofer" };
export const STATE_ORDER = ["RECIBIDO", "CLASIFICADO", "EN_REPARTO", "REPROGRAMADO", "ENTREGADO"];

export const statusBadge = (s) => h("span", { class: "badge s-" + s }, STATUS[s] ?? s);
export const serviceBadge = (s) => h("span", { class: "badge plain " + serviceTone(s) }, SERVICE[s] ?? s);
export const zoneTag = (z) => h("span", { class: "zone-tag zs-" + zoneShape(z), style: `--zc:${zoneColor(z)}` }, ZONE[z] ?? z);

/** Estados por los que pasó un paquete, en el orden del ciclo de vida. */
export function journeyChips(state) {
  const padded = (state?.bits ?? "00000").padStart(5, "0");
  const passed = STATE_ORDER.filter((_, pos) => padded[4 - pos] === "1");
  const wrap = h("ul", { class: "journey", "aria-label": "Pasó por" });
  passed.forEach((name) => wrap.append(h("li", { class: "journey-step s-" + name }, STATUS[name])));
  return h("div", { class: "journey-row" }, h("span", { class: "journey-label" }, "Pasó por:"),
    passed.length ? wrap : h("span", { class: "muted" }, "todavía ningún estado"));
}

export function fmtDate(sec) {
  if (!sec) return "";
  return new Date(sec * 1000).toLocaleString("es-AR", { dateStyle: "short", timeStyle: "short" });
}
export const fmtKg = (n) => `${Number(n).toLocaleString("es-AR", { maximumFractionDigits: 2 })} kg`;
export const fmtKm = (n) => `${Number(n).toLocaleString("es-AR", { maximumFractionDigits: 1 })} km`;

export function toast(message, kind = "ok") {
  const box = document.getElementById("toasts");
  const t = h("div", { class: "toast" + (kind === "error" ? " error" : "") }, message);
  box.append(t);
  setTimeout(() => t.remove(), kind === "error" ? 6500 : 3500);
}

export function loading(rows = 4) {
  return h("div", { class: "loading-block", role: "status", "aria-label": "Cargando" },
    ...Array.from({ length: rows }, (_, i) => h("div", { class: "skeleton", style: `width:${92 - i * 11}%` })));
}

export function empty(title, text, ...actions) {
  return h("div", { class: "empty" }, h("h3", {}, title), text ? h("p", {}, text) : null,
    actions.length ? h("div", { class: "actions" }, ...actions) : null);
}

export function errorBox(err, onRetry) {
  const list = (err.fields ?? []).filter((f) => f.message);
  return h("div", { class: "alert error", role: "alert" }, icon("alert"),
    h("div", {}, h("strong", {}, err.message || "Ocurrió un error"),
      list.length ? h("ul", {}, ...list.map((f) => h("li", {}, f.field ? `${f.field}: ${f.message}` : f.message))) : null,
      onRetry ? h("div", { style: "margin-top:8px" }, h("button", { class: "btn small", onClick: onRetry }, "Reintentar")) : null));
}

export function field(name, label, control, { hint } = {}) {
  const id = "f-" + name + "-" + Math.random().toString(36).slice(2, 7);
  control.id = id;
  control.setAttribute("name", name);
  const err = h("div", { class: "error", id: id + "-err" });
  control.setAttribute("aria-describedby", id + "-err");
  return h("div", { class: "field", dataset: { field: name } }, h("label", { for: id }, label), control,
    hint ? h("div", { class: "hint" }, hint) : null, err);
}

export function setFieldError(root, name, message) {
  const f = root.querySelector(`[data-field="${name}"]`);
  if (!f) return false;
  f.classList.toggle("invalid", !!message);
  f.querySelector(".error").textContent = message || "";
  const c = f.querySelector("input,select,textarea");
  if (c) c.setAttribute("aria-invalid", message ? "true" : "false");
  return true;
}

export function clearFieldErrors(root) {
  root.querySelectorAll("[data-field]").forEach((f) => setFieldError(root, f.dataset.field, ""));
  root.querySelector(".form-error")?.replaceChildren();
}

export function showApiErrors(root, err) {
  clearFieldErrors(root);
  const rest = [];
  for (const f of err.fields ?? []) {
    if (!f.field || !setFieldError(root, f.field, f.message)) rest.push(f);
  }
  const first = root.querySelector(".field.invalid .input");
  const slot = root.querySelector(".form-error");
  if (slot) {
    const e = new Error(err.fields?.length && !rest.length ? "Revisá los campos marcados." : err.message);
    e.fields = rest;
    slot.replaceChildren(errorBox(e));
  } else toast(err.message, "error");
  first?.focus();
}

export async function busy(button, fn) {
  button.classList.add("loading");
  button.disabled = true;
  try {
    return await fn();
  } finally {
    button.classList.remove("loading");
    button.disabled = false;
  }
}

export function dialog({ title, body, sheet = false, onClose }) {
  const dlg = h("dialog", { class: sheet ? "sheet" : "", "aria-labelledby": "dlg-title" });
  const close = h("button", { class: "btn ghost icon-btn", "aria-label": "Cerrar", onClick: () => dlg.close() }, icon("x"));
  dlg.append(h("div", { class: "dlg-head" }, h("h2", { id: "dlg-title" }, title), close), h("div", { class: "dlg-body" }, body));
  dlg.addEventListener("close", () => {
    dlg.remove();
    onClose?.();
  });
  dlg.addEventListener("click", (e) => {
    if (e.target === dlg) dlg.close();
  });
  document.body.append(dlg);
  dlg.showModal();
  return dlg;
}

export function confirmDialog({ title, text, confirmLabel, danger = false }) {
  return new Promise((resolve) => {
    let answer = false;
    const ok = h("button", { class: "btn " + (danger ? "danger" : "primary"), onClick: () => { answer = true; dlg.close(); } }, confirmLabel);
    const dlg = dialog({ title, body: [h("p", {}, text), h("div", { class: "actions" }, ok, h("button", { class: "btn", onClick: () => dlg.close() }, "Cancelar"))], onClose: () => resolve(answer) });
    ok.focus();
  });
}

export function promptNote({ title, label, confirmLabel, hint }) {
  return new Promise((resolve) => {
    let result = null;
    const input = h("textarea", { class: "input", rows: "3", maxlength: "200" });
    const f = field("note", label, input, { hint });
    const form = h("form", { novalidate: true }, f, h("div", { class: "form-actions" },
      h("button", { class: "btn primary", type: "submit" }, confirmLabel),
      h("button", { class: "btn", type: "button", onClick: () => dlg.close() }, "Cancelar")));
    form.addEventListener("submit", (e) => {
      e.preventDefault();
      const v = input.value.trim();
      if (v.length < 3) return setFieldError(form, "note", "Escribí al menos 3 caracteres.");
      result = v;
      dlg.close();
    });
    const dlg = dialog({ title, body: form, onClose: () => resolve(result) });
    input.focus();
  });
}

export function tabs(items, active, onChange) {
  const bar = h("div", { class: "tabs", role: "tablist" });
  const buttons = items.map(([id, label]) =>
    h("button", { role: "tab", "aria-selected": String(id === active), dataset: { id }, onClick: () => select(id) }, label));
  bar.append(...buttons);
  bar.addEventListener("keydown", (e) => {
    const i = buttons.findIndex((b) => b === document.activeElement);
    if (i < 0 || !["ArrowRight", "ArrowLeft"].includes(e.key)) return;
    const n = buttons[(i + (e.key === "ArrowRight" ? 1 : -1) + buttons.length) % buttons.length];
    n.focus();
    n.click();
  });
  function select(id) {
    buttons.forEach((b) => b.setAttribute("aria-selected", String(b.dataset.id === id)));
    onChange(id);
  }
  return bar;
}

export function subHead(title, subtitle, ...actions) {
  return h("div", { class: "page-head sub" }, h("div", {}, h("h2", {}, title), subtitle ? h("p", {}, subtitle) : null),
    actions.length ? h("div", { class: "actions" }, ...actions) : null);
}

export function pageHead(title, subtitle, ...actions) {
  return h("div", { class: "page-head" }, h("div", {}, h("h1", { tabindex: "-1" }, title), subtitle ? h("p", {}, subtitle) : null),
    actions.length ? h("div", { class: "actions" }, ...actions) : null);
}

export function shipmentTable(items, { extra, hit } = {}) {
  const head = h("tr", {}, ...["Código", "Destinatario", "Zona", "Servicio", "Peso", "Estado"].map((t, i) => h("th", { class: i === 4 ? "num" : "" }, t)), extra ? h("th", {}, "") : null);
  const rows = items.map((s) =>
    h("tr", { class: hit?.(s) ? "is-hit" : "" },
      h("td", { class: "code" }, s.code), h("td", {}, s.recipient), h("td", {}, zoneTag(s.zone)), h("td", {}, serviceBadge(s.service)),
      h("td", { class: "num" }, fmtKg(s.weightKg)), h("td", {}, statusBadge(s.status)), extra ? h("td", {}, extra(s)) : null));
  return h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, head), h("tbody", {}, ...rows)));
}
