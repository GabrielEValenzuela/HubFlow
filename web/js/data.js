import { api } from "./api.js";
import { setCatalogs } from "./ui.js";

let roadMapPromise = null;
let catalogsPromise = null;

export function roadMap() {
  if (!roadMapPromise) {
    roadMapPromise = api.get("/road-map").catch((e) => {
      roadMapPromise = null;
      throw e;
    });
  }
  return roadMapPromise;
}

/** Trae zonas, servicios y características una sola vez por sesión (o hasta que se cambie la configuración). */
export function loadCatalogs() {
  if (!catalogsPromise) {
    catalogsPromise = Promise.all([api.get("/zones"), api.get("/service-levels"), api.get("/shipment-features")])
      .then(([zones, services, features]) => setCatalogs({ zones: zones.items, services: services.items, features: features.items }))
      .catch((e) => {
        catalogsPromise = null;
        throw e;
      });
  }
  return catalogsPromise;
}

export const resetCaches = () => {
  roadMapPromise = null;
  catalogsPromise = null;
};
