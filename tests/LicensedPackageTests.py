import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
PACKAGE_SCRIPT = ROOT / "scripts" / "package-cubase.ps1"


class LicensedPackageScript(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.text = PACKAGE_SCRIPT.read_text(encoding="utf-8-sig")

    def test_uses_distinct_licensed_package_name(self):
        self.assertIn("Piano Auto Sustain 1.0 Cubase Windows x64 授权版", self.text)

    def test_includes_activation_helper_and_authorization_note(self):
        self.assertIn("License-Activation.ps1", self.text)
        self.assertIn("授权说明.txt", self.text)
        self.assertIn("AMU_PIANO_AUTO_SUSTAIN", self.text)

    def test_does_not_emit_source_archive(self):
        self.assertNotIn("Piano Auto Sustain 1.0 Source.zip", self.text)

    def test_writes_hash_manifest_after_all_payload_files(self):
        manifest = self.text.index("SHA256SUMS.txt")
        self.assertGreater(manifest, self.text.index("授权说明.txt"))
        self.assertGreater(manifest, self.text.index("License-Activation.ps1"))


if __name__ == "__main__":
    unittest.main()
