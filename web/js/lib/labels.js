import { h, dialog, SERVICE, ZONE } from "../ui.js";
import { icon } from "../icons.js";
import { roadMap } from "../data.js";

const esc = (t) => String(t).replace(/[&<>"]/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" })[c]);

const PRINT_CSS = `
  @page { size: A6 landscape; margin: 6mm; }
  body { font-family: Arial, Helvetica, sans-serif; margin: 0; color: #000; }
  .label { page-break-after: always; border: 2px solid #000; padding: 6mm; box-sizing: border-box; min-height: 88mm; }
  .label:last-child { page-break-after: auto; }
  .code { font-size: 34pt; font-weight: 700; letter-spacing: 1px; margin: 0 0 2mm; }
  .parcel { font-size: 20pt; font-weight: 700; margin: 0 0 4mm; }
  .row { font-size: 13pt; margin: 1mm 0; }
  .row b { display: inline-block; min-width: 38mm; }
  .together { margin-top: 4mm; font-size: 12pt; font-weight: 700; border: 2px solid #000; padding: 2mm; }
`;

function labelHtml(shipment, parcel, destination, together) {
  const total = shipment.parcelCount;
  return `<section class="label">
    <p class="code">${esc(shipment.code)}</p>
    <p class="parcel">Bulto ${parcel.ordinal} de ${total}</p>
    <p class="row"><b>Para</b>${esc(shipment.recipient)}</p>
    <p class="row"><b>Entregar en</b>${esc(destination)} (${esc(ZONE[shipment.zone] ?? shipment.zone)})</p>
    <p class="row"><b>Servicio</b>${esc(SERVICE[shipment.service] ?? shipment.service)}</p>
    <p class="row"><b>Peso a facturar</b>${parcel.billableWeightKg.toLocaleString("es-AR")} kg</p>
    <p class="row"><b>Etiqueta</b>${esc(parcel.label)}</p>
    ${together ? `<p class="together">Viaja junto con: ${esc(together)}</p>` : ""}
  </section>`;
}

/** Texto de las etiquetas listo para imprimir: una por bulto. */
export async function labelsHtml(shipment) {
  const map = await roadMap();
  const destination = map.nodes.find((n) => n.id === shipment.deliveryPoint)?.label ?? shipment.deliveryPoint;
  const together = shipment.group?.travelsWith?.join(", ") ?? "";
  return shipment.parcels.map((p) => labelHtml(shipment, p, destination, together)).join("");
}

export async function printLabels(shipment) {
  const html = await labelsHtml(shipment);
  const frame = document.createElement("iframe");
  frame.setAttribute("aria-hidden", "true");
  frame.style.cssText = "position:fixed;right:0;bottom:0;width:0;height:0;border:0";
  document.body.append(frame);
  const doc = frame.contentDocument;
  doc.open();
  doc.write(`<!doctype html><html lang="es"><head><meta charset="utf-8"><title>Etiquetas ${esc(shipment.code)}</title><style>${PRINT_CSS}</style></head><body>${html}</body></html>`);
  doc.close();
  setTimeout(() => {
    frame.contentWindow.focus();
    frame.contentWindow.print();
    setTimeout(() => frame.remove(), 1000);
  }, 150);
}

/** Aviso al ingresar un paquete: el código nuevo en grande y las etiquetas para imprimir. */
export function showRegistered(shipment, { onClose } = {}) {
  const preview = h("div", { class: "label-preview" });
  labelsHtml(shipment).then((html) => { preview.innerHTML = html.replaceAll("class=\"label\"", "class=\"label-card\""); });
  const body = h("div", { class: "registered" },
    h("p", { class: "muted" }, "El sistema le asignó este código de seguimiento:"),
    h("p", { class: "big-code", "aria-label": "Código de seguimiento " + shipment.code }, shipment.code),
    h("p", {}, shipment.parcelCount === 1 ? "Hay que pegar 1 etiqueta." : `Hay que pegar ${shipment.parcelCount} etiquetas, una en cada bulto.`),
    preview,
    h("div", { class: "form-actions" },
      h("button", { class: "btn primary", onClick: () => printLabels(shipment) }, icon("printer"), "Imprimir etiquetas"),
      h("button", { class: "btn", onClick: () => dlg.close() }, "Listo")));
  const dlg = dialog({ title: "Paquete ingresado", body, onClose });
  return dlg;
}
