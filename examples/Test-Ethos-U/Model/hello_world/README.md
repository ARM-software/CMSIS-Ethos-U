# Hello World

A small quantized neural network that approximates `sin(x)`.

- **Input/output:** one `int8` value each.
- **Test vector:** input `-55` (about 1.794 radians), expected output `124`.
- **Validation:** exact output match.

`hello_world_int8.tflite` is the original model; `hello_world_int8_vela.tflite`
and `hello_world_model.c` contain the Vela-compiled model. The supplied output
is compiled for Ethos-U85-256 with shared SRAM. Regenerate it before running
on U55 or changing the memory configuration.

See [VELA_SUMMARY.md](VELA_SUMMARY.md) for compilation and memory details.
Enable `MODEL_HELLO_WORLD` in the combined
[ML layer](../ML-MyModels.clayer.yml) to run its `TestModel_Int8` call in
[app_main.cpp](../../Source/app_main.cpp). See the
[integration guide](../../README.md#integrate-a-new-model) for conversion and
adding models.
