import json, pathlib, sys

root = pathlib.Path(sys.argv[1]).resolve()
out_file = root / 'compile_commands.json'

all_cmds = []

for json_file in root.rglob('compile_commands.json'):
    if json_file.resolve() == out_file.resolve():
        continue
    print('merging', json_file)
    with open(json_file, encoding='utf-8') as f:
        cmds = json.load(f)
    all_cmds.extend(cmds)

out_file.write_text(
    json.dumps(all_cmds, indent=2, ensure_ascii=False),
    encoding='utf-8'
)
print('wrote', out_file, 'with', len(all_cmds), 'entries')


