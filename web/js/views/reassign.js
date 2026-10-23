import { api } from "../api.js";
import { h, dialog, field, busy, toast, setFieldError, showApiErrors, clearFieldErrors } from "../ui.js";

/** Abre el diálogo para pasar un reparto en curso a otro chofer activo. Llama a onDone cuando se guardó. */
export function reassignRoute(route, onDone) {
  const select = h("select", { class: "input" }, h("option", { value: "" }, "Cargando choferes…"));
  const submit = h("button", { class: "btn primary", type: "submit" }, "Pasar el reparto");
  const form = h("form", { novalidate: true },
    h("p", { style: "margin-bottom:12px" }, `El reparto #${route.id} sigue con las mismas paradas y el mismo recorrido; solo cambia quién lo hace. Ahora lo lleva ${route.courier.name}.`),
    field("courierId", "Nuevo chofer", select), h("div", { class: "form-error" }),
    h("div", { class: "form-actions" }, submit, h("button", { class: "btn", type: "button", onClick: () => dlg.close() }, "Cancelar")));
  const dlg = dialog({ title: "Pasar a otro chofer", body: form });
  api.get("/couriers").then((r) => {
    const others = r.items.filter((u) => u.active && u.id !== route.courier.id);
    select.replaceChildren(h("option", { value: "" }, others.length ? "Elegí un chofer" : "No hay otro chofer disponible"), ...others.map((u) => h("option", { value: u.id }, u.name)));
  }).catch(() => select.replaceChildren(h("option", { value: "" }, "No se pudo cargar")));
  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    clearFieldErrors(form);
    if (!select.value) return setFieldError(form, "courierId", "Elegí quién sigue con el reparto.");
    try {
      const updated = await busy(submit, () => api.put(`/routes/${route.id}/courier`, { courierId: Number(select.value) }));
      dlg.close();
      toast(`El reparto #${route.id} ahora lo lleva ${updated.courier.name}.`);
      onDone?.(updated);
    } catch (err) {
      showApiErrors(form, err);
    }
  });
}
