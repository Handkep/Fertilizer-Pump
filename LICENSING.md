# Licensing and releases

Copyright (c) 2026 Paulus Handke. This original explanatory document is licensed
under CC-BY-NC-SA-4.0. The full license texts, rather than this summary, govern.

The additional permission in `LICENSE` allows verbatim redistribution of these
compliance notices, including with commercial firmware releases.

## File assignments

| Material | Assignment |
| --- | --- |
| Original source in `Firmware/Fertilizer Pump/src/`, and original future headers and tests | GPL-3.0-only |
| Original build configuration in `Firmware/Fertilizer Pump/platformio.ini` | GPL-3.0-only; pre-existing PlatformIO template text is excluded |
| Original circuit, PCB layout and project content in `KiCad/*.kicad_sch`, `KiCad/*.kicad_pcb` and `KiCad/*.kicad_pro` | CC-BY-NC-SA-4.0, to the extent protected by copyright or similar rights |
| Original design data exported to `KiCad/production/`, including Gerbers, drill files, netlists, BOMs and placement files | CC-BY-NC-SA-4.0, to the extent protected by copyright or similar rights |
| Original project `README.md`, this document, original text in `THIRD_PARTY_NOTICES.md`, and original production instructions | CC-BY-NC-SA-4.0 |
| Original future mechanical CAD and illustrations | CC-BY-NC-SA-4.0, with explicit notices when added and only to the extent of the author's rights |
| `KiCad/lib/`, embedded third-party symbols/footprints/models, external dependencies, PlatformIO template READMEs, editor settings and other third-party/template material | Excluded from the above grants; see `THIRD_PARTY_NOTICES.md` and applicable upstream notices |
| `LICENSES/` and quoted upstream notices | Their own copying terms; not relicensed as project documentation |

These are content assignments, not a claim to ownership of every byte in a
design file. Embedded third-party material retains its own license. This file
is the accompanying license notice for original CAD and generated files that
cannot conveniently carry a source-code header. Do not label library material
with the project's CC license merely because it is stored locally.

The firmware identifier is `GPL-3.0-only`. The identifier for original protected
design files and documentation is `CC-BY-NC-SA-4.0`. Copyright attribution for
original project contributions is `Copyright (c) 2026 Paulus Handke`.
The standard license texts are unmodified; no custom hardware license is used.

## What users may do

| Scenario | Result |
| --- | --- |
| Copy, modify or share protected design files for a private hobby | Permitted under CC-BY-NC-SA-4.0 |
| Pay a PCB or printing service to make parts for that hobby build | Permitted by the separate additional permission in `LICENSE`, to the extent of Paulus Handke's rights |
| Share modified protected design files noncommercially | Give credit, retain required notices, identify changes and comply with CC-BY-NC-SA-4.0 ShareAlike terms |
| Commercially exploit protected design files or documentation | Separate permission required to the extent the licensed rights apply |
| Manufacture, sell or operate physical hardware | CC-BY-NC-SA-4.0 alone does not guarantee control of these activities; assess applicable rights rather than assuming a blanket prohibition |
| Use, modify or redistribute firmware commercially | Permitted under the GPL; the hardware noncommercial terms do not apply to firmware |
| Share modified original documentation noncommercially | Follow attribution and ShareAlike requirements of CC-BY-NC-SA-4.0 |

No public disclosure of private modifications or delivery of editable CAD source
is required by CC-BY-NC-SA-4.0. An institution's
status does not give a blanket commercial-use exemption. Commercial hardware
or documentation permission requests go to
[repository issues](https://github.com/Handkep/Fertilizer-Pump/issues).
A separate written agreement is required; an unanswered request is not consent.

This is a mixed-license project with open-source firmware and noncommercial
design files. Do not describe the whole project as unrestricted open-source
hardware. CC-BY-NC-SA-4.0 licenses copyright and similar rights in protected
material, not patents, trademarks or design rights. It cannot create protection
for ideas or unprotected functional features, or guarantee control over physical
manufacture, operation or independently designed hardware. Creative Commons does
not recommend its licenses for hardware; the choice here concerns protected
design files. See the [Creative Commons FAQ](https://creativecommons.org/faq/).
Existing lawful grants for earlier versions are not withdrawn. The full
[standard license](LICENSES/CC-BY-NC-SA-4.0.txt) governs over explanatory summaries.

## Firmware release checklist

The current build targets ESP8266/NodeMCU with the Arduino framework; the README's
ESP32-S3 is planned hardware. Dependency ranges are not a record of the exact
versions used in a binary. No resolved package inventory is currently available
in this checkout. Before distributing any firmware binary or pre-flashed device:

1. Record the exact PlatformIO platform, framework, toolchain and library versions
   actually used. Review their complete licenses, component exceptions and notices,
   including bundled SDK code. Resolve any incompatible or unclear rights before
   distributing the combined binary.
2. Provide the complete Corresponding Source required by GPLv3: the exact project
   revision and modifications, required dependency sources, configuration, build
   scripts and installation information to the extent required. Use a GPLv3
   section 6 distribution method appropriate to the delivery channel. For a
   downloadable binary, provide equivalent source access alongside it; for a
   physical device, use an applicable section 6 method rather than assuming a
   link to a moving repository branch suffices.
3. Include GPLv3, required upstream license texts, copyright notices and any
   required modification notices. Meet the actual LGPL and other component
   obligations; this repository's GPL notice does not replace them.
4. If GPLv3 Installation Information for a User Product is required, supply it.
   Do not impose hardware noncommercial terms on recipients' GPL software rights.

No dependency replacement, commercial AccelStepper license, or technical firmware
change is required by the selected project licensing scheme.

## Hardware exports and contributions

With every future manufacturing package, include CC-BY-NC-SA-4.0,
source location, attribution, modification information and relevant third-party
notices. Ensure the source matches the export. Use the production instructions
for the existing exports; existing archive contents have not been rewritten.

Before redistributing local library collections, resolve their provenance and
include the applicable upstream licenses. Unknown origin is not a license grant.

Contributions must identify their author, origin and applicable license. Firmware
contributions use GPL-3.0-only; original protected design files and documentation
use CC-BY-NC-SA-4.0. Contributors retain their
copyright. These terms alone do not give Paulus Handke permission to grant
commercial rights in someone else's noncommercial contributions. Such rights
need separate written agreement from the relevant contributor. No copyright
assignment or commercial relicensing consent is implied by a pull request.
