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
export const SERVICE = { EXPRESS: "Express", PRIORITARIO: "Prioritario", ESTANDAR: "Estándar" };
export const ZONE = { CENTRO: "Centro", NORTE: "Norte", SUR: "Sur" };
export const ROLE = { ADMIN: "Administrador", OPERADOR: "Operador", PUM: "Repartidor (PUM)" };
export const BIT_ORDER = ["RECIBIDO", "CLASIFICADO", "EN_REPARTO", "REPROGRAMADO", "ENTREGADO"];

export const statusBadge = (s) => h("span", { class: "badge s-" + s }, STATUS[s] ?? s);
export const serviceBadge = (s) => h("span", { class: "badge plain svc-" + s }, SERVICE[s] ?? s);
export const zoneTag = (z) => h("span", { class: "zone-tag z-" + z }, ZONE[z] ?? z);

export function bitChips(state) {
  const bits = state?.bits ?? "00000";
  const padded = bits.padStart(5, "0");
  const wrap = h("span", { class: "bits", role: "list", "aria-label": "Estados por los que pasó el envío" });
  for (let pos = 4; pos >= 0; pos--) {
    const on = padded[4 - pos] === "1";
    const name = BIT_ORDER[pos];
    wrap.append(
      h("span", { class: "bit" + (on ? " on" : ""), role: "listitem", title: `${STATUS[name]}: ${on ? "sí" : "no"}` },
        h("b", {}, on ? "1" : "0"), STATUS[name]),
    );
  }
  return wrap;
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
