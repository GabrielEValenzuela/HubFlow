import { api } from "../api.js";
import { roadMap } from "../data.js";
import { h, pageHead, loading, empty, errorBox, field, setFieldError, clearFieldErrors, showApiErrors, busy, toast, statusBadge, zoneTag, serviceBadge, fmtKg, ZONE, FEATURE, zoneChoices, serviceChoices, featureChoices, serviceOrderSentence, isTrackingCode, CODE_ERROR, parcelsText, confirmDialog } from "../ui.js";
import { icon } from "../icons.js";
import { openShipment } from "./shipmentDetail.js";
import { showRegistered } from "../lib/labels.js";

export default async function plant(root) {
  const listSlot = h("div", {}, loading());
  const formSlot = h("div", {});
  const dispatchBtn = h("button", { class: "btn primary", onClick: dispatchNext }, icon("send"), "Despachar el próximo");
  const registerBtn = h("button", { class: "btn", "aria-expanded": "false", onClick: toggleForm }, icon("plus"), "Ingresar paquete");
  const linkBtn = h("button", { class: "btn", disabled: true, onClick: linkPicked }, icon("link"), "Que viajen juntos");

  const lookup = h("input", { class: "input", placeholder: "HF-7K3M9Q2X8", "aria-label": "Código del paquete", style: "width:170px" });
  const lookupForm = h("form", { class: "actions", onSubmit: (e) => {
    e.preventDefault();
    const c = lookup.value.trim().toUpperCase();
    if (!isTrackingCode(c)) return toast(CODE_ERROR, "error");
    openShipment(c, load);
  } }, lookup, h("button", { class: "btn", type: "submit" }, icon("search"), "Ver paquete"));

  root.append(pageHead("Paquetes en planta",
    `Paquetes que esperan salir, en el orden en que van a salir: primero ${serviceOrderSentence()}. Dentro de cada servicio, sale primero el que llegó antes.`,
    lookupForm, linkBtn, registerBtn, dispatchBtn), formSlot, listSlot);

  let groupOf = new Map();
  const picked = new Set();

  async function load() {
    try {
      const [data, groups] = await Promise.all([api.get("/pending-shipments", { limit: 200 }), api.get("/shipment-groups", { limit: 200 })]);
      groupOf = new Map(groups.items.flatMap((g) => g.codes.map((c) => [c, g])));
      for (const c of [...picked]) if (!data.items.some((s) => s.code === c)) picked.delete(c);
      listSlot.replaceChildren(table(data.items));
    } catch (e) {
      listSlot.replaceChildren(errorBox(e, load));
    }
  }

  function together(s) {
    const g = groupOf.get(s.code);
    return g ? h("div", {}, h("span", { class: "together", title: "Estos paquetes se cargan, se reparten y se reprograman juntos" }, icon("link"), "Viaja con " + g.codes.filter((c) => c !== s.code).join(", "))) : null;
  }

  function refreshLinkBtn() {
    linkBtn.disabled = picked.size < 2;
    linkBtn.textContent = "";
    linkBtn.append(icon("link"), picked.size >= 2 ? `Que viajen juntos (${picked.size})` : "Que viajen juntos");
  }

  async function linkPicked() {
    try {
      await busy(linkBtn, () => api.post("/shipment-groups", { codes: [...picked] }));
      toast(`${[...picked].join(", ")} ahora viajan juntos.`);
      picked.clear();
      load();
    } catch (e) {
      toast(e.message, "error");
    }
  }

  async function unlink(s) {
    const g = groupOf.get(s.code);
    if (!(await confirmDialog({ title: "Dejar de viajar juntos", text: `Los paquetes ${g.codes.join(", ")} se van a poder cargar y repartir por separado.`, confirmLabel: "Separar paquetes" }))) return;
    try {
      await api.del("/shipment-groups/" + g.id);
      toast("Los paquetes se pueden cargar por separado.");
      load();
    } catch (e) {
      toast(e.message, "error");
    }
  }

  function table(items) {
    if (!items.length) return empty("No hay paquetes en la planta", "Los paquetes que ingresen aparecen acá hasta que se despachan.", h("button", { class: "btn primary", onClick: toggleForm }, "Ingresar paquete"));
    return h("div", {}, h("div", { class: "table-wrap" }, h("table", {},
      h("thead", {}, h("tr", {}, h("th", {}, h("span", { class: "sr-only" }, "Elegir")), ...["Sale en", "Código", "Destinatario", "Zona", "Servicio", "Bultos", "Peso a facturar", "Estado", ""].map((t) => h("th", { class: t === "Peso a facturar" ? "num" : "" }, t)))),
      h("tbody", {}, ...items.map((s, i) => h("tr", {},
        h("td", {}, h("input", { class: "row-pick", type: "checkbox", "aria-label": `Elegir ${s.code} para unirlo con otros`, checked: picked.has(s.code), onChange: (e) => { e.target.checked ? picked.add(s.code) : picked.delete(s.code); refreshLinkBtn(); } })),
        h("td", { class: "code" }, String(i + 1)), h("td", { class: "code" }, s.code), h("td", {}, s.recipient, features(s), together(s)), h("td", {}, zoneTag(s.zone)),
        h("td", {}, serviceBadge(s.service)), h("td", {}, String(s.parcelCount)), h("td", { class: "num" }, fmtKg(s.weightKg)), h("td", {}, statusBadge(s.status)),
        h("td", {}, h("div", { class: "actions" }, h("button", { class: "btn small", onClick: () => openShipment(s.code, load) }, "Ver paquete"),
          groupOf.has(s.code) ? h("button", { class: "btn small", onClick: () => unlink(s) }, "Separar") : null))))))),
      h("p", { class: "muted small", style: "margin-top:8px" }, `${items.length} ${items.length === 1 ? "paquete" : "paquetes"} en espera. Para que varios paquetes del mismo pedido viajen juntos, tildalos y tocá “Que viajen juntos”: tienen que entregarse en el mismo lugar.`));
  }

  const features = (s) => (s.features?.length ? h("div", { class: "muted small" }, s.features.map((f) => FEATURE[f] ?? f).join(" · ")) : null);

  async function dispatchNext() {
    try {
      const s = await busy(dispatchBtn, () => api.post("/dispatches", {}));
      toast(`Salió el paquete ${s.code} hacia ${s.recipient} (${ZONE[s.zone]}).`);
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
    const recipient = h("input", { class: "input", maxlength: "80", autocomplete: "off" });
    const zone = h("select", { class: "input" }, h("option", { value: "" }, "Elegí una zona"), ...zoneChoices().map(([v, l]) => h("option", { value: v }, l)));
    const service = h("select", { class: "input" }, h("option", { value: "" }, "Elegí un servicio"), ...serviceChoices().map(([v, l]) => h("option", { value: v }, l)));
    const point = h("select", { class: "input", disabled: true }, h("option", { value: "" }, "Primero elegí la zona"));
    let nodes = [];
    roadMap().then((m) => (nodes = m.nodes.filter((n) => !n.isHub && n.active)));
    zone.addEventListener("change", () => {
      const opts = nodes.filter((n) => n.zone === zone.value);
      point.disabled = !opts.length;
      point.replaceChildren(h("option", { value: "" }, opts.length ? "Elegí el lugar de entrega" : "Primero elegí la zona"),
        ...opts.map((n) => h("option", { value: n.id }, n.label)));
    });
    const featureBoxes = featureChoices().map(([v, l]) => ({ v, box: h("input", { type: "checkbox", value: v }), l }));
    const featureRow = featureBoxes.length
      ? h("div", { class: "checks" }, ...featureBoxes.map(({ box, l }) => h("label", { class: "check" }, box, l)))
      : h("p", { class: "muted small" }, "No hay características cargadas.");

    const rows = [];
    const rowsBox = h("div", { class: "parcel-rows" });
    const total = h("p", { class: "parcel-total", "aria-live": "polite" });
    const addParcel = h("button", { class: "btn", type: "button", onClick: () => { rows.push(newRow()); drawRows(); rows.at(-1).weight.focus(); } }, icon("plus"), "Agregar otro bulto");
    const num = (el) => (el.value === "" ? NaN : Number(el.value.replace(",", ".")));
    function newRow() {
      const box = (label, hint, max) => h("input", { class: "input", type: "number", step: "0.1", min: "0.1", max, inputmode: "decimal", "aria-label": label, placeholder: hint, onInput: recompute });
      return { weight: box("Peso en kilos", "kg", "50"), length: box("Largo en centímetros", "cm", "300"), width: box("Ancho en centímetros", "cm", "300"), height: box("Alto en centímetros", "cm", "300") };
    }
    function recompute() {
      let real = 0, billable = 0, complete = 0;
      for (const r of rows) {
        const w = num(r.weight), l = num(r.length), a = num(r.width), alt = num(r.height);
        if ([w, l, a, alt].every((n) => n > 0)) {
          real += w;
          billable += Math.max(w, (l * a * alt) / 5000);
          complete++;
        }
      }
      total.textContent = complete
        ? `${parcelsText(rows.length)} · pesan ${fmtKg(real)} y se facturan ${fmtKg(billable)} (se cobra lo que más pese entre el peso y el tamaño de cada bulto).`
        : "Cargá el peso y las medidas de cada bulto.";
    }
    function drawRows() {
      rowsBox.replaceChildren(...rows.map((r, i) => h("div", { class: "parcel-row", dataset: { parcel: String(i) } },
        h("div", { class: "n" }, `Bulto ${i + 1}`),
        field(`parcels-${i}-weight`, "Peso (kg)", r.weight), field(`parcels-${i}-length`, "Largo (cm)", r.length),
        field(`parcels-${i}-width`, "Ancho (cm)", r.width), field(`parcels-${i}-height`, "Alto (cm)", r.height),
        rows.length > 1 ? h("button", { class: "btn small", type: "button", "aria-label": `Quitar el bulto ${i + 1}`, onClick: () => { rows.splice(i, 1); drawRows(); recompute(); } }, "Quitar") : h("span"))));
      addParcel.disabled = rows.length >= 20;
      recompute();
    }
    rows.push(newRow());
    drawRows();

    const submit = h("button", { class: "btn primary", type: "submit" }, "Ingresar paquete");
    const form = h("form", { class: "card", novalidate: true, style: "margin-bottom:16px" },
      h("div", { class: "card-head" }, h("h2", {}, "Paquete nuevo"), h("p", {}, "Completá los datos. El sistema le asigna el código de seguimiento.")),
      h("div", { class: "form-grid" },
        field("recipient", "Destinatario", recipient), field("zone", "Zona", zone),
        field("service", "Servicio", service), field("deliveryPoint", "Lugar de entrega", point)),
      h("fieldset", { class: "crit", style: "margin-top:12px" }, h("legend", {}, "Bultos (cajas) del paquete"),
        h("p", { class: "muted small" }, "Un paquete puede tener varias cajas. Cargá el peso y las medidas de cada una."),
        rowsBox, h("div", { class: "actions", style: "margin-top:8px" }, addParcel), total, h("div", { class: "error", id: "parcels-error", role: "alert" })),
      h("fieldset", { class: "crit", style: "margin-top:12px" }, h("legend", {}, "Cuidados del paquete (opcional)"), featureRow),
      h("div", { class: "form-error" }), h("div", { class: "form-actions" }, submit, h("button", { class: "btn ghost", type: "button", onClick: toggleForm }, "Cancelar")));

    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      clearFieldErrors(form);
      form.querySelector("#parcels-error").textContent = "";
      const errs = {};
      const r = recipient.value.trim();
      if (r.length < 2 || r.length > 80) errs.recipient = "Entre 2 y 80 caracteres.";
      if (!zone.value) errs.zone = "Elegí una zona.";
      if (!service.value) errs.service = "Elegí un servicio.";
      if (!point.value) errs.deliveryPoint = "Elegí un lugar de entrega.";
      rows.forEach((row, i) => {
        const w = num(row.weight);
        if (!(w >= 0.01 && w <= 50)) errs[`parcels-${i}-weight`] = "Entre 0,01 y 50 kg.";
        for (const [key, el] of [["length", row.length], ["width", row.width], ["height", row.height]]) {
          const v = num(el);
          if (!(v >= 0.1 && v <= 300)) errs[`parcels-${i}-${key}`] = "Entre 0,1 y 300 cm.";
        }
      });
      const keys = Object.keys(errs);
      if (keys.length) {
        keys.forEach((k) => setFieldError(form, k, errs[k]));
        return form.querySelector(".field.invalid .input").focus();
      }
      const parcels = rows.map((row) => ({ weightKg: num(row.weight), lengthCm: num(row.length), widthCm: num(row.width), heightCm: num(row.height) }));
      try {
        const s = await busy(submit, () => api.post("/shipments", { recipient: r, zone: zone.value, service: service.value, deliveryPoint: point.value, features: featureBoxes.filter((f) => f.box.checked).map((f) => f.v), parcels }));
        toggleForm();
        load();
        showRegistered(s);
      } catch (err) {
        for (const f of err.fields ?? []) {
          const m = /^parcels\[(\d+)\]\.(\w+)$/.exec(f.field ?? "");
          if (m) f.field = `parcels-${m[1]}-${m[2].replace("Cm", "").replace("Kg", "")}`;
        }
        showApiErrors(form, err);
      }
    });
    return form;
  }

  load();
}
