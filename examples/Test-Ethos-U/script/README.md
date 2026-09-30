# LiteRT model converter

`model-converter.py` wraps the Arm Ethos-U Vela compiler. It reads target settings
and model information from a generated `*.cbuild-mlops.yml` file and converts one
or more quantized LiteRT (TensorFlow Lite) input model files.

For the overall workflow and configuration format, see the CMSIS-Toolbox
[MLOps integration documentation](https://open-cmsis-pack.github.io/cmsis-toolbox/build-overview/#mlops-integration).

For every input model, the converter generates:

- a Vela-optimized `.tflite` model;
- a C source file containing the optimized model as a byte array; and
- a Markdown file containing the Vela report.

## Prerequisites

- Python 3.13 or newer
- Arm Ethos-U Vela available as `vela` on `PATH`
- PyYAML

Install the Python dependency:

```console
python -m pip install -r script/requirements.txt
```

Confirm that Vela is available:

```console
vela --version
```

## Basic usage

```console
python script/model-converter.py <project.cbuild-mlops.yml>
```

The path to the *.cbuild-mlops.yml file may be absolute or relative. Example:

```console
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml
```

### C source output directory

By default, each generated C source is written beside its input model. Use
`--out-dir` when the build system expects generated sources in a separate
directory:

```console
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml --out-dir source/model
```

An absolute path is used as given. A relative path is resolved from the
directory containing the `*.cbuild-mlops.yml` input file, independently of the
shell's current working directory. The converter creates the output directory
when necessary.

Only generated `*_model.c` files are redirected. The Vela-optimized
`*_vela.tflite` files and `VELA_SUMMARY.md` reports remain beside their input
models. If multiple input models would produce the same C filename, conversion
is rejected instead of overwriting one model with another.

## Using the converter from VS Code

The example contains a `.vscode.d/tasks.json` drop-in for the CMSIS Solution
extension. After opening or updating the solution, run **Tasks: Run Task** from
the Command Palette and select **Convert LiteRT models**. Choose the generated
MLOps configuration for U55, U65, or U85 when prompted.

If the task is not visible yet, run **Update Debug Tasks and Launch
Configurations** from the Command Palette. The extension merges the drop-in
task into the generated workspace `.vscode/tasks.json` file.

Generate the selected `*.cbuild-mlops.yml` file by setting up or building the
corresponding solution before running the conversion task.

## MLOps input

The generated `*.cbuild-mlops.yml` supplies the Vela configuration and model
selection:

```yaml
cbuild-mlops:
  vela:
    ini: Model/vela.ini
    options: >-
      --accelerator-config ethos-u55-128
      --system-config Ethos_U55_High_End_Embedded
      --memory-mode Shared_Sram
    misc: --verbose-all
  model:
    dir: Model
    name: tiny_cnn/tiny_cnn_int8.tflite
```

Paths are resolved relative to the directory containing the MLOps file.

The converter uses from the `*.cbuild-mlops.yml` file the following nodes:

- `vela.ini` as Vela's configuration file when present;
- `vela.options` for generated Vela arguments;
- `vela.misc` for optional additional Vela arguments;
- `model.dir` as the model working directory; and
- `model.name` to select one or more input models.

## Converting one model

Use a string-valued `model.name` for a single model:

```yaml
model:
  dir: Model
  name: tiny_cnn/tiny_cnn_int8.tflite
```

The model path is relative to `model.dir`.

## Converting multiple models

Use a sequence-valued `model.name` for multiple models:

```yaml
model:
  dir: Model
  name:
    - hello_world/hello_world_int8.tflite
    - tiny_cnn/tiny_cnn_int8.tflite
```

Models are converted in declared order and must be located below `model.dir`.

## Overwrite MLOps settings

Four optional arguments control outputs or override settings from the
`*.cbuild-mlops.yml` file:

```text
--system <name>    Vela system configuration
--memory <name>    Vela memory mode
--misc <options>   Complete miscellaneous option string
--out-dir <path>   Explicit path for generated C source files
```

Change the system or memory configuration:

```console
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml --system Ethos_U55_High_End_Embedded
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml --memory Shared_Sram
```

Replace all miscellaneous Vela options:

```console
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml --misc="--verbose-all --timing"
```

An empty value removes them:

```console
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml --misc=""
```

`--misc` replaces `vela.misc`; it does not append to it.

## Generated files

Given this input:

```text
Model/tiny_cnn/tiny_cnn_int8.tflite
```

the converter produces:

```text
Model/tiny_cnn/tiny_cnn_int8_vela.tflite
Model/tiny_cnn/tiny_cnn_model.c
Model/tiny_cnn/VELA_SUMMARY.md
```

The `_int8` suffix is removed when naming the C source. For `network.tflite`,
the outputs are `network_vela.tflite` and `network_model.c`.

The C source exports:

```c
const uint8_t tiny_cnn_int8_vela_tflite[];
const uint32_t tiny_cnn_int8_vela_tflite_len;
```

The array is aligned to 16 bytes and placed in the `ethos_model` section.

`VELA_SUMMARY.md` records the effective accelerator, system, and memory
configuration followed by Vela's report. Command-line overrides are reflected
in this file.

Outputs are prepared in temporary storage and replace destination files only
after Vela succeeds.

## Exit status and errors

- `0`: all models were converted.
- `1`: Vela or output generation failed.
- `2`: command-line or YAML configuration is invalid.

The converter validates configuration files, directories, and models before
starting Vela. Common errors include:

- Vela is missing from `PATH`;
- `model.dir`, a model, or the Vela INI file does not exist;
- `model.name` is missing, empty, or malformed; and
- a model path escapes `model.dir`.

## Running the tests

From the example directory:

```console
python -m unittest discover -s script_test -p "test_*.py" -v
```

The suite includes a real Vela conversion when `vela` is installed.
