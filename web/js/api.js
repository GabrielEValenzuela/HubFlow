import * as session from "./session.js";

const BASE = "/api/v1";
const pendingKeys = new Map();

export class ApiError extends Error {
  constructor(status, problem) {
    super(problem?.detail || problem?.title || "No se pudo completar la operación");
    this.status = status;
    this.code = problem?.code;
    this.title = problem?.title;
    this.fields = problem?.errors ?? [];
  }
}

function url(path, query) {
  const params = new URLSearchParams();
  for (const [k, v] of Object.entries(query ?? {})) {
    if (v !== undefined && v !== null && v !== "") params.set(k, v);
  }
  const qs = params.toString();
  return BASE + path + (qs ? "?" + qs : "");
}

async function send(method, path, { query, body, headers = {}, raw = false } = {}) {
  const h = { Accept: "application/json", ...headers };
  const tk = session.token();
  if (tk) h.Authorization = "Bearer " + tk;
  if (body !== undefined) h["Content-Type"] = "application/json";
  let res;
  try {
    res = await fetch(url(path, query), { method, headers: h, body: body === undefined ? undefined : JSON.stringify(body) });
  } catch {
    throw new ApiError(0, { detail: "No hay conexión con el servidor. Revisá tu red e intentá de nuevo." });
  }
  if (res.status === 401 && path !== "/sessions") {
    session.clear();
    window.dispatchEvent(new CustomEvent("hubflow:unauthorized"));
    throw new ApiError(401, { detail: "Tu sesión terminó. Ingresá de nuevo." });
  }
  if (!res.ok) {
    let problem = null;
    try {
      problem = await res.json();
    } catch {
      problem = { detail: res.statusText };
    }
    throw new ApiError(res.status, problem);
  }
  if (raw) return res;
  if (res.status === 204) return null;
  return res.json();
}

async function postOnce(path, body = {}) {
  const sig = path + "|" + JSON.stringify(body);
  let key = pendingKeys.get(sig);
  if (!key) {
    key = crypto.randomUUID();
    pendingKeys.set(sig, key);
  }
  const result = await send("POST", path, { body, headers: { "Idempotency-Key": key } });
  pendingKeys.delete(sig);
  return result;
}

export const api = {
  get: (path, query) => send("GET", path, { query }),
  post: postOnce,
  signIn: (email, password) => send("POST", "/sessions", { body: { email, password } }),
  patch: (path, body) => send("PATCH", path, { body }),
  put: (path, body) => send("PUT", path, { body }),
  del: (path) => send("DELETE", path),
  async download(path, filename) {
    const res = await send("GET", path, { raw: true });
    const blob = await res.blob();
    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = filename;
    document.body.append(a);
    a.click();
    a.remove();
    setTimeout(() => URL.revokeObjectURL(a.href), 2000);
  },
};
