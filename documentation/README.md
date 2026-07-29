# Rules for API Function documentation

This document describes how to generate Doxygen-style documentation.

## Required setup

### Windows

- Install bash shell (for example git bash for Windows)
- Install [Doxygen](https://www.doxygen.nl/download.html) (verified to work with v1.13.2)
- Install [Graphviz](http://www.graphviz.org/download/) (verified to work with v14.1.2)
- Install Java (PlantUML requires Java)
  - Check whether Java is already installed, run `java --version` from the command line
  - If not installed, download and install [prebuilt OpenJDK Binaries](https://adoptium.net/temurin/releases) (verified to work with v25.0.2)
- Download [PlantUML compiled Jar under Apache license](https://github.com/plantuml/plantuml/releases/download/v1.2026.1/plantuml-asl-1.2026.1.jar) (verified to work with v1.2026.1):
  - Optional: check [download page](https://plantuml.com/download) for latest version
  - Copy downloaded plantuml-asl-x.jar file to C:/Tools/plantuml/ (or any other directory)
  - Create plantuml.bat with content (adjust if you use different path):

    ```text
    @echo off
    java -jar C:/Tools/plantuml/plantuml-asl-1.2026.1.jar %*
    ```

  - Add path to plantuml.bat to the environment PATH

### Linux and macOS

Use your favorite package manager (apt, brew) to download and install:

- Doxygen (verified to work with v1.13.2)
- Graphviz (verified to work with v14.1.2)
- PlantUML (verified to work with v1.2026.1)

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
.\doxygen\drivers\drivers.dxy | Doxygen configuration for the Drivers section
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
Drivers | Low-level driver behavior, public API, runtime execution contract, interrupts, and platform hooks.
Integration | Cross-component design: memory topology, linker and MPU/SAU placement, cache policy, Vela-to-driver mapping, RTOS concerns, budgeting, validation, and tuning.
Zephyr | Zephyr setup, Ethos-U devicetree and Kconfig configuration, west and CMSIS Solution workflows, and an FVP example.

When information affects more than one component, document the complete decision
in Integration. Keep only the component-specific syntax or API contract in Vela
or Drivers and link to Integration. General should summarize concepts and link
to the main source of information rather than repeat configuration tables.

### Authoring for engineers and automated agents

- Start each page by stating its scope and what it does not own.
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

### Notes

- Check [additional PlantUML documentation](https://plantuml-documentation.readthedocs.io/en/latest/index.html)
