import { api } from "./api.js";
import { setCatalogs } from "./ui.js";
import { TRAVEL_ESTIMATE } from "./lib/routeOptions.js";

let roadMapPromise = null;
let catalogsPromise = null;

export function roadMap() {
  if (!roadMapPromise) {
    roadMapPromise = api.get("/road-map").then((map) => {
      if (map.travelEstimate) Object.assign(TRAVEL_ESTIMATE, map.travelEstimate);
      return map;
    }).catch((e) => {
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

let trackingBasePromise = null;

/** Enlace público de seguimiento de un paquete; la dirección base la define el servidor (HUBFLOW_PUBLIC_BASE_URL). */
export async function trackingLink(code) {
  if (!trackingBasePromise) {
    trackingBasePromise = api.get("/public/tracking-settings").then((s) => s.trackingLinkBase).catch((e) => {
      trackingBasePromise = null;
      throw e;
    });
  }
  return (await trackingBasePromise) + "?codigo=" + encodeURIComponent(code);
}

export const resetCaches = () => {
  roadMapPromise = null;
  catalogsPromise = null;
};
