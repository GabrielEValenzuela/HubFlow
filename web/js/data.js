import { api } from "./api.js";

let roadMapPromise = null;

export function roadMap() {
  if (!roadMapPromise) {
    roadMapPromise = api.get("/road-map").catch((e) => {
      roadMapPromise = null;
      throw e;
    });
  }
  return roadMapPromise;
}

export const resetCaches = () => {
  roadMapPromise = null;
};
