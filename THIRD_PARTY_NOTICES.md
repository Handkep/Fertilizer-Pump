# Third-party materials

Original explanatory text: Copyright (c) 2026 Paulus Handke, CC-BY-NC-SA-4.0.
Upstream notices and license texts retain their own terms. This inventory records
known declarations and unresolved items; it is not a certification of a binary.
The additional permission in `LICENSE` allows verbatim redistribution of this
notice with commercial firmware releases for license compliance.

## Firmware dependencies

`Firmware/Fertilizer Pump/platformio.ini` declares these dependencies. No local
`.pio` package installation or resolved version manifest was found during this
licensing change. The sources below verify the named versions, not a future
resolution of the version ranges.

| Component | Declaration / verified reference | License and attribution |
| --- | --- | --- |
| AccelStepper | `waspinator/AccelStepper@^1.64`; reference 1.64 | GPLv3 or a separately negotiated commercial license; Copyright (C) Mike McCauley. This project selects the GPLv3 option. |
| TMCStepper | `teemuatlut/TMCStepper@^0.7.3`; reference v0.7.3 | MIT; Copyright (c) 2019 teemuatlut. Full upstream text: `LICENSES/MIT-TMCStepper.txt`. |
| ESP8266 Arduino core, including Arduino/SoftwareSerial dependencies | `platform = espressif8266`, `framework = arduino`; resolved version unavailable | Upstream reference 3.1.2 has an LGPL-2.1 root license. Files and bundled SDK/components can have different licenses; review the actual installed distribution before a binary release. |

Upstream sources:

- [AccelStepper 1.64 license](https://github.com/waspinator/AccelStepper/blob/1.64/LICENSE)
- [AccelStepper licensing and commercial option](https://www.airspayce.com/mikem/arduino/AccelStepper/)
- [TMCStepper v0.7.3 license](https://github.com/teemuatlut/TMCStepper/blob/v0.7.3/LICENSE)
- [ESP8266 Arduino core 3.1.2 root license](https://github.com/esp8266/Arduino/blob/3.1.2/LICENSE)
- [ESP8266 Arduino core repository](https://github.com/esp8266/Arduino)

These components retain their own terms. They are not subject to the CC-BY-NC-SA-4.0
license on original project design files and documentation. Before
distributing a binary, audit exact resolved versions and all bundled components
using the checklist in `LICENSING.md`.

## KiCad symbols, footprints and models

Official KiCad libraries use CC-BY-SA-4.0 with the design exception documented at
<https://www.kicad.org/libraries/license/>. That exception allows designs using
the library data to have other licenses. It does not generally exempt
redistributed library collections from the library's terms. Preserve upstream
license and attribution documents when distributing such collections.

The following local files have no sufficiently verified provenance or individual
license notice in this checkout:

- `KiCad/lib/fertilizer-pump.kicad_sym`: module symbol referring to BIGTREETECH
  TMC2209 documentation; this link does not establish authorship or permission.
- `KiCad/lib/fertilizer-pump.pretty/BTT TMC2209.kicad_mod`: description refers to
  a Pololu breakout footprint; the exact upstream source and modification history
  are unconfirmed.
- `KiCad/lib/fertilizer-pump.pretty/D_SOD-123FL.kicad_mod`: origin and license are
  unconfirmed.

These library files are excluded from the project's CC-BY-NC-SA-4.0 grant.
Confirm their origin and retain upstream notices before redistributing them as
a library collection. Apply the KiCad library license only after confirming the
material actually comes from those libraries. The same principle applies to
third-party data embedded in schematic/PCB files and generated outputs; a project
license never overwrites upstream rights.

## Templates and tools

The `include/README`, `lib/README` and `test/README` within the firmware project
contain PlatformIO instructional templates. PlatformIO boilerplate in build
configuration, editor configuration, and ignore files is not claimed as original
project documentation or hardware design. Tool-generated data and external CAD
models must retain any applicable third-party terms. No license on the KiCad or
PlatformIO application itself is being applied to the project's original output.
