const KEY = "hubflow.session";
let memory = null;

export function load() {
  if (memory) return memory;
  try {
    const raw = sessionStorage.getItem(KEY);
    memory = raw ? JSON.parse(raw) : null;
  } catch {
    memory = null;
  }
  return memory;
}

export function save(session) {
  memory = session;
  try {
    sessionStorage.setItem(KEY, JSON.stringify(session));
  } catch {
    /* sin almacenamiento: la sesión vive solo en memoria */
  }
}

export function clear() {
  memory = null;
  try {
    sessionStorage.removeItem(KEY);
  } catch {
    /* nada que limpiar */
  }
}

export const token = () => {
  const s = load();
  if (!s) return null;
  if (s.expiresAt && s.expiresAt * 1000 < Date.now()) {
    clear();
    return null;
  }
  return s.token;
};
export const user = () => load()?.user ?? null;
export const role = () => user()?.role ?? null;
export const isOperator = () => role() === "OPERADOR" || role() === "ADMIN";
