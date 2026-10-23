import { api } from "../api.js";
import { roadMap } from "../data.js";
import { h, pageHead, loading, empty, errorBox, field, setFieldError, clearFieldErrors, showApiErrors, busy, toast, statusBadge, zoneTag, serviceBadge, fmtKg, SERVICE, ZONE } from "../ui.js";
import { icon } from "../icons.js";
import { openShipment } from "./shipmentDetail.js";

export default async function pending(root) {
  const listSlot = h("div", {}, loading());
  const formSlot = h("div", {});
  const dispatchBtn = h("button", { class: "btn primary", onClick: dispatchNext }, icon("send"), "Despachar próximo");
  const registerBtn = h("button", { class: "btn", "aria-expanded": "false", onClick: toggleForm }, icon("plus"), "Registrar envío");
  const lookup = h("input", { class: "input", placeholder: "PKG-1001", "aria-label": "Código a buscar", style: "width:150px" });
  const lookupForm = h("form", { class: "actions", onSubmit: (e) => {
    e.preventDefault();
    const c = lookup.value.trim().toUpperCase();
    if (!/^PKG-\d{4}$/.test(c)) return toast("El código tiene el formato PKG-0000.", "error");
    openShipment(c, load);
  } }, lookup, h("button", { class: "btn", type: "submit" }, icon("search"), "Ver envío"));

  root.append(pageHead("Bandeja de pendientes",
    "Envíos pendientes de salir, en orden de despacho: Express, luego Prioritario, luego Estándar; dentro de un mismo servicio, por orden de registro.",
    lookupForm, registerBtn, dispatchBtn), formSlot, listSlot);

  async function load() {
    try {
      const data = await api.get("/pending-shipments", { limit: 200 });
      listSlot.replaceChildren(table(data.items));
    } catch (e) {
      listSlot.replaceChildren(errorBox(e, load));
    }
  }

  function table(items) {
    if (!items.length) return empty("No hay envíos pendientes", "Los envíos registrados aparecen acá hasta que se despachan.", h("button", { class: "btn primary", onClick: toggleForm }, "Registrar envío"));
    return h("div", {}, h("div", { class: "table-wrap" }, h("table", {},
      h("thead", {}, h("tr", {}, ...["Orden", "Código", "Destinatario", "Zona", "Servicio", "Peso", "Estado", ""].map((t) => h("th", { class: t === "Peso" ? "num" : "" }, t)))),
      h("tbody", {}, ...items.map((s, i) => h("tr", {},
        h("td", { class: "code" }, String(i + 1)), h("td", { class: "code" }, s.code), h("td", {}, s.recipient), h("td", {}, zoneTag(s.zone)),
        h("td", {}, serviceBadge(s.service)), h("td", { class: "num" }, fmtKg(s.weightKg)), h("td", {}, statusBadge(s.status)),
        h("td", {}, h("button", { class: "btn small", onClick: () => openShipment(s.code, load) }, "Ver detalle"))))))),
      h("p", { class: "muted small", style: "margin-top:8px" }, `${items.length} envíos en espera.`));
  }

  async function dispatchNext() {
    try {
      const s = await busy(dispatchBtn, () => api.post("/dispatches", {}));
      toast(`Salió ${s.code} hacia ${s.recipient} (${ZONE[s.zone]}).`);
      load();
    } catch (e) {
      toast(e.message, "error");
    }
  }

  let formOpen = false;
  function toggleForm() {
    formOpen = !formOpen;
    registerBtn.setAttribute("aria-expanded", String(formOpen));
    if (!formOpen) return formSlot.replaceChildren();
    formSlot.replaceChildren(buildForm());
    formSlot.querySelector("input")?.focus();
  }

  function buildForm() {
    const code = h("input", { class: "input", placeholder: "PKG-1009", maxlength: "8", autocapitalize: "characters", autocomplete: "off" });
    const recipient = h("input", { class: "input", maxlength: "80", autocomplete: "off" });
    const weight = h("input", { class: "input", type: "number", step: "0.01", min: "0.01", max: "50", inputmode: "decimal" });
    const zone = h("select", { class: "input" }, h("option", { value: "" }, "Elegí una zona"), ...Object.entries(ZONE).map(([v, l]) => h("option", { value: v }, l)));
    const service = h("select", { class: "input" }, h("option", { value: "" }, "Elegí un servicio"), ...Object.entries(SERVICE).map(([v, l]) => h("option", { value: v }, l)));
    const point = h("select", { class: "input", disabled: true }, h("option", { value: "" }, "Primero elegí la zona"));
    let nodes = [];
    roadMap().then((m) => (nodes = m.nodes.filter((n) => !n.isHub)));
    zone.addEventListener("change", () => {
      const opts = nodes.filter((n) => n.zone === zone.value);
      point.disabled = !opts.length;
      point.replaceChildren(h("option", { value: "" }, opts.length ? "Elegí el punto de entrega" : "Primero elegí la zona"),
        ...opts.map((n) => h("option", { value: n.id }, `${n.id} · ${n.label}`)));
    });
    const submit = h("button", { class: "btn primary", type: "submit" }, "Registrar envío");
    const form = h("form", { class: "card", novalidate: true, style: "margin-bottom:16px" },
      h("div", { class: "card-head" }, h("h2", {}, "Nuevo envío"), h("p", {}, "Los datos se validan también en el servidor.")),
      h("div", { class: "form-grid" },
        field("code", "Código", code, { hint: "PKG- y cuatro dígitos" }), field("recipient", "Destinatario", recipient),
        field("weightKg", "Peso (kg)", weight, { hint: "Entre 0,01 y 50" }), field("zone", "Zona", zone),
        field("service", "Servicio", service), field("deliveryPoint", "Punto de entrega", point)),
      h("div", { class: "form-error" }), h("div", { class: "form-actions" }, submit, h("button", { class: "btn ghost", type: "button", onClick: toggleForm }, "Cancelar")));

    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      clearFieldErrors(form);
      const errs = {};
      const c = code.value.trim().toUpperCase();
      if (!/^PKG-\d{4}$/.test(c)) errs.code = "Usá el formato PKG-0000.";
      const r = recipient.value.trim();
      if (r.length < 2 || r.length > 80) errs.recipient = "Entre 2 y 80 caracteres.";
      const w = Number(weight.value);
      if (weight.value === "" || !(w >= 0.01 && w <= 50)) errs.weightKg = "Ingresá un peso entre 0,01 y 50 kg.";
      if (!zone.value) errs.zone = "Elegí una zona.";
      if (!service.value) errs.service = "Elegí un servicio.";
      if (!point.value) errs.deliveryPoint = "Elegí un punto de entrega.";
      const keys = Object.keys(errs);
      if (keys.length) {
        keys.forEach((k) => setFieldError(form, k, errs[k]));
        return form.querySelector(".field.invalid .input").focus();
      }
      try {
        const s = await busy(submit, () => api.post("/shipments", { code: c, recipient: r, zone: zone.value, weightKg: w, service: service.value, deliveryPoint: point.value }));
        toast(`Registrado ${s.code}.`);
        toggleForm();
        load();
      } catch (err) {
        showApiErrors(form, err);
      }
    });
    return form;
  }

  load();
}
