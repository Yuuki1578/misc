buffer = dict()
fail = 0

for k in range(0, 1024 * 50):
    buffer[k << 4] = k
    val = buffer.get(k << 4)
    if val is None:
        fail += 1
    else:
        print(f"[{k}] key: {k << 4}, value: {val}")
else:
    print(f"failed retrieve count: {fail}")
