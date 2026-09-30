#!/usr/bin/env python3
# Copyright (c) 2026 Arm Limited (or its affiliates). All rights reserved.
# SPDX-License-Identifier: Apache-2.0
"""Convert one or more quantized LiteRT models for an Ethos-U target."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from typing import Any

import yaml


class ConfigurationError(Exception):
    """Invalid command-line or YAML configuration."""


def load_yaml(path: Path) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as stream:
            value = yaml.safe_load(stream)
    except OSError as exc:
        raise ConfigurationError(f"cannot read '{path}': {exc}") from exc
    except yaml.YAMLError as exc:
        raise ConfigurationError(f"invalid YAML in '{path}': {exc}") from exc
    if not isinstance(value, dict):
        raise ConfigurationError(f"'{path}' must contain a YAML mapping")
    return value


def require_mapping(parent: dict[str, Any], key: str, source: Path) -> dict[str, Any]:
    value = parent.get(key)
    if not isinstance(value, dict):
        raise ConfigurationError(f"'{source}': '{key}' must be a mapping")
    return value


def require_string(parent: dict[str, Any], key: str, source: Path) -> str:
    value = parent.get(key)
    if not isinstance(value, str) or not value.strip():
        raise ConfigurationError(f"'{source}': '{key}' must be a non-empty string")
    return value


def resolved(base: Path, value: str) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = base / path
    return path.resolve()


def below(path: Path, root: Path, description: str) -> Path:
    try:
        path.relative_to(root)
    except ValueError as exc:
        raise ConfigurationError(f"{description} escapes model.dir: '{path}'") from exc
    return path


def read_models(model: dict[str, Any], mlops_dir: Path, source: Path) -> tuple[Path, list[Path]]:
    model_dir = resolved(mlops_dir, require_string(model, "dir", source))
    if not model_dir.is_dir():
        raise ConfigurationError(f"model.dir is not a directory: '{model_dir}'")

    value = model.get("name")
    if isinstance(value, str) and value.strip():
        names = [value]
    elif (isinstance(value, list) and value
          and all(isinstance(item, str) and item.strip() for item in value)):
        names = value
    else:
        raise ConfigurationError(
            f"'{source}': 'model.name' must be a non-empty string or list of strings"
        )

    paths: list[Path] = []
    seen: set[Path] = set()
    for name in names:
        path = below(resolved(model_dir, name), model_dir, "model path")
        if path.suffix.lower() != ".tflite":
            raise ConfigurationError(f"model is not a .tflite file: '{path}'")
        if not path.is_file():
            raise ConfigurationError(f"model does not exist: '{path}'")
        if path in seen:
            raise ConfigurationError(f"model is listed more than once: '{path}'")
        seen.add(path)
        paths.append(path)
    return model_dir, paths


def split_options(value: Any, key: str, source: Path) -> list[str]:
    if value is None:
        return []
    if not isinstance(value, str):
        raise ConfigurationError(f"'{source}': '{key}' must be a string")
    try:
        return shlex.split(value, posix=os.name != "nt")
    except ValueError as exc:
        raise ConfigurationError(f"'{source}': cannot parse '{key}': {exc}") from exc


def replace_option(options: list[str], option: str, value: str) -> list[str]:
    result: list[str] = []
    replaced = False
    index = 0
    while index < len(options):
        item = options[index]
        if item == option:
            if index + 1 >= len(options):
                raise ConfigurationError(f"Vela option '{option}' has no value")
            if not replaced:
                result.extend((option, value))
                replaced = True
            index += 2
            continue
        if item.startswith(option + "="):
            if not replaced:
                result.extend((option, value))
                replaced = True
            index += 1
            continue
        result.append(item)
        index += 1
    if not replaced:
        result.extend((option, value))
    return result


def option_value(options: list[str], option: str) -> str:
    for index, item in enumerate(options):
        if item == option and index + 1 < len(options):
            return options[index + 1]
        if item.startswith(option + "="):
            return item.split("=", 1)[1]
    return "not specified"


def c_identifier(stem: str) -> str:
    identifier = re.sub(r"\W", "_", stem)
    if not identifier or identifier[0].isdigit():
        identifier = "_" + identifier
    return identifier


def command_line(command: list[str]) -> str:
    if os.name == "nt":
        return subprocess.list2cmdline(command)
    return shlex.join(command)


def emit_c(path: Path, symbol: str, payload: bytes) -> None:
    rows = []
    for index in range(0, len(payload), 12):
        row = ", ".join(f"0x{byte:02x}" for byte in payload[index:index + 12])
        rows.append(f"  {row},")
    content = (
        "/* Generated by script/model-converter.py -- do not edit by hand. */\n"
        "\n"
        "#include <stdint.h>\n\n"
        f"const uint8_t {symbol}[]\n"
        "    __attribute__((aligned(16), section(\"ethos_model\"))) = {\n"
        + "\n".join(rows)
        + "\n};\n"
        f"const uint32_t {symbol}_len = {len(payload)};\n"
    )
    path.write_text(content, encoding="utf-8", newline="\n")


def summary_text(model_name: str, options: list[str], report: str) -> str:
    return (
        f"# Vela report: {model_name}\n\n"
        f"- accelerator: `{option_value(options, '--accelerator-config')}`\n"
        f"- system config: `{option_value(options, '--system-config')}`\n"
        f"- memory mode: `{option_value(options, '--memory-mode')}`\n\n"
        f"```\n{report.strip()}\n```\n"
    )


def replace_file(staged: Path, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = destination.with_name(destination.name + ".tmp")
    shutil.copyfile(staged, temporary)
    os.replace(temporary, destination)


def convert_model(
    model_path: Path,
    vela_ini: Path | None,
    options: list[str],
    out_dir: Path | None,
) -> None:
    input_stem = model_path.stem
    model_name = input_stem[:-5] if input_stem.endswith("_int8") else input_stem
    symbol = c_identifier(input_stem + "_vela_tflite")
    model_output_dir = model_path.parent

    with tempfile.TemporaryDirectory(prefix="model-converter-") as temporary:
        stage = Path(temporary)
        command = ["vela"]
        if vela_ini is not None:
            command.extend(("--config", str(vela_ini)))
        command.extend(options)
        command.extend((str(model_path), "--output-dir", str(stage)))

        print(f"Converting {model_path}")
        print(
            "Executing: "
            f"{command_line(command)}"
        )
        try:
            process = subprocess.run(command, text=True, capture_output=True)
        except OSError as exc:
            raise RuntimeError(f"cannot run Vela: {exc}") from exc

        report = process.stdout
        if process.stdout:
            print(process.stdout, end="" if process.stdout.endswith("\n") else "\n")
        if process.stderr:
            print(process.stderr, file=sys.stderr,
                  end="" if process.stderr.endswith("\n") else "\n")
        if process.returncode:
            raise RuntimeError(
                f"Vela failed for '{model_path}' with exit status {process.returncode}"
            )

        vela_model = stage / f"{input_stem}_vela.tflite"
        if not vela_model.is_file():
            raise RuntimeError(f"Vela did not produce expected file '{vela_model.name}'")

        c_file = stage / f"{model_name}_model.c"
        emit_c(c_file, symbol, vela_model.read_bytes())
        summary = stage / "VELA_SUMMARY.md"
        summary.write_text(
            summary_text(model_name, options, report),
            encoding="utf-8",
            newline="\n",
        )

        replace_file(vela_model, model_output_dir / vela_model.name)
        replace_file(
            c_file,
            (out_dir if out_dir is not None else model_output_dir)
            / c_file.name,
        )
        replace_file(summary, model_output_dir / summary.name)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert .tflite model files with parameters provided by *.cbuild-mlops.yml"
    )
    parser.add_argument("cbuild_mlops", type=Path, help="generated *.cbuild-mlops.yml file")
    parser.add_argument("--system", help="override the Vela system configuration")
    parser.add_argument("--memory", help="override the Vela memory mode")
    parser.add_argument("--misc", help="replace miscellaneous Vela options")
    parser.add_argument(
        "--out-dir",
        type=Path,
        help="explicit path for generated C source files",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    source = args.cbuild_mlops.resolve()
    document = load_yaml(source)
    root = require_mapping(document, "cbuild-mlops", source)
    vela = require_mapping(root, "vela", source)
    model = require_mapping(root, "model", source)

    options = split_options(vela.get("options"), "cbuild-mlops.vela.options", source)
    misc_value = args.misc if args.misc is not None else vela.get("misc")
    misc = split_options(misc_value, "cbuild-mlops.vela.misc", source)
    if any(item == "--system-config" or item.startswith("--system-config=")
           or item == "--memory-mode" or item.startswith("--memory-mode=")
           for item in misc):
        raise ConfigurationError(
            "misc options must not contain --system-config or --memory-mode"
        )
    options.extend(misc)

    if args.system is not None:
        options = replace_option(options, "--system-config", args.system)
    if args.memory is not None:
        options = replace_option(options, "--memory-mode", args.memory)

    ini_value = vela.get("ini")
    vela_ini = None
    if ini_value is not None:
        if not isinstance(ini_value, str) or not ini_value.strip():
            raise ConfigurationError(
                f"'{source}': 'cbuild-mlops.vela.ini' must be a non-empty string"
            )
        vela_ini = resolved(source.parent, ini_value)
        if not vela_ini.is_file():
            raise ConfigurationError(f"Vela INI file does not exist: '{vela_ini}'")

    _, models = read_models(model, source.parent, source)
    out_dir = None
    if args.out_dir is not None:
        out_dir = resolved(source.parent, os.fspath(args.out_dir))

        output_names = []
        for model_path in models:
            stem = model_path.stem
            model_name = stem[:-5] if stem.endswith("_int8") else stem
            output_names.append(f"{model_name}_model.c")
        duplicates = sorted(
            name for name in set(output_names) if output_names.count(name) > 1
        )
        if duplicates:
            raise ConfigurationError(
                "models produce duplicate C output names: " + ", ".join(duplicates)
            )

    for model_path in models:
        convert_model(model_path, vela_ini, options, out_dir)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except ConfigurationError as exc:
        print(f"model-converter: configuration error: {exc}", file=sys.stderr)
        sys.exit(2)
    except RuntimeError as exc:
        print(f"model-converter: {exc}", file=sys.stderr)
        sys.exit(1)
