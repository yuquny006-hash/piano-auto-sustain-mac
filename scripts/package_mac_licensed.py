"""Mac candidate packager. Must run on macOS after a licensed build."""
import argparse
import json
import pathlib
import platform
import plistlib
import shutil
import subprocess
import tempfile
from package_portable import check_binary, write_manifest, verify_manifest, digest, ROOT

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--build',required=True);p.add_argument('--output',required=True);p.add_argument('--public-key',required=True)
    a=p.parse_args()
    if platform.system()!='Darwin': raise RuntimeError('Native macOS build required')
    build=pathlib.Path(a.build).resolve();out=pathlib.Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
    cache=(build/'CMakeCache.txt').read_text()
    if 'PAS_MAC_PUBLIC_KEY_HEX:STRING='+a.public_key not in cache: raise RuntimeError('Build public key mismatch')
    name='Piano-Auto-Sustain-1.0-macOS-Universal-Licensed-CANDIDATE'
    archive=out/(name+'.zip')
    if archive.exists(): raise RuntimeError('Refusing to overwrite an existing build')
    with tempfile.TemporaryDirectory(dir=out) as temp:
        root=pathlib.Path(temp)/name;root.mkdir()
        for relative in ('BassPedal_artefacts/Release/VST3/Piano Auto Sustain.vst3','BassPedalAU_artefacts/Release/AU/Piano Auto Sustain MIDI FX.component'):
            src=build/relative
            info=plistlib.loads((src/'Contents/Info.plist').read_bytes())
            check_binary(src/'Contents/MacOS'/info['CFBundleExecutable'],'macOS','Universal')
            subprocess.run(['codesign','--verify','--deep','--strict',str(src)],check=True)
            shutil.copytree(src,root/src.name,symlinks=True)
        for source,dest in [('mac-licensed-install.sh','Install.command'),('mac-licensed-uninstall.sh','Uninstall.command')]:
            target=root/dest;shutil.copy2(ROOT/'scripts'/source,target);target.chmod(0o755)
        shutil.copy2(ROOT/'docs/MAC-LICENSED-GUIDE.md',root/'Mac使用说明.md')
        shutil.copy2(ROOT/'docs/MAC-ACCEPTANCE.md',root/'Mac验收清单.md')
        shutil.copytree(ROOT/'licenses',root/'licenses')
        (root/'BUILD-INFO.json').write_text(json.dumps(dict(product='Piano Auto Sustain',version='1.0',platform='macOS-Universal',authorization='Ed25519 machine-bound',publicKey=a.public_key,mac_signing='ad-hoc; not notarized',daw_acceptance='pending'),indent=2))
        write_manifest(root);verify_manifest(root)
        subprocess.run(['ditto','-c','-k','--sequesterRsrc','--keepParent',str(root),str(archive)],check=True)
    archive.with_suffix('.zip.sha256').write_text(digest(archive)+'  '+archive.name+'\n')
    print(archive)

if __name__=='__main__': main()
