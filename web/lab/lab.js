import * as session from "../js/session.js";
import { api } from "../js/api.js";
import { h, tabs, errorBox, field, busy, showApiErrors, clearFieldErrors, setFieldError } from "../js/ui.js";
import { icon } from "../js/icons.js";

const TABS = [
  ["lista", "Lista de pendientes", () => import("./pendingList.js")],
  ["mesa", "Mesa: Queue y Stack", () => import("./queueStack.js")],
  ["indices", "Índices: BST y B+", () => import("./indexes.js")],
  ["conjuntos", "Set y BitSet", () => import("./sets.js")],
  ["decide", "Cómo decide el PUM", () => import("./pum.js")],
];
const app = document.getElementById("app");
let cleanup = null;
let token = 0;

const allowed = () => ["OPERADOR", "ADMIN"].includes(session.role()) && session.token();
const parts = () => location.hash.slice(1).split("/").filter(Boolean);

window.addEventListener("hubflow:unauthorized", () => start());

function start() {
  if (typeof cleanup === "function") cleanup();
  cleanup = null;
  allowed() ? shell() : login();
}

function login() {
  const email = h("input", { class: "input", type: "email", autocomplete: "username", required: true });
  const password = h("input", { class: "input", type: "password", autocomplete: "current-password", required: true });
  const submit = h("button", { class: "btn primary", type: "submit", style: "width:100%" }, "Ingresar");
  const form = h("form", { novalidate: true, class: "login-form" }, field("email", "Correo", email), field("password", "Contraseña", password), h("div", { class: "form-error" }), h("div", { class: "form-actions" }, submit));
  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    clearFieldErrors(form);
    if (!email.value.trim() || !password.value) return setFieldError(form, email.value.trim() ? "password" : "email", "Completá este dato.");
    try {
      const res = await busy(submit, () => api.signIn(email.value.trim(), password.value));
      if (!["OPERADOR", "ADMIN"].includes(res.user.role)) return showApiErrors(form, Object.assign(new Error("Esta página es para el jefe de planta o el administrador."), { fields: [] }));
      session.save(res);
      shell();
    } catch (err) {
      showApiErrors(form, err.status === 401 ? Object.assign(err, { message: "Correo o contraseña incorrectos." }) : err);
    }
  });
  app.replaceChildren(h("main", { class: "lab-login", id: "main" }, h("h1", {}, "Por dentro"),
    h("p", { class: "muted" }, "Material de estudio del curso. Ingresá con una cuenta de jefe de planta o de administrador."), form));
}

function shell() {
  const user = session.user();
  const panel = h("div", { class: "lab-panel", role: "tabpanel", "aria-label": "Contenido de la sección" });
  const first = TABS.some(([k]) => k === parts()[0]) ? parts()[0] : "lista";
  const bar = tabs(TABS.map(([k, l]) => [k, l]), first, (key) => {
    history.replaceState(null, "", "#/" + key);
    show(key);
  });
  app.replaceChildren(h("div", { class: "lab-shell" },
    h("header", { class: "lab-top" },
      h("span", { class: "brand" }, h("span", { class: "brand-mark" }, icon("tree", 18)), "HubFlow · Por dentro"),
      h("div", { class: "who" }, h("small", {}, user.name), h("button", { class: "btn small", onClick: () => { session.clear(); start(); } }, icon("logout", 16), "Salir"))),
    h("main", { class: "lab-main", id: "main", tabindex: "-1" },
      h("h1", {}, "Por dentro"),
      h("p", { class: "lab-sub" }, "Cómo están implementadas las estructuras y algoritmos de HubFlow."),
      h("aside", { class: "lab-intro" }, icon("layers", 20), h("p", {}, "Esta página es el material de estudio del curso. Reúne las estructuras de datos propias y los algoritmos que usa el sistema, con sus costos y su recorrido paso a paso. No forma parte de las pantallas de trabajo.")),
      bar, panel)));
  show(first);

  async function show(key) {
    const mine = ++token;
    if (typeof cleanup === "function") cleanup();
    cleanup = null;
    panel.replaceChildren();
    try {
      const mod = await TABS.find(([k]) => k === key)[2]();
      if (mine !== token) return;
      const id = key === "decide" ? parts()[1] : undefined;
      const result = await mod.default(panel, { id });
      if (mine !== token) return typeof result === "function" ? result() : undefined;
      cleanup = typeof result === "function" ? result : null;
    } catch (e) {
      panel.replaceChildren(errorBox(e));
    }
  }
}

start();
