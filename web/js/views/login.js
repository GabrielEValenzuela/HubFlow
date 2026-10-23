import { api } from "../api.js";
import * as session from "../session.js";
import { h, field, busy, showApiErrors, clearFieldErrors, setFieldError } from "../ui.js";
import { icon } from "../icons.js";
import { navigate, home } from "../router.js";

export default function login(root) {
  document.title = "Ingresar · HubFlow";
  const email = h("input", { class: "input", type: "email", autocomplete: "username", required: true, autofocus: true });
  const password = h("input", { class: "input", type: "password", autocomplete: "current-password", required: true });
  const submit = h("button", { class: "btn primary", type: "submit", style: "width:100%" }, "Ingresar");
  const form = h("form", { novalidate: true, class: "login-form" },
    field("email", "Correo", email), field("password", "Contraseña", password),
    h("div", { class: "form-error" }), h("div", { class: "form-actions" }, submit));

  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    clearFieldErrors(form);
    let bad = false;
    if (!/^\S+@\S+\.\S+$/.test(email.value.trim())) { setFieldError(form, "email", "Ingresá un correo válido."); bad = true; }
    if (!password.value) { setFieldError(form, "password", "Ingresá tu contraseña."); bad = true; }
    if (bad) return form.querySelector(".invalid .input").focus();
    try {
      const res = await busy(submit, () => api.signIn(email.value.trim(), password.value));
      session.save(res);
      navigate(home());
    } catch (err) {
      showApiErrors(form, err.status === 401 ? Object.assign(err, { message: "Correo o contraseña incorrectos." }) : err);
    }
  });

  root.append(h("div", { class: "login" },
    h("section", { class: "login-hero", "aria-hidden": "true" },
      h("div", { class: "brand" }, h("span", { class: "brand-mark" }, icon("route", 18)), "HubFlow"),
      h("p", { class: "login-claim" }, "Gestión de envíos desde el centro de distribución hasta la entrega."),
      loginMap()),
    h("section", { class: "login-panel" },
      h("h1", {}, "Ingresar a HubFlow"),
      h("p", { class: "muted" }, "Operadores, repartidores y administración usan la misma cuenta de HubFlow."),
      form)));
}

function loginMap() {
  const ns = "http://www.w3.org/2000/svg";
  const wrap = document.createElement("div");
  wrap.className = "login-map";
  wrap.innerHTML = `<svg viewBox="0 0 320 180" role="presentation"><g fill="none" stroke="currentColor" stroke-width="1.5" opacity=".35"><path d="M40 140 L110 90 L170 120 L240 50 L290 80"/><path d="M110 90 L120 30 L240 50"/><path d="M170 120 L200 160"/></g><path class="route-draw" d="M40 140 L110 90 L170 120 L240 50" fill="none" stroke="currentColor" stroke-width="3.5" stroke-linecap="round" stroke-linejoin="round"/><g fill="currentColor"><rect x="32" y="132" width="16" height="16" rx="3"/><circle cx="110" cy="90" r="5"/><circle cx="170" cy="120" r="5"/><circle cx="240" cy="50" r="7"/><circle cx="120" cy="30" r="4" opacity=".4"/><circle cx="290" cy="80" r="4" opacity=".4"/><circle cx="200" cy="160" r="4" opacity=".4"/></g></svg>`;
  return wrap;
}
