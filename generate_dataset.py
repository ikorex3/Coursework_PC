import os
from datasets import load_dataset

OUT_DIR = "data"
TARGET_BYTES = 2 * 1024 * 1024 * 1024

os.makedirs(OUT_DIR, exist_ok=True)

dataset = load_dataset("wikimedia/wikipedia", "20231101.en", split="train", streaming=True)

total_bytes = 0
count = 0

for row in dataset:
    text = row["text"]
    if len(text) < 31000:
        continue

    content = f"{row['title']}\n\n{text}"
    count += 1

    file_path = os.path.join(OUT_DIR, f"wiki_{count:06d}.txt")
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(content)

    total_bytes += len(content.encode("utf-8"))
    if count % 1000 == 0:
        gb = total_bytes / (1024 ** 3)
        print(f"Downloaded: {count} files ({gb:.2f} GB)")

    if total_bytes >= TARGET_BYTES:
        break

print(f"Done: {count} files, {total_bytes / (1024 ** 3):.2f} GB")
