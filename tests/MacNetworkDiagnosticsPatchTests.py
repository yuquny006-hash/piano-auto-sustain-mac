import importlib.util
from pathlib import Path
import tempfile
import unittest
spec = importlib.util.spec_from_file_location("patcher", Path(__file__).resolve().parents[1] / "scripts/patch_juce_mac_network_diagnostics.py")
patcher = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patcher)

class PatchTests(unittest.TestCase):
    def test_patch_is_idempotent_and_preserves_completion(self):
        original = "prefix\n" + patcher.ANCHOR + "\n  state = finished;\n}\nsuffix"
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "network.mm"
            path.write_text(original, encoding="utf-8")
            patcher.patch(path)
            result = path.read_text(encoding="utf-8")
            self.assertEqual(result, original.replace(patcher.ANCHOR, patcher.REPLACEMENT))
            patcher.patch(path)
            self.assertEqual(result, path.read_text(encoding="utf-8"))
    def test_unexpected_sdk_is_rejected_without_write(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "network.mm"
            path.write_text("unsupported", encoding="utf-8")
            with self.assertRaises(ValueError): patcher.patch(path)
            self.assertEqual(path.read_text(), "unsupported")
    def test_logging_does_not_include_sensitive_payload(self):
        for forbidden in ("HTTPBody", "localizedDescription", "absoluteString", "licenseKey", "machineHash", "signature"):
            self.assertNotIn(forbidden, patcher.REPLACEMENT)
        self.assertIn('license.amumusic.com', patcher.REPLACEMENT)
        self.assertIn('/api/activate', patcher.REPLACEMENT)

if __name__ == "__main__": unittest.main()
