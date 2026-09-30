# CMSIS MLOps LiteRT model converter specification

## Purpose

`model-converter.py` wraps the Ethos-U Vela compiler. It converts one or more
quantized LiteRT models (TensorFlow Lite) and generates their integration
files. A generated `*.cbuild-mlops.yml` file supplies model names, paths, and
Vela configuration.

The surrounding workflow and generated configuration are defined by the
CMSIS-Toolbox [MLOps integration documentation](https://open-cmsis-pack.github.io/cmsis-toolbox/build-overview/#mlops-integration).

## Location

```text
examples/Test-Ethos-U/script/model-converter.py
```

## Invocation

```console
python script/model-converter.py <cbuild-mlops.yml>
```

The MLOps file is a required positional argument. The following options are
independent and optional:

```text
--system <name>    Override the Vela system configuration.
--memory <name>    Override the Vela memory mode.
--misc <options>   Replace the miscellaneous Vela options.
--out-dir <path>   Explicit path for generated C source files.
```

Examples:

```console
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml \
  --system Ethos_U55_High_End_Embedded --memory Shared_Sram
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml \
  --misc="--verbose-all --timing"
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml \
  --out-dir source/model
```

## Input from `*.cbuild-mlops.yml`

The script reads these nodes:

```text
cbuild-mlops.vela.ini
cbuild-mlops.vela.options
cbuild-mlops.vela.misc
cbuild-mlops.model.dir
cbuild-mlops.model.name
```

Paths in the generated file are relative to the directory containing the
`*.cbuild-mlops.yml` file.

`model.dir` is the working directory for model conversion. It contains the
input models and receives the Vela output.

`model.name` selects the models to convert and shall be either:

- a non-empty string selecting one model; or
- a non-empty list of strings selecting multiple models.

Each model path is relative to `model.dir`. Lists are processed in their
declared order. For example:

```yaml
model:
  name:
    - hello_world/hello_world_int8.tflite
    - tiny_cnn/tiny_cnn_int8.tflite
```

## Vela settings and overrides

By default, the complete Vela configuration comes from
`cbuild-mlops.vela.options`, `cbuild-mlops.vela.misc`, and
`cbuild-mlops.vela.ini`.

The script tokenizes `vela.options` and `vela.misc` with `shlex.split` and
invokes Vela with a subprocess argument list and `shell=False`. Miscellaneous
Vela options are passed through unchanged and in their specified order.

`--system` replaces the value of `--system-config` in `vela.options`.
`--memory` replaces the value of `--memory-mode`. Both `--option value` and
`--option=value` forms are supported. If an overridden option is absent, the
script appends it in this form:

```text
--system-config <value>
--memory-mode <value>
```

`--misc` replaces the complete value of `cbuild-mlops.vela.misc` for the
current invocation; it does not append to it. An explicitly empty value removes
all miscellaneous options:

```console
python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml --misc=""
```

A miscellaneous option string shall not contain `--system-config` or
`--memory-mode`; these settings belong to their dedicated fields and command
line options.

Overrides affect only the current invocation. The script never modifies the
generated `*.cbuild-mlops.yml` file.

## Conversion

For every selected model, the script invokes the equivalent of:

```console
vela --config <vela.ini> <vela.options> <model> --output-dir <temporary-directory>
```

The input path is resolved below `model.dir`. The script rejects a path that
escapes this directory. It accepts any `.tflite` filename; an `_int8` suffix is
not required.

Before invoking Vela, the script validates the MLOps YAML, Vela INI file, model
directory, and every input model. It stops with a nonzero status when
validation or conversion fails, while preserving Vela output for diagnosis.

## Generated files

For every input model, the converter produces:

```text
<input-stem>_vela.tflite
<model-name>_model.c
VELA_SUMMARY.md
```

For `tiny_cnn/tiny_cnn_int8.tflite`, these files are:

```text
tiny_cnn_int8_vela.tflite
tiny_cnn_model.c
VELA_SUMMARY.md
```

`model-name` is the input stem with a trailing `_int8` removed. Without that
suffix, the input stem is used unchanged.

By default, all three files are written beside the input model. When
`--out-dir` is specified, only the generated C source is written to that
directory. The optimized flatbuffer and Vela summary remain beside the input
model.

An absolute `--out-dir` path is used as given. A relative path is resolved from
the directory containing the `*.cbuild-mlops.yml` file, not from the process
working directory. The script creates the directory when required. If multiple
inputs map to the same generated C filename, the script rejects the
configuration instead of overwriting an output.

The C source embeds the Vela output using:

```c
const uint8_t <input-stem>_vela_tflite[];
const uint32_t <input-stem>_vela_tflite_len;
```

The byte array is aligned to 16 bytes and placed in the `ethos_model` section:

```c
__attribute__((aligned(16), section("ethos_model")))
```

The source includes `<stdint.h>`, uses LF line endings, and contains a
generated-file notice and Apache-2.0 SPDX identifier.

`VELA_SUMMARY.md` contains the model name, effective accelerator, system
configuration, memory mode, and Vela textual report in a fenced code block.
Effective values include `--system` and `--memory` overrides.

Vela first writes into temporary storage. Only after successful conversion and
output generation does the script replace destination files. A failure does
not leave a partial flatbuffer, C source, or summary.

## Dependencies

The script requires:

- Python 3.10 or newer;
- PyYAML; and
- a Vela executable on `PATH`.

It works on Windows, Linux, and macOS and uses `pathlib` for path handling. It
does not import LiteRT or TensorFlow because it operates directly on existing
quantized `.tflite` files.

## Test-Ethos-U example

`Test-Ethos-U55.csolution.yml` illustrates the input that CMSIS-Toolbox copies
to the generated `*.cbuild-mlops.yml` file:

```yaml
model:
  dir: Model
  name:
    - hello_world/hello_world_int8.tflite
    - tiny_cnn/tiny_cnn_int8.tflite
```

The example uses a sequence because it always converts both models. Other
projects may use a string-valued `name` when converting one model.

## Acceptance criteria

- A string-valued `model.name` converts one model.
- A sequence-valued `model.name` converts every listed model in one invocation.
- Each model produces a Vela flatbuffer, C array, and Vela summary.
- Default Vela settings come entirely from `*.cbuild-mlops.yml`.
- `--system` and `--memory` override only their corresponding Vela settings.
- Miscellaneous Vela options are preserved by default.
- `--misc` replaces rather than appends to miscellaneous Vela options.
- `--out-dir` redirects only generated C source files.
- The generated `*.cbuild-mlops.yml` file is never modified.
- Relative paths work when invoked from a different working directory.
- Missing files, invalid YAML, unsafe paths, and Vela failures are reported
  clearly and return a nonzero status.
