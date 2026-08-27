# Rules for API Function documentation

This document describes how to generate Doxygen-style documentation.

## Required setup

### Windows

- Install bash shell (for example git bash for Windows)
- Install [Doxygen](https://www.doxygen.nl/download.html) v1.17.0 or later

### Linux and macOS

Use your favorite package manager (apt, brew) to download and install:

- Doxygen v1.17.0 or later

## Build documentation

To generate documentation execute `./gen_doc.sh` in a bash shell.

## Folder structure

Directory | Description
:-- | :--
.\doxygen\ | Doxygen documentation for Ethos-U
.\doxygen\gen_doc.sh | Script for building documentation
.\doxygen\style_template\ | Additional files defining styles and appearance
.\doxygen\general\general.dxy | Doxygen configuration for the General section
.\doxygen\vela\vela.dxy | Doxygen configuration for the Vela section
.\doxygen\driver\driver.dxy | Doxygen configuration for the Driver section
.\doxygen\integration\integration.dxy | Doxygen configuration for the Integration section
.\doxygen\zephyr\zephyr.dxy | Doxygen configuration for the Zephyr section
.\doxygen\&lt;section&gt;\src\ | Markdown sources for each section
.\doxygen\&lt;section&gt;\src\images\ | Image assets for each section
.\documentation\ | html output folder

## Content

The manual has five sections:

Section | Content
:-- | :--
General | Architecture overview, terminology, and routing to detailed material.
Vela | Compiler installation, inputs, options, configuration syntax, output, and compiler diagnostics.
Driver | Low-level driver behavior, public API, runtime execution contract, interrupts, and platform hooks.
Integration | Cross-component design: memory topology, linker and MPU/SAU placement, cache policy, Vela-to-driver mapping, RTOS concerns, budgeting, validation, and tuning.
Zephyr | Zephyr setup, Ethos-U devicetree and Kconfig configuration, west and CMSIS Solution workflows, and an FVP example.

When information affects more than one component, document the complete decision
in Integration. Keep only the component-specific syntax or API contract in Vela
or Driver and link to Integration. General should summarize concepts and link
to the main source of information rather than repeat configuration tables.

### Authoring for engineers and automated agents

- Start each chapter by stating its scope.
- Use the exact configuration keys, API names, files, and generated region
  numbers that a reader must inspect or change.
- Separate facts, platform assumptions, examples, and measurements. Label
  target-specific examples as examples.
- Record tool and model versions where behavior can change.
- Prefer decision tables and verification checklists for mappings that span
  Vela, linker, driver, and hardware configuration.
- Give every concept one primary explanation. Elsewhere, add a short context
  sentence and a link.
- Do not publish unresolved investigation notes as guidance. Move confirmed,
  reusable conclusions into the owning section and leave a provenance pointer
  in `temp` if the history remains useful.
