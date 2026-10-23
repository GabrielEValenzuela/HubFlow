import * as session from "./session.js";

const routes = [];
let cleanup = null;
let outlet = null;
let onChange = () => {};

export function define(pattern, { roles, load, nav }) {
  const keys = [];
  const rx = new RegExp("^" + pattern.replace(/:([a-z]+)/g, (_, k) => (keys.push(k), "([^/]+)")) + "$");
  routes.push({ pattern, rx, keys, roles, load, nav });
}

export const navigate = (path) => {
  if (location.hash === "#" + path) render();
  else location.hash = path;
};
export const routeList = () => routes;

export function start(getOutlet, changed) {
  outlet = getOutlet;
  onChange = changed;
  window.addEventListener("hashchange", render);
  render();
}

export function home() {
  return session.isOperator() ? "/pendientes" : "/rutas";
}

export async function render() {
  const path = location.hash.slice(1) || "/";
  if (!session.token()) {
    if (path !== "/login") return navigate("/login");
  }
  if (path === "/" || (path === "/login" && session.token())) return navigate(home());
  const route = routes.find((r) => r.rx.test(path));
  if (!route) return navigate(home());
  if (route.roles && !route.roles.includes(session.role())) return navigate(home());
  const m = route.rx.exec(path);
  const params = Object.fromEntries(route.keys.map((k, i) => [k, decodeURIComponent(m[i + 1])]));
  if (cleanup) {
    try {
      cleanup();
    } catch {
      /* la vista anterior ya no existe */
    }
    cleanup = null;
  }
  onChange(route, path);
  const root = outlet();
  root.replaceChildren();
  const view = await route.load();
  const token = (render.token = (render.token ?? 0) + 1);
  const result = await view.default(root, params);
  if (token !== render.token) {
    if (typeof result === "function") result();
    return;
  }
  cleanup = typeof result === "function" ? result : null;
  root.querySelector("h1")?.focus({ preventScroll: true });
}
