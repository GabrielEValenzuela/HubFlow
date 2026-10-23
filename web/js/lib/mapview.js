import { svg, zoneColor } from "../ui.js";

const K = 52;

export class MapView {
  constructor(map, { compact = false, stops = [], title = "Mapa de la ciudad" } = {}) {
    this.map = map;
    this.compact = compact;
    this.nodes = new Map(map.nodes.map((n) => [n.id, n]));
    const xs = map.nodes.map((n) => n.x), ys = map.nodes.map((n) => n.y);
    const pad = compact ? 1.2 : 1.6;
    const minX = (Math.min(...xs) - pad) * K, minY = (Math.min(...ys) - pad) * K;
    const w = (Math.max(...xs) - Math.min(...xs) + pad * 2) * K, hgt = (Math.max(...ys) - Math.min(...ys) + pad * 2) * K;
    this.el = svg("svg", { class: "citymap" + (compact ? " compact" : ""), viewBox: `${minX} ${minY} ${w} ${hgt}`, role: "img", "aria-label": title });
    this.edgeEls = new Map();
    this.nodeEls = new Map();
    const gEdges = svg("g", { class: "edges" });
    const gPath = svg("g", { class: "paths" });
    const gNodes = svg("g", { class: "nodes" });
    this.gPath = gPath;
    for (const e of map.edges) {
      const a = this.nodes.get(e.from), b = this.nodes.get(e.to);
      const line = svg("line", { x1: a.x * K, y1: a.y * K, x2: b.x * K, y2: b.y * K, class: "edge" });
      gEdges.append(line);
      this.edgeEls.set(key(e.from, e.to), line);
      if (!compact) gEdges.append(svg("text", { x: ((a.x + b.x) / 2) * K, y: ((a.y + b.y) / 2) * K - 3, class: "ecost", "text-anchor": "middle" }, String(e.costKm)));
    }
    const stopMap = new Map(stops.map((s) => [s.nodeId, s]));
    this.stopEls = new Map();
    for (const n of map.nodes) {
      const g = svg("g", { class: "mnode" + (n.isHub ? " hub" : "") + (n.active === false ? " off" : ""), style: n.isHub ? null : `--zc:${zoneColor(n.zone)}` });
      g.append(svg("title", {}, `${n.id} · ${n.label}`));
      const r = compact ? 12 : 15;
      g.append(n.isHub ? svg("rect", { x: n.x * K - r, y: n.y * K - r, width: r * 2, height: r * 2, rx: 6, class: "shape" }) : svg("circle", { cx: n.x * K, cy: n.y * K, r, class: "shape" }));
      g.append(svg("circle", { cx: n.x * K, cy: n.y * K, r: r + 6, class: "ring" }));
      g.append(svg("text", { x: n.x * K, y: n.y * K + 4, class: "nid", "text-anchor": "middle" }, n.id));
      if (!compact) g.append(svg("text", { x: n.x * K, y: n.y * K + r + 15, class: "nlabel", "text-anchor": "middle" }, n.label));
      const stop = stopMap.get(n.id);
      if (stop) {
        const badge = svg("g", { class: "stopmark " + (stop.outcome ?? "PENDIENTE") },
          svg("circle", { cx: n.x * K + r, cy: n.y * K - r, r: 9 }),
          svg("text", { x: n.x * K + r, y: n.y * K - r + 3.5, "text-anchor": "middle" }, stop.outcome === "ENTREGADO" ? "✓" : stop.outcome === "REPROGRAMADO" ? "!" : String(stop.order)));
        g.append(badge);
        this.stopEls.set(n.id, badge);
      }
      gNodes.append(g);
      this.nodeEls.set(n.id, g);
    }
    this.courier = svg("g", { class: "courier", hidden: true }, svg("circle", { r: 9 }), svg("path", { d: "M-4 3l4-8 4 8-4-2z", class: "arrowhead" }));
    this.el.append(gEdges, gPath, gNodes, this.courier);
    this.anim = null;
  }

  render({ expanded = null, frontier = [], explored = [], path = null, done = [], from = null, to = null, courier = null, animate = false, speed = 1 } = {}) {
    const fs = new Set(frontier), es = new Set(explored);
    for (const [id, g] of this.nodeEls) {
      g.classList.toggle("explored", es.has(id));
      g.classList.toggle("frontier", fs.has(id));
      g.classList.toggle("expanded", id === expanded);
      g.classList.toggle("origin", id === from);
      g.classList.toggle("target", id === to);
    }
    for (const l of this.edgeEls.values()) l.classList.remove("on", "done");
    this.gPath.replaceChildren();
    const draw = (p, cls) => {
      if (!p || p.length < 2) return;
      this.gPath.append(svg("polyline", { points: p.map((id) => `${this.nodes.get(id).x * K},${this.nodes.get(id).y * K}`).join(" "), class: "route " + cls }));
    };
    for (const p of done) draw(p, "done");
    draw(path, "now");
    this.moveCourier(courier, path, animate, speed);
  }

  moveCourier(at, path, animate, speed) {
    cancelAnimationFrame(this.anim);
    if (!at) {
      this.courier.setAttribute("hidden", "");
      return;
    }
    this.courier.removeAttribute("hidden");
    const pts = (path && animate && path.length > 1 ? path : [at]).map((id) => this.nodes.get(id));
    if (pts.length === 1) return this.place(pts[0].x * K, pts[0].y * K, 0);
    const lens = [];
    let total = 0;
    for (let i = 1; i < pts.length; i++) {
      const d = Math.hypot((pts[i].x - pts[i - 1].x) * K, (pts[i].y - pts[i - 1].y) * K);
      lens.push(d);
      total += d;
    }
    const duration = Math.max(500, 800 / speed);
    const t0 = performance.now();
    const step = (now) => {
      const t = Math.min(1, (now - t0) / duration);
      let d = t * total, i = 0;
      while (i < lens.length - 1 && d > lens[i]) { d -= lens[i]; i++; }
      const f = lens[i] ? d / lens[i] : 1;
      const x = (pts[i].x + (pts[i + 1].x - pts[i].x) * f) * K, y = (pts[i].y + (pts[i + 1].y - pts[i].y) * f) * K;
      this.place(x, y, Math.atan2(pts[i + 1].y - pts[i].y, pts[i + 1].x - pts[i].x) * 180 / Math.PI + 90);
      if (t < 1) this.anim = requestAnimationFrame(step);
    };
    this.anim = requestAnimationFrame(step);
  }

  place(x, y, deg) {
    this.courier.setAttribute("transform", `translate(${x} ${y})`);
    this.courier.firstChild.nextSibling.setAttribute("transform", `rotate(${deg})`);
  }

  markStop(nodeId, outcome, order) {
    const b = this.stopEls.get(nodeId);
    if (!b) return;
    b.setAttribute("class", "stopmark " + outcome);
    b.querySelector("text").textContent = outcome === "ENTREGADO" ? "✓" : outcome === "REPROGRAMADO" ? "!" : String(order);
  }

  destroy() {
    cancelAnimationFrame(this.anim);
  }
}

const key = (a, b) => (a < b ? a + "|" + b : b + "|" + a);
