"""Fault-injection test of shell transaction; macOS signature/quarantine APIs are mocked.
Does not claim Gatekeeper or macOS installation compatibility.
"""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
BASH=os.environ.get('PAS_TEST_BASH','bash')

class InstallerTests(unittest.TestCase):
    def run_case(self,fault):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp); package=root/'package';package.mkdir();home=root/'home'
            entries=[('VST3','Piano Auto Sustain.vst3'),('Components','Piano Auto Sustain MIDI FX.component')]
            for folder,name in entries:
                old=home/'Library/Audio/Plug-Ins'/folder/name;old.mkdir(parents=True);(old/'payload').write_text('old')
                new=package/name;new.mkdir();(new/'payload').write_text('new')
            (package/'SHA256SUMS.txt').write_text('0'*64+'  harmless\n')
            prelude='''
uname() { echo Darwin; }
shasum() { [[ "$FAULT" != hash ]]; }
codesign() { return 0; }
xattr() { [[ "$FAULT" != quarantine ]]; }
ditto() { cp -R "$1" "$2"; }
mv() {
  if [[ ( "$FAULT" == second || "$FAULT" == restore ) && "$1" == */new && "$2" == */Components/* ]]; then return 1; fi
  if [[ "$FAULT" == restore && "$1" == */previous && "$2" == */VST3/* ]]; then return 1; fi
  command mv "$@"
}
'''
            script=(ROOT/'scripts/mac-licensed-install.sh').read_text(encoding='utf8').replace('/usr/bin/ditto','ditto')
            script=script.replace('set -euo pipefail','set -euo pipefail\n'+prelude,1)
            path=package/'Install.command';path.write_text(script,encoding='utf8',newline='\n')
            env=dict(os.environ,HOME=home.as_posix(),FAULT=fault)
            run=subprocess.run([BASH,str(path)],env=env,capture_output=True,text=True,encoding='utf8',errors='replace')
            if fault: self.assertNotEqual(run.returncode,0,run.stdout+run.stderr)
            else: self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            for folder,name in entries:
                if fault=='restore' and folder=='VST3':
                    self.assertIn('RESTORE INCOMPLETE',run.stdout)
                    backups=list((home/'Library/Audio/Plug-Ins/VST3').glob('.PAS-backup-*/previous/payload'))
                    self.assertEqual(len(backups),1)
                    self.assertEqual(backups[0].read_text(),'old')
                    continue
                self.assertEqual((home/'Library/Audio/Plug-Ins'/folder/name/'payload').read_text(),'old' if fault else 'new',run.stdout+run.stderr)
    def test_success(self):self.run_case('')
    def test_failed_checksum_preserves_both(self):self.run_case('hash')
    def test_failed_quarantine_preserves_both(self):self.run_case('quarantine')
    def test_second_install_failure_rolls_back_first(self):self.run_case('second')
    def test_rollback_failure_preserves_backup_and_reports(self):self.run_case('restore')

if __name__=='__main__':unittest.main()
