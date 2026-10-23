import { api } from "../api.js";
import { roadMap, loadCatalogs, resetCaches } from "../data.js";
import { h, pageHead, loading, errorBox, empty, field, busy, toast, dialog, tabs, subHead, clearFieldErrors, setFieldError, showApiErrors, confirmDialog, CATALOG, zoneChoices, zoneTag } from "../ui.js";
import { icon } from "../icons.js";

const DIRECTIONS = { N: ["Norte", 0, -1], S: ["Sur", 0, 1], E: ["Este", 1, 0], O: ["Oeste", -1, 0] };

/** Convierte un nombre en un código corto y único: "Zona Oeste" pasa a ZONA_OESTE. */
function codeFor(name, taken) {
  const base = (name.normalize("NFD").replace(/[̀-ͯ]/g, "").toUpperCase().replace(/[^A-Z0-9]+/g, "_").replace(/^_+|_+$/g, "").slice(0, 14) || "NUEVO").padEnd(2, "X");
  let code = base;
  for (let n = 2; taken.has(code); n++) code = base.slice(0, 16 - String(n).length) + n;
  return code;
}

function askText({ title, label, value = "", confirmLabel }) {
  return new Promise((resolve) => {
    let result = null;
    const input = h("input", { class: "input", value, maxlength: "60", autocomplete: "off" });
    const form = h("form", { novalidate: true }, field("name", label, input), h("div", { class: "form-actions" },
      h("button", { class: "btn primary", type: "submit" }, confirmLabel), h("button", { class: "btn", type: "button", onClick: () => dlg.close() }, "Cancelar")));
    form.addEventListener("submit", (e) => {
      e.preventDefault();
      const v = input.value.trim();
      if (v.length < 2) return setFieldError(form, "name", "Escribí al menos 2 letras.");
      result = v;
      dlg.close();
    });
    const dlg = dialog({ title, body: form, onClose: () => resolve(result) });
    input.focus();
    input.select();
  });
}

/** Pantalla del administrador: zonas, servicios y características con los que trabaja la planta. */
export default async function settings(root) {
  const slot = h("div", {}, loading());
  let active = "zonas";
  let map = null;
  const bar = tabs([["zonas", "Zonas"], ["servicios", "Servicios"], ["cuidados", "Cuidados del paquete"]], active, (id) => { active = id; draw(); });
  root.append(pageHead("Configuración", "Con qué zonas, servicios y cuidados trabaja la planta. Los cambios se aplican enseguida; nada se borra, solo se desactiva."), bar, slot);

  async function reload() {
    resetCaches();
    try {
      await loadCatalogs();
      map = await roadMap();
      draw();
    } catch (e) {
      slot.replaceChildren(errorBox(e, reload));
    }
  }

  const draw = () => slot.replaceChildren(active === "zonas" ? zones() : active === "servicios" ? services() : features());

  async function change(btn, fn, okMessage) {
    try {
      await busy(btn, fn);
      toast(okMessage);
      await reload();
    } catch (e) {
      toast(e.message, "error");
    }
  }

  const stateBadge = (on, onText = "Activa", offText = "Desactivada") => h("span", { class: "badge " + (on ? "s-ENTREGADO" : "s-PENDIENTE") }, on ? onText : offText);

  function addForm({ title, hint, fields, submitLabel, onSubmit }) {
    const submit = h("button", { class: "btn primary", type: "submit" }, icon("plus"), submitLabel);
    const form = h("form", { class: "card", novalidate: true, style: "margin-top:var(--s-4)" },
      h("div", { class: "card-head" }, h("div", {}, h("h3", {}, title), hint ? h("p", {}, hint) : null)),
      h("div", { class: "form-grid" }, ...fields), h("div", { class: "form-error" }), h("div", { class: "form-actions" }, submit));
    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      clearFieldErrors(form);
      try {
        const done = await busy(submit, () => onSubmit(form));
        if (done) await reload();
      } catch (err) {
        showApiErrors(form, err);
      }
    });
    return form;
  }

  // ---- Zonas
  function zones() {
    const nodes = (map?.nodes ?? []).filter((n) => !n.isHub);
    const cards = CATALOG.zones.map((z) => {
      const mine = nodes.filter((n) => n.zone === z.code);
      return h("article", { class: "card", style: "margin-bottom:var(--s-3)" },
        h("div", { class: "card-head" }, h("div", {}, h("h3", {}, zoneTag(z.code)), h("p", {}, `${mine.length} ${mine.length === 1 ? "lugar de entrega" : "lugares de entrega"}`)),
          h("div", { class: "actions" }, stateBadge(z.active),
            h("button", { class: "btn small", onClick: async (e) => {
              const name = await askText({ title: "Renombrar zona", label: "Nombre de la zona", value: z.name, confirmLabel: "Guardar nombre" });
              if (name) change(e.currentTarget, () => api.patch("/zones/" + z.code, { name }), "Nombre guardado.");
            } }, "Cambiar nombre"),
            h("button", { class: "btn small" + (z.active ? "" : " primary"), onClick: async (e) => {
              const btn = e.currentTarget;
              if (z.active && !(await confirmDialog({ title: "Desactivar zona", text: `${z.name} deja de aceptar paquetes nuevos. Para poder hacerlo no debe haber paquetes en la planta, repartos en curso ni lugares de entrega activos en esa zona.`, confirmLabel: "Desactivar zona", danger: true }))) return;
              change(btn, () => api.patch("/zones/" + z.code, { active: !z.active }), z.active ? `${z.name} quedó desactivada.` : `${z.name} volvió a estar activa.`);
            } }, z.active ? "Desactivar" : "Activar"))),
        mine.length ? h("ul", { class: "places" }, ...mine.map(place)) : h("p", { class: "muted small" }, "Esta zona todavía no tiene lugares de entrega."));
    });
    return h("div", {}, subHead("Zonas de reparto", "Cada zona agrupa lugares de entrega. Los paquetes y las cargas se arman por zona."),
      cards.length ? h("div", {}, ...cards) : empty("No hay zonas", "Agregá la primera zona para empezar."), newZoneForm(), newPlaceForm(nodes));
  }

  function place(n) {
    return h("li", {}, h("span", {}, h("strong", {}, n.label), " ", h("span", { class: "muted small code" }, n.id), n.active ? null : h("span", { class: "badge s-PENDIENTE", style: "margin-left:8px" }, "Fuera de servicio")),
      h("span", { class: "actions" },
        h("button", { class: "btn small", onClick: (e) => movePlace(e.currentTarget, n) }, "Pasar a otra zona"),
        h("button", { class: "btn small", onClick: (e) => change(e.currentTarget, () => api.patch("/road-nodes/" + n.id, { active: !n.active }), n.active ? `${n.label} quedó fuera de servicio.` : `${n.label} volvió a estar en servicio.`) }, n.active ? "Sacar de servicio" : "Volver a habilitar")));
  }

  function movePlace(btn, n) {
    const select = h("select", { class: "input" }, ...zoneChoices().filter(([c]) => c !== n.zone).map(([c, l]) => h("option", { value: c }, l)));
    const form = h("form", { novalidate: true }, field("zone", `Nueva zona de ${n.label}`, select), h("div", { class: "form-actions" },
      h("button", { class: "btn primary", type: "submit" }, "Pasar de zona"), h("button", { class: "btn", type: "button", onClick: () => dlg.close() }, "Cancelar")));
    form.addEventListener("submit", (e) => {
      e.preventDefault();
      if (!select.value) return setFieldError(form, "zone", "Elegí una zona.");
      dlg.close();
      change(btn, () => api.patch("/road-nodes/" + n.id, { zone: select.value }), `${n.label} ahora está en otra zona.`);
    });
    const dlg = dialog({ title: "Pasar a otra zona", body: form });
  }

  function newZoneForm() {
    const name = h("input", { class: "input", maxlength: "60", autocomplete: "off" });
    return addForm({
      title: "Agregar zona", hint: "Después de crearla, agregale al menos un lugar de entrega.", submitLabel: "Agregar zona",
      fields: [field("name", "Nombre de la zona", name)],
      onSubmit: async (form) => {
        const v = name.value.trim();
        if (v.length < 2) { setFieldError(form, "name", "Escribí al menos 2 letras."); return false; }
        await api.post("/zones", { code: codeFor(v, new Set(CATALOG.zones.map((z) => z.code))), name: v });
        toast(`Zona ${v} agregada.`);
        return true;
      },
    });
  }

  function newPlaceForm(nodes) {
    const label = h("input", { class: "input", maxlength: "80", autocomplete: "off" });
    const zone = h("select", { class: "input" }, ...zoneChoices().map(([c, l]) => h("option", { value: c }, l)));
    const km = h("input", { class: "input", type: "number", min: "0.1", max: "100", step: "0.1", inputmode: "decimal" });
    const dir = h("select", { class: "input" }, ...Object.entries(DIRECTIONS).map(([k, [l]]) => h("option", { value: k }, l)));
    const near = h("select", { class: "input" }, ...(map?.nodes ?? []).map((n) => h("option", { value: n.id }, n.label)));
    return addForm({
      title: "Agregar lugar de entrega", hint: "Indicá a qué distancia y en qué dirección queda respecto de un lugar que ya existe.", submitLabel: "Agregar lugar",
      fields: [field("label", "Nombre del lugar", label), field("zone", "Zona", zone), field("km", "Distancia en kilómetros", km), field("dir", "Queda hacia el", dir), field("near", "Respecto de", near)],
      onSubmit: async (form) => {
        const errs = {};
        if (label.value.trim().length < 2) errs.label = "Escribí el nombre del lugar.";
        if (!zone.value) errs.zone = "Elegí una zona. Si no hay ninguna, agregala primero.";
        const dist = Number(km.value);
        if (!(dist >= 0.1 && dist <= 100)) errs.km = "Escribí una distancia entre 0,1 y 100 km.";
        if (Object.keys(errs).length) { Object.entries(errs).forEach(([k, m]) => setFieldError(form, k, m)); return false; }
        const base = map.nodes.find((n) => n.id === near.value);
        const [, dx, dy] = DIRECTIONS[dir.value];
        const taken = new Set(map.nodes.map((n) => n.id));
        const prefix = zone.value.slice(0, 3);
        let n = 1;
        while (taken.has(prefix + n)) n++;
        await api.post("/road-nodes", { id: prefix + n, label: label.value.trim(), zone: zone.value, x: Math.round((base.x + dx * dist) * 100) / 100, y: Math.round((base.y + dy * dist) * 100) / 100, links: [{ to: base.id, costKm: dist }] });
        toast(`${label.value.trim()} quedó agregado.`);
        return true;
      },
    });
  }

  // ---- Servicios
  function services() {
    const top = CATALOG.services[0]?.priority ?? 0;
    const rows = CATALOG.services.map((sv, i) => h("tr", {},
      h("td", { class: "code" }, String(i + 1)), h("td", {}, h("strong", {}, sv.name), " ", h("span", { class: "muted small code" }, sv.code)), h("td", { class: "code" }, String(sv.priority)), h("td", {}, stateBadge(sv.active, "Activo", "Desactivado")),
      h("td", {}, h("span", { class: "actions" },
        h("button", { class: "btn small", onClick: async (e) => {
          const name = await askText({ title: "Renombrar servicio", label: "Nombre del servicio", value: sv.name, confirmLabel: "Guardar nombre" });
          if (name) change(e.currentTarget, () => api.patch("/service-levels/" + sv.code, { name }), "Nombre guardado.");
        } }, "Cambiar nombre"),
        h("button", { class: "btn small" + (sv.active ? "" : " primary"), onClick: (e) => change(e.currentTarget, () => api.patch("/service-levels/" + sv.code, { active: !sv.active }), sv.active ? `${sv.name} quedó desactivado.` : `${sv.name} volvió a estar activo.`) }, sv.active ? "Desactivar" : "Activar")))));
    const name = h("input", { class: "input", maxlength: "60", autocomplete: "off" });
    const priority = h("input", { class: "input", type: "number", min: "1", max: "1000", step: "1", value: String(top + 1) });
    return h("div", {}, subHead("Servicios", "El servicio con el número de prioridad más alto sale primero de la planta. A igual prioridad, sale primero el paquete que llegó antes. La prioridad no se puede cambiar después de crear el servicio."),
      CATALOG.services.length ? h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, h("tr", {}, ...["Sale en", "Servicio", "Prioridad", "Estado", ""].map((t) => h("th", {}, t)))), h("tbody", {}, ...rows))) : empty("No hay servicios", "Agregá el primer servicio."),
      addForm({
        title: "Agregar servicio", hint: `Hoy el que sale primero es ${CATALOG.services[0]?.name ?? "—"}, con prioridad ${top}. Poné un número mayor para que el nuevo salga antes.`, submitLabel: "Agregar servicio",
        fields: [field("name", "Nombre del servicio", name), field("priority", "Prioridad", priority, { hint: "Un número entero entre 1 y 1000" })],
        onSubmit: async (form) => {
          const v = name.value.trim(), p = Number(priority.value);
          const errs = {};
          if (v.length < 2) errs.name = "Escribí al menos 2 letras.";
          if (!Number.isInteger(p) || p < 1 || p > 1000) errs.priority = "Escribí un número entero entre 1 y 1000.";
          if (Object.keys(errs).length) { Object.entries(errs).forEach(([k, m]) => setFieldError(form, k, m)); return false; }
          await api.post("/service-levels", { code: codeFor(v, new Set(CATALOG.services.map((s) => s.code))), name: v, priority: p });
          toast(`Servicio ${v} agregado.`);
          return true;
        },
      }));
  }

  // ---- Cuidados del paquete (características)
  function features() {
    const rows = CATALOG.features.map((f) => h("tr", {},
      h("td", {}, h("strong", {}, f.name), " ", h("span", { class: "muted small code" }, f.code)), h("td", {}, stateBadge(f.active, "Activo", "Desactivado")),
      h("td", {}, h("span", { class: "actions" },
        h("button", { class: "btn small", onClick: async (e) => {
          const name = await askText({ title: "Renombrar cuidado", label: "Nombre", value: f.name, confirmLabel: "Guardar nombre" });
          if (name) change(e.currentTarget, () => api.patch("/shipment-features/" + f.code, { name }), "Nombre guardado.");
        } }, "Cambiar nombre"),
        h("button", { class: "btn small" + (f.active ? "" : " primary"), onClick: (e) => change(e.currentTarget, () => api.patch("/shipment-features/" + f.code, { active: !f.active }), f.active ? `${f.name} quedó desactivado.` : `${f.name} volvió a estar activo.`) }, f.active ? "Desactivar" : "Activar")))));
    const name = h("input", { class: "input", maxlength: "60", autocomplete: "off" });
    return h("div", {}, subHead("Cuidados del paquete", "Etiquetas para marcar los paquetes que necesitan un trato especial, por ejemplo frágil o refrigerado. Desactivar una etiqueta solo impide usarla en paquetes nuevos."),
      CATALOG.features.length ? h("div", { class: "table-wrap" }, h("table", {}, h("thead", {}, h("tr", {}, ...["Cuidado", "Estado", ""].map((t) => h("th", {}, t)))), h("tbody", {}, ...rows))) : empty("No hay cuidados cargados", "Agregá el primero."),
      addForm({
        title: "Agregar cuidado", submitLabel: "Agregar cuidado", fields: [field("name", "Nombre", name)],
        onSubmit: async (form) => {
          const v = name.value.trim();
          if (v.length < 2) { setFieldError(form, "name", "Escribí al menos 2 letras."); return false; }
          await api.post("/shipment-features", { code: codeFor(v, new Set(CATALOG.features.map((f) => f.code))), name: v });
          toast(`${v} agregado.`);
          return true;
        },
      }));
  }

  reload();
}
