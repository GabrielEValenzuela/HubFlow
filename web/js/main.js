import * as session from "./session.js";
import * as router from "./router.js";
import { h, toast, ROLE } from "./ui.js";
import { icon } from "./icons.js";
import { resetCaches, loadCatalogs } from "./data.js";

const PLANT = ["OPERADOR"];
const ADMIN = ["ADMIN"];
const MANAGEMENT = ["OPERADOR", "ADMIN"];

router.define("/login", { load: () => import("./views/login.js") });

// Navegación por puesto: el chofer ve solo su ruta; el jefe de planta, el movimiento del día; el administrador, la visión global.
router.define("/mi-ruta", { roles: ["PUM"], load: () => import("./views/myRoute.js"), nav: () => ["Mi trabajo", "Mi ruta", "route"] });
router.define("/planta", { roles: PLANT, load: () => import("./views/plant.js"), nav: () => ["Planta", "Paquetes en planta", "inbox"] });
router.define("/carga", { roles: PLANT, load: () => import("./views/loadPlan.js"), nav: () => ["Planta", "Armar carga", "layers"] });
router.define("/resumen", { roles: ADMIN, load: () => import("./views/summary.js"), nav: () => ["Operación", "Resumen de la operación", "chart"] });
router.define("/repartos", { roles: MANAGEMENT, load: () => import("./views/deliveries.js"), nav: (role) => [role === "ADMIN" ? "Operación" : "Planta", role === "ADMIN" ? "Repartos" : "Repartos en curso", "send"] });
router.define("/repartos/:id", { roles: MANAGEMENT, load: () => import("./views/routeDetail.js") });
router.define("/buscar", { roles: PLANT, load: () => import("./views/findShipments.js"), nav: () => ["Planta", "Buscar paquete", "search"] });
router.define("/personal", { roles: ADMIN, load: () => import("./views/staff.js"), nav: () => ["Administración", "Personal", "users"] });
router.define("/configuracion", { roles: ADMIN, load: () => import("./views/settings.js"), nav: () => ["Administración", "Configuración", "settings"] });
router.define("/actividad", { roles: ADMIN, load: () => import("./views/activity.js"), nav: () => ["Administración", "Actividad", "shield"] });

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
}

function drawNav(activePath) {
  const role = session.role();
  let group = null;
  const items = [];
  for (const r of router.routeList()) {
    if (!r.nav || (r.roles && !r.roles.includes(role))) continue;
    const [gl, lb, ic] = r.nav(role);
    if (gl !== group) {
      group = gl;
      items.push(h("div", { class: "nav-group" }, gl));
    }
    const active = activePath === r.pattern || (r.pattern === "/repartos" && activePath.startsWith("/repartos/"));
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

router.start(() => main, changed, () => loadCatalogs().catch(() => toast("No se pudieron cargar las zonas y los servicios. Actualizá la página.", "error")));
