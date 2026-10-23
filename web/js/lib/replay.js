export function buildFrames(sequence) {
  const frames = [];
  sequence.segments.forEach((seg, si) => {
    seg.steps.forEach((st, i) => {
      frames.push({ type: "step", seg: si, from: seg.from, to: seg.to, index: i, count: seg.steps.length, expanded: st.expanded, frontier: st.frontier, explored: st.explored, metrics: seg.metrics });
    });
    frames.push({ type: "path", seg: si, from: seg.from, to: seg.to, path: seg.path, expanded: null, frontier: [], explored: seg.steps.length ? seg.steps[seg.steps.length - 1].explored : [], metrics: seg.metrics });
  });
  return frames;
}

export class Replay {
  constructor(length, onFrame) {
    this.length = length;
    this.onFrame = onFrame;
    this.index = -1;
    this.playing = false;
    this.speed = 1;
    this.timer = null;
    this.onState = () => {};
  }

  seek(i) {
    this.index = Math.max(-1, Math.min(this.length - 1, i));
    this.onFrame(this.index);
    this.onState();
  }

  next() {
    if (this.index >= this.length - 1) return this.pause();
    this.seek(this.index + 1);
  }

  prev() {
    this.pause();
    this.seek(this.index - 1);
  }

  reset() {
    this.pause();
    this.seek(-1);
  }

  play() {
    if (this.index >= this.length - 1) this.index = -1;
    this.playing = true;
    this.onState();
    this.tick();
  }

  pause() {
    this.playing = false;
    clearTimeout(this.timer);
    this.onState();
  }

  toggle() {
    this.playing ? this.pause() : this.play();
  }

  setSpeed(s) {
    this.speed = s;
  }

  tick() {
    clearTimeout(this.timer);
    this.timer = setTimeout(() => {
      if (!this.playing) return;
      this.seek(this.index + 1);
      if (this.index >= this.length - 1) this.pause();
      else this.tick();
    }, 900 / this.speed);
  }

  destroy() {
    this.playing = false;
    clearTimeout(this.timer);
  }
}
