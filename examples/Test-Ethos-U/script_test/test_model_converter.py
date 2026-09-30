#!/usr/bin/env python3
# Copyright (c) 2026 Arm Limited (or its affiliates). All rights reserved.
# SPDX-License-Identifier: Apache-2.0

"""Tests for model-converter.py."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

import yaml


EXAMPLE = Path(__file__).resolve().parent.parent
SCRIPT = EXAMPLE / "script" / "model-converter.py"
SPEC = importlib.util.spec_from_file_location("model_converter", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
converter = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = converter
SPEC.loader.exec_module(converter)


class ModelConverterTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.model_dir = self.root / "Model"
        self.model_dir.mkdir()

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def write_yaml(self, path: Path, value: object) -> None:
        path.write_text(yaml.safe_dump(value, sort_keys=False), encoding="utf-8")

    def touch_model(self, name: str, content: bytes = b"TFL3") -> Path:
        path = self.model_dir / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
        return path

    def test_single_model(self) -> None:
        expected = self.touch_model("one/model.tflite")
        model = {"dir": "Model", "name": "one/model.tflite"}
        _, models = converter.read_models(model, self.root, self.root / "input.yml")
        self.assertEqual(models, [expected.resolve()])

    def test_multiple_models(self) -> None:
        first = self.touch_model("one/a.tflite")
        second = self.touch_model("two/b.tflite")
        model = {
            "dir": "Model",
            "name": ["one/a.tflite", "two/b.tflite"],
        }
        _, models = converter.read_models(model, self.root, self.root / "input.yml")
        self.assertEqual(models, [first.resolve(), second.resolve()])

    def test_model_may_not_escape_model_directory(self) -> None:
        outside = self.root / "outside.tflite"
        outside.write_bytes(b"TFL3")
        model = {"dir": "Model", "name": "../outside.tflite"}
        with self.assertRaisesRegex(converter.ConfigurationError, "escapes model.dir"):
            converter.read_models(model, self.root, self.root / "input.yml")

    def test_replace_option_supports_both_forms(self) -> None:
        options = [
            "--system-config=old",
            "--memory-mode",
            "old-memory",
            "--verbose-all",
        ]
        options = converter.replace_option(options, "--system-config", "new")
        options = converter.replace_option(options, "--memory-mode", "new-memory")
        self.assertEqual(
            options,
            [
                "--system-config",
                "new",
                "--memory-mode",
                "new-memory",
                "--verbose-all",
            ],
        )

    def test_cli_overrides_and_replaces_misc(self) -> None:
        model = self.touch_model("network.tflite")
        ini = self.root / "vela.ini"
        ini.write_text("", encoding="utf-8")
        mlops = self.root / "input.cbuild-mlops.yml"
        self.write_yaml(
            mlops,
            {
                "cbuild-mlops": {
                    "vela": {
                        "ini": "vela.ini",
                        "options": (
                            "--accelerator-config ethos-u55-128 "
                            "--system-config old-system --memory-mode old-memory"
                        ),
                        "misc": "--verbose-all",
                    },
                    "model": {"dir": "Model", "name": model.name},
                }
            },
        )

        argv = [
            str(SCRIPT),
            str(mlops),
            "--system",
            "new-system",
            "--memory",
            "new-memory",
            "--misc=--timing",
        ]
        with mock.patch.object(sys, "argv", argv), mock.patch.object(
            converter, "convert_model"
        ) as convert:
            self.assertEqual(converter.main(), 0)

        passed_options = convert.call_args.args[2]
        self.assertEqual(
            passed_options,
            [
                "--accelerator-config",
                "ethos-u55-128",
                "--system-config",
                "new-system",
                "--memory-mode",
                "new-memory",
                "--timing",
            ],
        )

    def test_conversion_generates_all_related_files(self) -> None:
        source = self.touch_model("tiny_cnn_int8.tflite")
        options = [
            "--accelerator-config",
            "ethos-u55-128",
            "--system-config",
            "test-system",
            "--memory-mode",
            "test-memory",
        ]

        def fake_run(command: list[str], **_: object) -> subprocess.CompletedProcess[str]:
            output_dir = Path(command[command.index("--output-dir") + 1])
            (output_dir / "tiny_cnn_int8_vela.tflite").write_bytes(b"VELA")
            return subprocess.CompletedProcess(command, 0, "Vela report\n", "")

        with mock.patch.object(converter.subprocess, "run", side_effect=fake_run):
            converter.convert_model(source, None, options, None)

        self.assertEqual(
            (self.model_dir / "tiny_cnn_int8_vela.tflite").read_bytes(), b"VELA"
        )
        c_source = (self.model_dir / "tiny_cnn_model.c").read_text(encoding="utf-8")
        self.assertIn("tiny_cnn_int8_vela_tflite[]", c_source)
        self.assertIn('section("ethos_model")', c_source)
        summary = (self.model_dir / "VELA_SUMMARY.md").read_text(encoding="utf-8")
        self.assertIn("# Vela report: tiny_cnn", summary)
        self.assertIn("test-system", summary)
        self.assertIn("Vela report", summary)

    def test_out_dir_redirects_only_c_source(self) -> None:
        source = self.touch_model("one/network.tflite")
        out_dir = self.root / "generated" / "model"

        def fake_run(command: list[str], **_: object) -> subprocess.CompletedProcess[str]:
            vela_out = Path(command[command.index("--output-dir") + 1])
            (vela_out / "network_vela.tflite").write_bytes(b"VELA")
            return subprocess.CompletedProcess(command, 0, "Vela report\n", "")

        with mock.patch.object(converter.subprocess, "run", side_effect=fake_run):
            converter.convert_model(source, None, [], out_dir)

        self.assertTrue((out_dir / "network_model.c").is_file())
        self.assertFalse((source.parent / "network_model.c").exists())
        self.assertTrue((source.parent / "network_vela.tflite").is_file())
        self.assertTrue((source.parent / "VELA_SUMMARY.md").is_file())

    def test_failed_vela_does_not_create_outputs(self) -> None:
        source = self.touch_model("network.tflite")
        failed = subprocess.CompletedProcess(["vela"], 1, "", "failure")
        with mock.patch.object(converter.subprocess, "run", return_value=failed):
            with self.assertRaisesRegex(RuntimeError, "Vela failed"):
                converter.convert_model(source, None, [], None)
        self.assertFalse((self.model_dir / "network_vela.tflite").exists())
        self.assertFalse((self.model_dir / "network_model.c").exists())
        self.assertFalse((self.model_dir / "VELA_SUMMARY.md").exists())

    @unittest.skipUnless(shutil.which("vela"), "Vela is not installed")
    def test_real_vela_conversion(self) -> None:
        example = EXAMPLE / "Model"
        source = self.model_dir / "tiny_cnn_int8.tflite"
        ini = self.root / "vela.ini"
        shutil.copyfile(example / "tiny_cnn" / source.name, source)
        shutil.copyfile(example / "vela.ini", ini)

        converter.convert_model(
            source,
            ini,
            [
                "--accelerator-config",
                "ethos-u55-128",
                "--system-config",
                "Ethos_U55_High_End_Embedded",
                "--memory-mode",
                "Shared_Sram",
            ],
            None,
        )

        self.assertTrue((self.model_dir / "tiny_cnn_int8_vela.tflite").is_file())
        self.assertTrue((self.model_dir / "tiny_cnn_model.c").is_file())
        self.assertTrue((self.model_dir / "VELA_SUMMARY.md").is_file())


if __name__ == "__main__":
    unittest.main()
