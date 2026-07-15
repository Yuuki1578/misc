buffer = dict()
fail = 0

for k in range(0, 1024 * 100):
    key = str(k << 16)
    buffer[key] = k
    val = buffer.get(key)
    if val is None:
        fail += 1
    else:
        print(f'[{k}] key: "{key}", value: {val}')
else:
    print(f"failed retrieve count: {fail}")
