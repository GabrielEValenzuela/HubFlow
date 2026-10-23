import { api } from "../api.js";
import * as session from "../session.js";
import { h, pageHead, loading, errorBox, field, busy, setFieldError, clearFieldErrors, showApiErrors, toast, ROLE, confirmDialog } from "../ui.js";
import { icon } from "../icons.js";

export default async function staff(root) {
  const slot = h("div", {}, loading());
  const formSlot = h("div", {});
  const createBtn = h("button", { class: "btn primary", "aria-expanded": "false", onClick: toggle }, icon("plus"), "Agregar persona");
  root.append(pageHead("Personal", "Las personas que usan HubFlow y qué hace cada una. Quien está sin acceso no puede entrar, pero su historial se conserva.", createBtn), formSlot, slot);
  const me = session.user();

  async function load() {
    try {
      const data = await api.get("/users", { limit: 100 });
      slot.replaceChildren(h("div", { class: "table-wrap" }, h("table", {},
        h("thead", {}, h("tr", {}, ...["Nombre", "Correo", "Puesto", "Acceso"].map((t) => h("th", {}, t)))),
        h("tbody", {}, ...data.items.map(row)))));
    } catch (e) {
      slot.replaceChildren(errorBox(e, load));
    }
  }

  function row(u) {
    const self = u.id === me.id;
    const role = h("select", { class: "input", "aria-label": `Puesto de ${u.name}`, style: "min-width:170px", disabled: self, onChange: async (e) => {
      try {
        await api.patch("/users/" + u.id, { role: e.target.value });
        toast(`${u.name} ahora es ${ROLE[e.target.value]}.`);
      } catch (err) {
        toast(err.message, "error");
        e.target.value = u.role;
      }
    } }, ...Object.entries(ROLE).map(([v, l]) => h("option", { value: v, selected: v === u.role }, l)));
    const toggleBtn = h("button", { class: "btn small" + (u.active ? "" : " primary"), disabled: self, title: self ? "No podés quitarte el acceso a vos mismo" : null, onClick: async (e) => {
      if (u.active && !(await confirmDialog({ title: "Quitar acceso", text: `${u.name} no va a poder entrar a HubFlow hasta que le devuelvas el acceso.`, confirmLabel: "Quitar acceso", danger: true }))) return;
      try {
        await busy(e.currentTarget, () => api.patch("/users/" + u.id, { active: !u.active }));
        toast(u.active ? `${u.name} ya no tiene acceso.` : `${u.name} volvió a tener acceso.`);
        load();
      } catch (err) {
        toast(err.message, "error");
      }
    } }, u.active ? "Quitar acceso" : "Dar acceso");
    return h("tr", {}, h("td", {}, h("strong", {}, u.name), self ? h("span", { class: "muted small" }, " (vos)") : null), h("td", {}, u.email), h("td", {}, role),
      h("td", {}, h("span", { class: "badge " + (u.active ? "s-ENTREGADO" : "s-PENDIENTE") }, u.active ? "Con acceso" : "Sin acceso"), " ", toggleBtn));
  }

  let open = false;
  function toggle() {
    open = !open;
    createBtn.setAttribute("aria-expanded", String(open));
    if (!open) return formSlot.replaceChildren();
    const name = h("input", { class: "input", autocomplete: "off" });
    const email = h("input", { class: "input", type: "email", autocomplete: "off" });
    const pass = h("input", { class: "input", type: "password", autocomplete: "new-password" });
    const role = h("select", { class: "input" }, ...Object.entries(ROLE).map(([v, l]) => h("option", { value: v, selected: v === "PUM" }, l)));
    const submit = h("button", { class: "btn primary", type: "submit" }, "Guardar persona");
    const form = h("form", { class: "card", novalidate: true, style: "margin-bottom:16px" }, h("div", { class: "card-head" }, h("h2", {}, "Agregar persona")),
      h("div", { class: "form-grid" }, field("name", "Nombre", name), field("email", "Correo", email), field("role", "Puesto", role), field("password", "Clave inicial", pass, { hint: "Entre 8 y 72 letras o números" })),
      h("div", { class: "form-error" }), h("div", { class: "form-actions" }, submit, h("button", { class: "btn ghost", type: "button", onClick: toggle }, "Cancelar")));
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      clearFieldErrors(form);
      const errs = {};
      if (name.value.trim().length < 2) errs.name = "Entre 2 y 80 caracteres.";
      if (!/^\S+@\S+\.\S+$/.test(email.value.trim())) errs.email = "Ingresá un correo válido.";
      if (pass.value.length < 8 || pass.value.length > 72) errs.password = "La clave debe tener entre 8 y 72 caracteres.";
      const ks = Object.keys(errs);
      if (ks.length) { ks.forEach((k) => setFieldError(form, k, errs[k])); return form.querySelector(".invalid .input").focus(); }
      try {
        await busy(submit, () => api.post("/users", { name: name.value.trim(), email: email.value.trim(), role: role.value, password: pass.value }));
        toast("Persona agregada.");
        toggle();
        load();
      } catch (err) {
        showApiErrors(form, err);
      }
    });
    formSlot.replaceChildren(form);
    name.focus();
  }
  load();
}
