import hashlib
import os
import pathlib
import shutil
import sys

def main() -> int:
    workspace = pathlib.Path(os.environ.get('GITHUB_WORKSPACE', '.')).resolve()
    configured = os.environ.get('CUBEX_XEX_OUTPUT')
    if not configured:
        print('CUBEX_XEX_OUTPUT is not configured; no XEX will be fabricated.', file=sys.stderr)
        return 2
    source = pathlib.Path(configured).resolve()
    if not source.is_file():
        print(f'Configured XEX output does not exist: {source}', file=sys.stderr)
        return 2
    if source != workspace / 'default.xex':
        shutil.copy2(source, workspace / 'default.xex')
    target = workspace / 'default.xex'
    size = target.stat().st_size
    with target.open('rb') as stream:
        magic = stream.read(4)
    if size < 4096 or magic != b'XEX2':
        print(f'Invalid XEX candidate: size={size}, magic={magic!r}; expected XEX2 container.', file=sys.stderr)
        return 3
    digest = hashlib.sha256(target.read_bytes()).hexdigest()
    print(f'Container check passed: {target}; size={size}; sha256={digest}')
    print('Note: container validation alone does not establish correct imports, runtime, or console boot.')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())