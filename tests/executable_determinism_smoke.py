"""Build every linked executable in two separate trees and require identical bytes.

The source-to-EXE chain runs DOS tools whose unwritten stack bytes reach the
EXE header (CPE2X's reserved words). They are only reproducible when the
emulated CPU rate, not host load, schedules the timer interrupt. The second
tree has a longer path, so mount spelling and file locations are covered too.

Needs the `nix develop` build environment; reads no retail files.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO))

from scripts.kf.retail import IMAGE_KEYS  # noqa: E402

INPUTS = ('scripts', 'config', 'include', 'src', 'vendor')


def build(root: Path) -> dict[str, str]:
    root.mkdir(parents=True)
    for name in INPUTS:
        shutil.copytree(REPO / name, root / name,
                        ignore=shutil.ignore_patterns('__pycache__'))
    subprocess.run([sys.executable, '-m', 'scripts.kf.executable'], cwd=root, check=True,
                   env=dict(os.environ, PYTHONPATH=str(root)))
    outputs = {}
    for image in IMAGE_KEYS:
        # An image with no source units yet is skipped by the link step.
        if not (root / 'build/link' / image / (image.upper() + '.EXE')).is_file():
            continue
        for suffix in ('.EXE', '.CPE'):
            path = root / 'build/link' / image / (image.upper() + suffix)
            outputs[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    return outputs


def main() -> int:
    with tempfile.TemporaryDirectory(prefix='kf-determinism-') as directory:
        base = Path(directory)
        first = build(base / 'a')
        second = build(base / 'second-tree-with-a-longer-mount-path' / 'b')
    if set(first) != set(second):
        print('the two builds linked different images', file=sys.stderr)
        return 1
    if not first:
        print('no image has source units yet; nothing to compare')
        return 0
    differing = sorted(name for name in first if first[name] != second[name])
    for name in sorted(first):
        print(f'{name}: {first[name]}' + (f' != {second[name]}' if name in differing else ''))
    if differing:
        print(f'nondeterministic outputs: {", ".join(differing)}', file=sys.stderr)
        return 1
    print('all executables and native links are byte-identical across both builds')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
