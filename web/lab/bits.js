import { h, STATE_ORDER as BIT_ORDER, STATUS } from "../js/ui.js";

/** Representación del BitSet de 5 bits: un bit por estado alcanzado. */
export function bitChips(state) {
  const bits = state?.bits ?? "00000";
  const padded = bits.padStart(5, "0");
  const wrap = h("span", { class: "bits", role: "list", "aria-label": "Estados por los que pasó el envío" });
  for (let pos = 4; pos >= 0; pos--) {
    const on = padded[4 - pos] === "1";
    const name = BIT_ORDER[pos];
    wrap.append(
      h("span", { class: "bit" + (on ? " on" : ""), role: "listitem", title: `${STATUS[name]}: ${on ? "sí" : "no"}` },
        h("b", {}, on ? "1" : "0"), STATUS[name]),
    );
  }
  return wrap;
}

