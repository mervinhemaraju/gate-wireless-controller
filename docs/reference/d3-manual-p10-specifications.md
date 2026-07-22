# D3 Electrical Specifications (D3/D5 Manual)

- **Date:** 2026-07-22
- **Phase:** 3 reference (feeds buck-converter sizing and power budget)
- **Status:** verified (faithful transcription of manual p.10). The aux rail's
  actual loaded/unloaded voltage still has to be measured on this gate.

Source: Centurion D3/D5 installation manual p.10 "Specifications", read visually
from the scanned PDF (poppler render, 2026-07-22). D3 column only; D5 values
omitted.

## D3 electricals [manual p.10]

| Spec | D3 value |
|---|---|
| Power supply voltage | 220V AC +/-10% 50Hz (also 110V AC, 19V AC options) |
| Motor voltage | 12V DC |
| AC current draw @ 220V | 120mA |
| DC current draw (max) | 15A |
| Output shaft rotational speed | 73rpm |
| Rated gate speed (pull force <5kg) | 16m/min |
| Starting thrust | 35kgF |
| Rated thrust | 12kgF |
| Maximum gate mass | 300kg |
| Maximum gate length | 11m |
| End of travel control | Sealed optical counter with origin switch |
| Collision sensitivity | Electronic, adjustable |
| Temperature range | -10C to +50C |
| Housing protection | IP55 |
| Control card | CP80 |
| Corrosion protection (baseplate) | Zincroshield |
| Mass of unit (packed) incl. 7.5Ah battery | 12kg |

Battery / duty (D3): battery-driven with a 7Ah battery; standby ~12 (units per
the manual's "max operations per day / standby" rows). Powered by a 12V DC
system.

## Aux supply for the ESP32

The system is 12V DC, battery-backed (7.5Ah). The **12V** accessory terminal is
"linked directly to the battery via the 3A fast-blow auxiliary fuse" [manual
p.31], so the aux rail sits at battery voltage: nominally 12V, but a
battery-backed rail on charge typically runs higher (commonly ~13.8V). The
manual gives no explicit current rating for the 12V accessory terminal beyond
the 3A fast-blow auxiliary fuse [manual p.40].

**Measure before sizing the buck converter input:** the 12V rail loaded and
unloaded, in the worst case (on charge). Size the buck's max input rating above
the measured on-charge voltage, not the 12V nominal. [UNVERIFIED on this gate:
measure]
