# Zephyr

This chapter explains how to configure and build a Zephyr application that
uses an Arm Ethos-U NPU. It covers the Zephyr module, devicetree, Kconfig,
command-line `west` workflow, and the CMSIS Solution extension. Board porting,
interrupt encoding, memory addresses, and security attribution remain
device-specific and must come from the board or device support package.

## How the integration works

Zephyr provides the Ethos-U driver as a module and connects it to TensorFlow
Lite for Microcontrollers (TFLM) as a custom operator. A Vela-compiled model
contains the Ethos-U command stream. When the application invokes the model,
TFLM dispatches the custom operator to the Ethos-U driver.

The following configuration layers must agree:

| Layer | Responsibility |
|---|---|
| Vela | Compile the model for the exact Ethos-U architecture, MAC configuration, system configuration, and memory mode. |
| Devicetree | Describe the NPU register range, interrupt, security/privilege mode, and optional fast-memory region. |
| Kconfig | Enable TFLM and the Ethos-U integration. |
| Board support | Provide the physical addresses, interrupt specifier, memory regions, and required security attribution. |

See <a href="../vela/index.html">Vela</a> for model compilation and
<a href="../integration/index.html">Integration</a> for the end-to-end memory and driver
mapping.

## Prerequisites

1. Install Zephyr, its Python virtual environment, and `west` by following the
   [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).
2. Select a Zephyr board that describes an Ethos-U NPU, or add the required
   device-specific devicetree node to the board support.
3. Install the FVP or hardware debug connection required by the selected board.
4. Compile the quantized model with Vela for the NPU and memory configuration
   implemented by that board.

Enable the TFLM module in the Zephyr workspace and fetch its dependencies:

```bash
west config manifest.project-filter -- +tflite-micro
west update
```

Run these commands from the west workspace. Re-run `west update` when the
manifest revision or module selection changes.

## Describe the NPU in devicetree

The Zephyr [`arm,ethos-u` binding](https://docs.zephyrproject.org/latest/build/dts/api/bindings/arm/arm%2Cethos-u.html)
belongs in the device-specific devicetree because the NPU address and interrupt
are properties of the SoC integration. Use an application overlay only to
adjust a board definition for the application; do not copy addresses or
interrupt encodings from a different device.

The binding uses these properties:

| Property | Requirement | Purpose |
|---|---|---|
| `compatible = "arm,ethos-u"` | Required | Selects the Zephyr Ethos-U driver. |
| `reg` | Required | Defines the NPU register address and size. |
| `interrupts` or `interrupts-extended` | Required | Defines the NPU interrupt using the parent controller's encoding. |
| `status = "okay"` | Required to use the device | Enables the NPU instance. |
| `secure-enable` | Optional boolean | Configures secure operation when present. |
| `privilege-enable` | Optional boolean | Configures privileged operation when present. |
| `fast-memory-region` | Optional phandle | Selects a `zephyr,memory-region` used as spill cache for a Vela Dedicated-SRAM model on Ethos-U65 or Ethos-U85. |

The following is a structural example. Replace every `SOC_*` value with values
from the device integration and use the interrupt-cell format required by the
interrupt controller:

```devicetree
/ {
    ethosu_fast_memory: memory@SOC_ETHOSU_FAST_BASE {
        compatible = "zephyr,memory-region";
        reg = <SOC_ETHOSU_FAST_BASE SOC_ETHOSU_FAST_SIZE>;
    };

    ethosu: npu@SOC_ETHOSU_BASE {
        compatible = "arm,ethos-u";
        reg = <SOC_ETHOSU_BASE SOC_ETHOSU_SIZE>;
        interrupts = <SOC_ETHOSU_IRQ SOC_ETHOSU_IRQ_FLAGS>;
        status = "okay";

        /* Add only when required by the system configuration. */
        secure-enable;
        privilege-enable;

        /* Ethos-U65/U85 Dedicated-SRAM mode only. */
        fast-memory-region = <&ethosu_fast_memory>;
    };
};
```

If the board already declares the NPU, an application overlay typically only
needs to enable or override that node, for example:

```devicetree
&ethosu {
    status = "okay";
};
```

Omit `fast-memory-region` when the Vela model does not use Dedicated-SRAM mode.
When it is present, its address and size must match the fast staging/cache area
used by the linker and Vela configuration.

## Configure the application

Enable TFLM and Ethos-U in the application's `prj.conf`:

```text
CONFIG_CPP=y
CONFIG_TENSORFLOW_LITE_MICRO=y
CONFIG_ETHOS_U=y
```

Add stack, heap, C++ language-level, and logging options according to the
application's measured requirements. Keep the non-Vela model available only if
the application intentionally supports a CPU fallback; enabling
`CONFIG_ETHOS_U` requires the Vela-compiled model for NPU execution.

When the Arm GNU Toolchain is supplied through `vcpkg` and the application is
intended to use the toolchain's Newlib, select it explicitly:

```text
CONFIG_NEWLIB_LIBC=y
CONFIG_PICOLIBC=n
CONFIG_PICOLIBC_USE_MODULE=n
CONFIG_MINIMAL_LIBC=n
```

## Build with west

From the application directory, select an Ethos-U-capable board and perform a
pristine build:

```bash
west build -b <board-name> . --pristine auto
```

Use the board's normal runner to flash hardware or start a simulator. For
example, a board with a configured simulation runner can use:

```bash
west build -t run
```

Consult the board documentation when a separate FVP launch command or debug
configuration is required.

## Build in Arm Keil Studio for VS Code

The CMSIS Solution extension can display, build, and debug a Zephyr application
that uses the `west` build system. Install Zephyr in a central workspace, then
set these entries under the CMSIS Solution **Environment Variables** user or
workspace setting:

| Variable | Typical Linux/macOS value |
|---|---|
| `ZEPHYR_BASE` | `$HOME/zephyrproject/zephyr` |
| `PATH` | `$HOME/zephyrproject/.venv/bin` |
| `VIRTUAL_ENV` | `$HOME/zephyrproject/.venv` |

On Windows, use `$HOME/zephyrproject/.venv/Scripts` for the virtual-environment
`PATH`. Adjust every path to the actual Zephyr workspace. The corresponding VS
Code setting is:

```json
"cmsis-csolution.environmentVariables": {
    "ZEPHYR_BASE": "$HOME/zephyrproject/zephyr",
    "PATH": "$HOME/zephyrproject/.venv/bin",
    "VIRTUAL_ENV": "$HOME/zephyrproject/.venv"
}
```

Copy or clone the application into the CMSIS solution workspace. In
`csolution.yml`, point a west project entry at the application directory:

```yaml
projects:
  - west:
      app-path: ./my-zephyr-application
```

Do not add a `cproject.yml` for the Zephyr application. The `west` entry owns
that build. See
[Work with Zephyr applications](https://mdk-packs.github.io/vscode-cmsis-solution-docs/zephyr.html)
for the current extension setup and build/debug workflow.

## Corstone-300 FVP example

The
[Arm Ethos Zephyr Playground](https://github.com/Arm-Examples/Arm-Ethos-Zephyr-Playground)
contains an animal-classification application for the
`mps3/corstone300/fvp` board. It includes both a standard TFLM model and a
Vela-compiled model, and enables NPU acceleration with `CONFIG_ETHOS_U=y`.

After installing the Corstone-300 FVP and enabling the TFLM west module, clone
and build the example from the Zephyr tree:

```bash
cd "$ZEPHYR_BASE/samples/modules/tflite-micro"
git clone https://github.com/Arm-Examples/Arm-Ethos-Zephyr-Playground \
  animal_classification
cd animal_classification
west build -b mps3/corstone300/fvp . --pristine auto
source run.sh
```

The example's board overlay enlarges the simulated ITCM and DTCM regions for
the model and tensor arena. Those sizes are FVP example settings, not defaults
for physical targets. For a different board, replace the overlay, Vela target
configuration, memory placement, and runner with values validated for that
device.

## Verification checklist

- Confirm the build uses the intended board and devicetree node has
  `status = "okay"`.
- Confirm `CONFIG_TENSORFLOW_LITE_MICRO=y` and `CONFIG_ETHOS_U=y` in the final
  Zephyr configuration.
- Confirm the Vela target, system configuration, and memory mode match the
  physical NPU and devicetree memory mapping.
- Confirm the NPU interrupt is delivered and the driver completes an inference.
- Compare inference output against a known-good input before measuring
  performance.
- Measure stack, heap, tensor-arena, model, and optional spill-cache use on the
  final target.

## References

- [CMSIS Solution: Work with Zephyr applications](https://mdk-packs.github.io/vscode-cmsis-solution-docs/zephyr.html)
- [Zephyr `arm,ethos-u` devicetree binding](https://docs.zephyrproject.org/latest/build/dts/api/bindings/arm/arm%2Cethos-u.html)
- [Arm Ethos Zephyr Playground](https://github.com/Arm-Examples/Arm-Ethos-Zephyr-Playground)
