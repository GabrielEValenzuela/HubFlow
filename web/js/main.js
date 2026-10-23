import * as session from "./session.js";
import * as router from "./router.js";
import { h, toast, ROLE } from "./ui.js";
import { icon } from "./icons.js";
import { resetCaches } from "./data.js";

const OP = ["OPERADOR", "ADMIN"];
const ADMIN = ["ADMIN"];

router.define("/login", { load: () => import("./views/login.js") });
router.define("/pendientes", { roles: OP, load: () => import("./views/pending.js"), nav: ["Operación", "Pendientes", "inbox"] });
router.define("/mesa", { roles: OP, load: () => import("./views/sorting.js"), nav: ["Operación", "Mesa de clasificación", "layers"] });
router.define("/indices", { roles: OP, load: () => import("./views/indexes.js"), nav: ["Operación", "Índices", "tree"] });
router.define("/consultas", { roles: OP, load: () => import("./views/queries.js"), nav: ["Operación", "Consultas", "sets"] });
router.define("/metricas", { roles: OP, load: () => import("./views/metrics.js"), nav: ["Operación", "Métricas", "chart"] });
router.define("/rutas", { load: () => import("./views/routes.js"), nav: ["Reparto", "Rutas", "route"] });
router.define("/rutas/:id", { load: () => import("./views/routeDetail.js") });
router.define("/simulador", { roles: OP, load: () => import("./views/simulator.js"), nav: ["Reparto", "Simular PUM", "map"] });
router.define("/usuarios", { roles: ADMIN, load: () => import("./views/users.js"), nav: ["Administración", "Usuarios", "users"] });
router.define("/auditoria", { roles: ADMIN, load: () => import("./views/audit.js"), nav: ["Administración", "Auditoría", "shield"] });

const app = document.getElementById("app");
let main = null;
let nav = null;

function logout(message) {
  session.clear();
  resetCaches();
  if (message) toast(message, "error");
  router.navigate("/login");
}

window.addEventListener("hubflow:unauthorized", () => logout("Tu sesión terminó. Ingresá de nuevo."));

function buildShell() {
  const user = session.user();
  const isPum = user.role === "PUM";
  nav = h("nav", { class: "nav", "aria-label": "Principal" });
  main = h("main", { class: "main", id: "main", tabindex: "-1" });
  app.replaceChildren(
    h("div", { class: "shell" },
      h("aside", { class: "sidebar" },
        h("a", { class: "brand", href: "#/" }, h("span", { class: "brand-mark" }, icon("route", 18)), "HubFlow"),
        nav,
        h("div", { class: "sidebar-foot" },
          h("div", { class: "who" }, h("strong", {}, user.name), h("small", {}, ROLE[user.role] ?? user.role)),
          h("button", { class: "btn small", onClick: () => logout() }, icon("logout", 16), "Salir"))),
      main));
  return isPum;
}

function drawNav(activePath) {
  const role = session.role();
  let group = null;
  const items = [];
  for (const r of router.routeList()) {
    if (!r.nav || (r.roles && !r.roles.includes(role))) continue;
    const [g, label, ic] = r.nav;
    let [gl, lb] = [g, label];
    if (role === "PUM" && r.pattern === "/rutas") lb = "Mi hoja de ruta";
    if (gl !== group) {
      group = gl;
      items.push(h("div", { class: "nav-group" }, gl));
    }
    const active = activePath === r.pattern || (r.pattern === "/rutas" && activePath.startsWith("/rutas/"));
    items.push(h("a", { href: "#" + r.pattern, "aria-current": active ? "page" : null }, icon(ic), lb));
  }
  nav.replaceChildren(...items);
}

function changed(route, path) {
  if (route.pattern === "/login") {
    main = h("div", { id: "login-root" });
    app.replaceChildren(main);
    delete document.body.dataset.user;
    return;
  }
  if (!document.querySelector(".shell") || session.user()?.email !== document.body.dataset.user) {
    buildShell();
    document.body.dataset.user = session.user().email;
  }
  drawNav(path);
  document.title = "HubFlow";
}

router.start(() => main, changed);
