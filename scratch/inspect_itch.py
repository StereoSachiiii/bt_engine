with open("ex20101224.TEST_ITCH_50", "rb") as f:
    data = f.read(64)
    print(" ".join(f"{b:02x}" for b in data))
