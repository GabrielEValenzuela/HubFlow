const API = "/api/v1/public/tracking/";
const REFRESH_MS = 60000;

const $ = (id) => document.getElementById(id);
const form = $("buscar");
const input = $("codigo");
const codeError = $("codigo-error");
const result = $("resultado");

let state = { code: null, token: null, expiresAt: 0, view: null, details: null };
let timer = null;

function h(tag, attrs = {}, ...kids) {
  const el = document.createElement(tag);
  for (const [k, v] of Object.entries(attrs)) {
    if (v === null || v === undefined || v === false) continue;
    if (k.startsWith("on")) el.addEventListener(k.slice(2).toLowerCase(), v);
    else el.setAttribute(k, v === true ? "" : v);
  }
  for (const kid of kids.flat()) if (kid !== null && kid !== undefined && kid !== false) el.append(kid.nodeType ? kid : document.createTextNode(String(kid)));
  return el;
}

const dayFmt = new Intl.DateTimeFormat("es-AR", { weekday: "long", day: "numeric", month: "long" });
const hourFmt = new Intl.DateTimeFormat("es-AR", { hour: "2-digit", minute: "2-digit" });
const when = (sec) => {
  const d = new Date(sec * 1000);
  return `${dayFmt.format(d)}, ${hourFmt.format(d)}`;
};
const hour = (sec) => hourFmt.format(new Date(sec * 1000));
const minutesText = (s) => { const m = Math.max(1, Math.ceil(s / 60)); return m === 1 ? "1 minuto" : `${m} minutos`; };

async function call(method, path, { body, token } = {}) {
  const headers = { Accept: "application/json" };
  if (body !== undefined) headers["Content-Type"] = "application/json";
  if (token) headers.Authorization = "Bearer " + token;
  let res;
  try {
    res = await fetch(API + path, { method, headers, body: body === undefined ? undefined : JSON.stringify(body), cache: "no-store" });
  } catch {
    throw { kind: "network" };
  }
  if (res.ok) return res.json();
  let problem = {};
  try { problem = await res.json(); } catch { /* sin cuerpo */ }
  throw { kind: res.status === 404 ? "notfound" : res.status === 429 ? "limited" : res.status === 401 ? "expired" : "other", status: res.status, retryAfter: Number(res.headers.get("Retry-After")) || 0, problem };
}

function say(message) {
  codeError.textContent = message;
  input.setAttribute("aria-invalid", message ? "true" : "false");
}

function showMessage(kind, error) {
  const text = {
    network: "No pudimos conectarnos. Revisá tu conexión y probá de nuevo.",
    notfound: "No encontramos un paquete con esos datos. Revisá el código y lo que ingresaste.",
    limited: `Hiciste varios intentos seguidos. Por tu seguridad, esperá ${minutesText(error?.retryAfter || 900)} y volvé a probar.`,
    other: "Tuvimos un problema de nuestro lado. Probá de nuevo en unos minutos.",
  }[kind] ?? "Tuvimos un problema. Probá de nuevo en unos minutos.";
  return text;
}

function stop() {
  clearInterval(timer);
  timer = null;
}

function schedule() {
  stop();
  if (state.view && !state.view.finished) {
    timer = setInterval(() => { if (!document.hidden) refresh(); }, REFRESH_MS);
  }
}

async function refresh() {
  try {
    const data = await call("GET", encodeURIComponent(state.code), { token: state.token });
    state.view = data;
    state.details = data.details ?? null;
    draw();
  } catch (e) {
    if (e.kind === "expired") {
      state.token = null;
      state.details = null;
      draw("Por seguridad el detalle se ocultó. Si querés verlo de nuevo, confirmá tus datos.");
    }
  }
}

function setUrl(code) {
  const url = new URL(location.href);
  url.searchParams.set("codigo", code);
  history.replaceState(null, "", url);
}

async function search(raw) {
  const code = raw.trim().toUpperCase().replace(/\s+/g, "");
  say("");
  if (code.length < 8) return say("Escribí el código completo, por ejemplo HF-XXXXXXXXX.");
  const button = form.querySelector("button");
  button.disabled = true;
  try {
    state = { code, token: null, expiresAt: 0, view: await call("GET", encodeURIComponent(code)), details: null };
    input.value = code;
    setUrl(code);
    draw();
    schedule();
    result.querySelector("h2")?.focus();
  } catch (e) {
    state.view = null;
    result.hidden = true;
    stop();
    say(showMessage(e.kind, e));
  } finally {
    button.disabled = false;
  }
}

function timeline(view) {
  return h("ol", { class: "timeline", "aria-label": "Recorrido del paquete" },
    view.timeline.map((step, i, all) => h("li", { class: i === all.length - 1 ? "now" : "", "aria-current": i === all.length - 1 ? "step" : null },
      h("div", { class: "what" }, step.label), h("div", { class: "at" }, when(step.at)))));
}

function detailsCard(d, view) {
  const parcels = d.parcels.total === 1 ? "1 bulto" : `${d.parcels.total} bultos`;
  const delivered = d.parcels.delivered === d.parcels.total ? (d.parcels.total === 1 ? "ya fue entregado" : "ya se entregaron todos") : `${d.parcels.delivered} de ${d.parcels.total} entregados`;
  let eta = null;
  if (d.estimatedArrival) {
    const e = d.estimatedArrival;
    const before = e.stopsBefore === 0 ? "Tu paquete es la próxima parada." : e.stopsBefore === 1 ? "Hay una entrega antes que la tuya." : `Hay ${e.stopsBefore} entregas antes que la tuya.`;
    eta = h("div", { class: "eta" }, h("div", {}, "Lo esperamos entre las "), h("strong", {}, `${hour(e.from)} y las ${hour(e.to)}`), h("div", {}, before),
      h("p", { class: "note" }, "Es una estimación: puede cambiar según el tránsito y las demás entregas."));
  }
  return h("div", { class: "card details", id: "detalle" },
    h("h3", {}, `Hola, ${d.recipientFirstName}`),
    eta,
    h("dl", { class: "facts" },
      h("div", {}, h("dt", {}, "Zona"), h("dd", {}, d.zone)),
      h("div", {}, h("dt", {}, "Barrio de entrega"), h("dd", {}, d.neighbourhood)),
      h("div", {}, h("dt", {}, "Tu envío"), h("dd", {}, `${parcels}, ${delivered}`))),
    h("p", { class: "note" }, "Por tu seguridad, estos datos se ocultan solos a los 15 minutos."));
}

function verifyCard(notice) {
  let mode = "phone";
  const field = h("input", { class: "input", id: "prueba", inputmode: "numeric", autocomplete: "off", maxlength: "4", "aria-describedby": "prueba-error" });
  const error = h("p", { class: "error", id: "prueba-error", role: "alert" });
  const submit = h("button", { class: "btn primary", type: "submit" }, "Confirmar y ver más");
  const label = h("label", { for: "prueba" }, "Últimos 4 dígitos del teléfono");
  const choose = (value) => {
    mode = value;
    field.value = "";
    field.setAttribute("maxlength", mode === "phone" ? "4" : "12");
    field.setAttribute("inputmode", mode === "phone" ? "numeric" : "text");
    label.textContent = mode === "phone" ? "Últimos 4 dígitos del teléfono" : "Código postal";
    error.textContent = "";
  };
  const radio = (value, text) => h("label", {}, h("input", { type: "radio", name: "tipo", value, checked: value === mode, onChange: () => choose(value) }), text);
  const box = h("form", { class: "card more", id: "verificar", novalidate: true },
    h("h3", {}, "¿Querés ver más detalles?"),
    h("p", { class: "help" }, notice ?? "Para cuidar tus datos, primero necesitamos confirmar que sos quien espera el paquete."),
    h("fieldset", { class: "choices" }, h("legend", { class: "sr-only" }, "Cómo querés confirmar"), radio("phone", "Con mi teléfono"), radio("postal", "Con mi código postal")),
    label, field, error, h("div", { style: "margin-top:12px" }, submit));
  box.addEventListener("submit", async (event) => {
    event.preventDefault();
    error.textContent = "";
    const value = field.value.trim();
    if (mode === "phone" && !/^\d{4}$/.test(value)) return (error.textContent = "Escribí los 4 últimos dígitos, solo números.");
    if (mode === "postal" && !/^(\d{4}|[A-Za-z]\d{4}[A-Za-z]{3})$/.test(value.replace(/\s/g, ""))) return (error.textContent = "Escribí tu código postal: 4 números (1425) o con letras (C1425ABC).");
    submit.disabled = true;
    try {
      const access = await call("POST", encodeURIComponent(state.code) + "/verifications", { body: mode === "phone" ? { phoneLast4: value } : { postalCode: value } });
      state.token = access.token;
      state.expiresAt = access.expiresAt;
      await refresh();
      $("detalle")?.scrollIntoView({ block: "start", behavior: "smooth" });
    } catch (e) {
      error.textContent = e.kind === "notfound" ? "Ese dato no coincide con el de este paquete. Revisalo y probá de nuevo." : showMessage(e.kind, e);
      submit.disabled = e.kind === "limited";
      if (e.kind === "limited") setTimeout(() => { submit.disabled = false; }, (e.retryAfter || 900) * 1000);
      return;
    }
    submit.disabled = false;
  });
  return box;
}

function draw(notice) {
  const view = state.view;
  const current = view.status;
  result.hidden = false;
  result.replaceChildren(
    h("div", { class: "hero", "data-stage": current.stage },
      h("p", { class: "kicker" }, view.code),
      h("h2", { tabindex: "-1" }, current.label),
      h("p", { class: "when" }, when(current.at))),
    h("div", { class: "card" }, h("h3", {}, "El recorrido de tu paquete"), timeline(view)),
    state.details ? detailsCard(state.details, view) : verifyCard(notice));
}

form.addEventListener("submit", (event) => {
  event.preventDefault();
  search(input.value);
});

const initial = new URLSearchParams(location.search).get("codigo");
if (initial) {
  input.value = initial.toUpperCase();
  search(initial);
}
