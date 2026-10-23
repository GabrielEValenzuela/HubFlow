import { svg } from "../js/ui.js";

const short = (c) => c.replace(/^(PKG|HF)-/, "");

export function drawBst(root, { path = [], found = null } = {}) {
  const nodes = [];
  let index = 0;
  (function walk(n, depth, parent) {
    if (!n) return;
    walk(n.left, depth + 1, n);
    const item = { code: n.code, depth, x: index++, parent: parent?.code ?? null, side: parent ? (parent.left === n ? "izquierda" : "derecha") : null };
    nodes.push(item);
    walk(n.right, depth + 1, n);
  })(root, 0, null);
  const byCode = new Map(nodes.map((n) => [n.code, n]));
  const W = 78, H = 62, PX = 46, PY = 36;
  const width = Math.max(1, nodes.length) * W + PX;
  const height = (Math.max(0, ...nodes.map((n) => n.depth)) + 1) * H + PY;
  const pos = (n) => [PX / 2 + n.x * W + W / 2 - 4, PY / 2 + n.depth * H + 10];
  const pathSet = new Set(path.map((p) => p.code));
  const dir = new Map(path.map((p) => [p.code, p.direction]));
  const el = svg("svg", { class: "tree-svg", viewBox: `0 0 ${width} ${height}`, width, height, role: "img", "aria-label": "Árbol binario de búsqueda de códigos de envío" });
  for (const n of nodes) {
    if (!n.parent) continue;
    const [x1, y1] = pos(byCode.get(n.parent));
    const [x2, y2] = pos(n);
    const on = pathSet.has(n.code) && pathSet.has(n.parent);
    el.append(svg("line", { x1, y1: y1 + 14, x2, y2: y2 - 14, class: "edge" + (on ? " on" : "") }));
  }
  for (const n of nodes) {
    const [x, y] = pos(n);
    const on = pathSet.has(n.code);
    const hit = found === n.code;
    const g = svg("g", { class: "node" + (on ? " on" : "") + (hit ? " hit" : "") },
      svg("title", {}, n.code + (dir.get(n.code) ? " · " + dir.get(n.code) : "")),
      svg("rect", { x: x - 33, y: y - 14, width: 66, height: 28, rx: 7 }),
      svg("text", { x, y: y + 4, "text-anchor": "middle" }, short(n.code)));
    if (on && dir.get(n.code) && dir.get(n.code) !== "encontrado") {
      g.append(svg("text", { x, y: y + 27, "text-anchor": "middle", class: "dir" }, "→ " + dir.get(n.code)));
    }
    el.append(g);
  }
  return el;
}

export function drawBPlus(root, { hits = new Set(), range = null } = {}) {
  const KEY_W = 60, PAD = 10, GAP = 22, LEVEL_H = 92;
  const levels = [];
  let leafCursor = 0;
  const measure = (n) => PAD * 2 + Math.max(1, n.keys.length) * KEY_W;
  (function place(n, depth) {
    (levels[depth] ??= []).push(n);
    n.w = measure(n);
    n.depth = depth;
    if (n.leaf || !n.children.length) {
      n.cx = leafCursor + n.w / 2;
      leafCursor += n.w + GAP;
    } else {
      n.children.forEach((c) => place(c, depth + 1));
      n.cx = (n.children[0].cx + n.children[n.children.length - 1].cx) / 2;
    }
  })(root, 0);
  const width = Math.max(leafCursor, root.w + 20) + 20;
  const height = levels.length * LEVEL_H + 24;
  const el = svg("svg", { class: "tree-svg", viewBox: `0 0 ${width} ${height}`, width, height, role: "img", "aria-label": "Árbol B+ de orden 4" });
  el.append(svg("defs", {}, svg("marker", { id: "arrow", viewBox: "0 0 10 10", refX: 8, refY: 5, markerWidth: 7, markerHeight: 7, orient: "auto" }, svg("path", { d: "M0 0 10 5 0 10z", class: "arrow-head" }))));
  const flat = levels.flat();
  const leaves = flat.filter((n) => n.leaf || !n.children.length);
  const y = (n) => 14 + n.depth * LEVEL_H;
  for (const n of flat) {
    for (const c of n.children ?? []) {
      el.append(svg("line", { x1: n.cx, y1: y(n) + 34, x2: c.cx, y2: y(c), class: "edge" + (hasHit(c) ? " on" : "") }));
    }
  }
  function hasHit(n) {
    if (n.leaf || !n.children?.length) return n.keys.some((k) => hits.has(k));
    return n.children.some(hasHit);
  }
  leaves.forEach((l, i) => {
    const nx = leaves[i + 1];
    if (nx) {
      const ay = y(l) + 17;
      el.append(svg("line", { x1: l.cx + l.w / 2, y1: ay, x2: nx.cx - nx.w / 2 - 1, y2: ay, class: "leaf-link" + (hasHit(l) && hasHit(nx) ? " on" : ""), "marker-end": "url(#arrow)" }));
    }
  });
  for (const n of flat) {
    const leaf = n.leaf || !n.children.length;
    const g = svg("g", { class: "bnode" + (leaf ? " leaf" : "") + (hasHit(n) ? " on" : "") });
    g.append(svg("rect", { x: n.cx - n.w / 2, y: y(n), width: n.w, height: 34, rx: 8 }));
    n.keys.forEach((k, i) => {
      const x = n.cx - n.w / 2 + PAD + i * KEY_W;
      if (i > 0) g.append(svg("line", { x1: x, y1: y(n) + 5, x2: x, y2: y(n) + 29, class: "sep" }));
      if (hits.has(k)) g.append(svg("rect", { x: x + 3, y: y(n) + 5, width: KEY_W - 6, height: 24, rx: 5, class: "keybg" }));
      g.append(svg("text", { x: x + KEY_W / 2, y: y(n) + 22, "text-anchor": "middle", class: hits.has(k) ? "key hit" : "key" }, short(k)));
    });
    g.append(svg("text", { x: n.cx, y: y(n) + 50, "text-anchor": "middle", class: "cap" }, leaf ? "hoja" : "índice"));
    el.append(g);
  }
  if (range) el.setAttribute("data-range", range);
  return el;
}
