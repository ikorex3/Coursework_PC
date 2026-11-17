import os
import time
import threading
import requests
import matplotlib.pyplot as plt
URL = "http://localhost:8080"
os.makedirs("docs", exist_ok=True)

def save_plot(x, y, title, xlabel, ylabel, filename):
    plt.plot(x, y, marker="o")
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.xticks(x)
    plt.grid(True)
    plt.savefig(f"docs/{filename}")
    plt.clf()
threads_list = [1, 2, 4, 8, 12]
times = []
for t in threads_list:
    r = requests.post(f"{URL}/api/index", json={"threads": t}, timeout=1200).json()
    times.append(r["time_ms"])
    print(f"Threads: {t} | Time: {r['time_ms']} ms | Docs: {r['documents_indexed']}")
save_plot(threads_list, times, "Index Time vs Threads", "Threads", "Time (ms)", "benchmark_indexing.png")
clients_list = [1, 5, 10, 20, 50]
rps_list = []

def worker(count=10):
    for _ in range(count):
        requests.post(f"{URL}/api/search", json={"query": "space", "mode": "keyword"}, timeout=30)

for c in clients_list:
    start = time.time()
    workers = [threading.Thread(target=worker) for _ in range(c)]
    for w in workers:
        w.start()
    for w in workers:
        w.join()
    elapsed = time.time() - start
    rps = (c * 10) / elapsed
    rps_list.append(rps)
    print(f"Clients: {c} | Time: {elapsed:.2f}s | RPS: {rps:.1f}")
save_plot(clients_list, rps_list, "Search Throughput", "Clients", "Requests / sec", "benchmark_throughput.png")
