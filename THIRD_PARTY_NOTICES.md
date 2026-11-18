# Third-party materials

Original explanatory text: Copyright (c) 2026 Paulus Handke, CC-BY-NC-SA-4.0.
Upstream material retains its own terms. See `LICENSE` for permission to copy
this notice verbatim for license compliance, including commercially.

## Firmware

Locally installed versions are listed below; dependency ranges in `platformio.ini`
may resolve differently. This list is not a manifest of a particular binary.

| Component | License and source |
| --- | --- |
| AccelStepper 1.64 | Copyright (C) Mike McCauley. [GPLv3 option selected](https://github.com/waspinator/AccelStepper/blob/1.64/LICENSE). |
| TMCStepper 0.7.3 | Copyright (c) 2019 teemuatlut. [MIT](https://github.com/teemuatlut/TMCStepper/blob/v0.7.3/LICENSE); full notice: [LICENSES/MIT-TMCStepper.txt](LICENSES/MIT-TMCStepper.txt). |
| ESP8266 Arduino core 3.1.2 (PlatformIO package 3.30102.0) | [LGPL-2.1](https://github.com/esp8266/Arduino/blob/3.1.2/LICENSE); package declares LGPL-2.1-or-later. Bundled libraries, SDK and other components retain their individual terms. |

When distributing firmware, include required upstream notices and license texts,
provide the corresponding source under the applicable licenses, and check the
components actually included in that build.

## CAD and templates

Official KiCad libraries use [CC-BY-SA-4.0 with the design exception](https://www.kicad.org/libraries/license/).
[Espressif's libraries](https://github.com/espressif/kicad-libraries/blob/master/LICENSE.md)
declare the same terms; the project references `PCM_Espressif` symbols and footprints.
Redistributed library collections retain upstream attribution and license documents.

Origin and applicable licenses still need verification for:

- `KiCad/lib/fertilizer-pump.kicad_sym` and both footprints in `KiCad/lib/fertilizer-pump.pretty/` (BTT TMC2209 and D_SOD-123FL).
- The external `hdk_footprints` entries used by the project (rotary encoder connector and SHOU HAN switches).
- Embedded STEP models: `TMC2209 V1.3.step`, `SW-SMD_L4.0-W2.9-H2-LS5.0.step`, `SOD-123FL_L2.8-W1.8-H1.1-LS3.6.step` and `DFN-10_L3.0-W3.0-P0.50-BL-EP2.5.step`.

These unverified parts are excluded from the project's license grants; their
presence or a datasheet link does not establish permission to redistribute them.
PlatformIO template READMEs, configuration boilerplate and editor settings also
retain their upstream terms and are excluded from the project's original material.
