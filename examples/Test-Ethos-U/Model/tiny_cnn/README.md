# Tiny CNN

A small quantized convolutional network that classifies 16 x 16 images into
four classes.

- **Input:** 256 `int8` values; the test image contains diagonal stripes.
- **Output:** four `int8` logits, expected `[-23, -32, 123, -92]`.
- **Validation:** exact output match and winning class index 2 (diagonal stripes).

`tiny_cnn_int8.tflite` is the original model; `tiny_cnn_int8_vela.tflite`
and `tiny_cnn_model.c` contain the Vela-compiled model. The supplied output
is compiled for Ethos-U85-256 with shared SRAM. Regenerate it before running
on U55 or changing the memory configuration.

See [VELA_SUMMARY.md](VELA_SUMMARY.md) for compilation and memory details.
Enable `MODEL_TINY_CNN` in the combined
[ML layer](../ML-MyModels.clayer.yml) to run its `TestModel_Int8` call in
[app_main.cpp](../../Source/app_main.cpp). See the
[integration guide](../../README.md#integrate-a-new-model) for conversion and
adding models.
