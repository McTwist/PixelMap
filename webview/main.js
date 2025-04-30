import './style.css';
import { Map, View } from 'ol';
import TileLayer from 'ol/layer/Tile';
import { defaults as defaultControls } from 'ol/control/defaults';
import { Projection } from 'ol/proj';
import { XYZ } from 'ol/source';
//import { TileDebug } from 'ol/source';
import { TileGrid } from 'ol/tilegrid';

const extentInf = [-Infinity, -Infinity, Infinity, Infinity];

const projection = new Projection({
  code: 'pixelmap',
  units: 'pixels',
  extent: [0, 0, 512, 512]
});

const tileGrid = new TileGrid({
  extent: extentInf,
  minZoom: 1,
  resolutions: [256, 128, 64, 32, 16, 8, 4, 2, 1],
  tileSize: [512, 512],
  origin: [0, 0]
});

/*const regionGrid = new TileGrid({
  extend: extentInf,
  minZoom: 1,
  resolutions: [256, 128, 64, 32, 16, 8, 4, 2, 1],
  tileSizes: [1, 2, 4, 8, 16, 32, 64, 128, 256],
  origin: [0, 0]
});*/

const dataLayer = new TileLayer({
  source: new XYZ({
    url: 'data/{z}/r.{x}.{y}.png',
    maxZoom: 8,
    projection,
    tileGrid,
    wrapX: false
  })
});

/*const debugLayer = new TileLayer({
  source: new TileDebug({
    tileGrid,
    projection,
    wrapX: false
  }),
  visible: false
});

const regionLayer = new TileLayer({
  source: new TileDebug({
    template: "x:{x} y:{y}",
    tileGrid: regionGrid,
    projection,
    wrapX: false
  }),
  visible: false
});*/

const view = new View({
  projection,
  extent: extentInf,
  resolutions: tileGrid.getResolutions(),
  center: [0, 0],
  zoom: 8,
  zoomFactor: 1,
  enableRotation: false
});

const map = new Map({
  target: 'map',
  layers: [
    dataLayer,
    //regionLayer,
    //debugLayer
  ],
  controls: defaultControls({
    attribution: false
  }),
  view
});

/*map.on('moveend', () => {
  regionLayer.setVisible(map.getView().getZoom() > 6.5);
});*/
