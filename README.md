# Parallel Search Engine with LLM Query Expansion

## Архітектура та компоненти

* **`ThreadPool`** — пул робочих потоків із чергою задач і синхронізацією (`mutex`, `condition_variable`)
* **`IndexTable`** — інвертований індекс (власна хеш-таблиця `HashMap` + `shared_mutex`) з ранжуванням за **TF-IDF**
* **`DataIndexer`** — рекурсивний обхід директорії датасету та паралельна індексація файлів через пул потоків
* **`OllamaClient`** — інтеграція з Ollama (`/api/generate`) для розширення запитів синонімами та ключовими словами
* **`SearchApp`** — HTTP/WebSocket сервер на базі **Crow** (порт `8080`)
* **Клієнти:** мінімалістичний **Web UI** (`web/index.html`) та консольний **CLI-клієнт** (`cli.cpp` $\to$ `./search`)

UML-діаграми у папці [`docs/`]

---

## Збирання та запуск

### 1. Підготовка датасету
Скрипт `generate_dataset.py` завантажує статті англійської Вікіпедії та зберігає їх як окремі `.txt` файли у папку `data/`:
```bash
pip install datasets
python generate_dataset.py
```

### 2. Запуск системи через Docker Compose
```bash
docker compose up --build -d
```

### 3. Завантаження моделі в Ollama (один раз)
```bash
docker exec -it coursework_ollama ollama pull llama3.2:1b
```
Веб-інтерфейс доступний за адресою: **http://localhost:8080**
---

### Web UI
`http://localhost:8080` у браузері:
1. Обрати кількість потоків і натиснути **Build Index**.
2. Ввести запит, під час роботи будуть виводитись етапи WebSocket-пайплайна (`query_expansion` $\to$ `keyword_search` $\to$ `done`), розширені ключові слова і знайдені документи.

### CLI-клієнт
Пошук за ключовими словами (`mode: keyword`):
```bash
docker exec -it coursework_server ./search "запит англійською"
```
Пошук із розширенням запиту через Ollama (`mode: expanded`):
```bash
docker exec -it coursework_server ./search "запит англійською" --expanded
```

---

## Специфікація API

### REST API

* **`GET /api/health`** — перевірка стану сервера та Ollama
  * Відповідь (`200 OK`): `{"status": string, "ollama": boolean}` (напр., `{"status": "ok", "ollama": true}`)

* **`POST /api/index`** — паралельна побудова інвертованого індексу
  * Запит: `{"threads": int}` (за замовчуванням `4`)
  * Відповідь (`200 OK`): `{"status": string, "documents_indexed": int, "time_ms": int}`

* **`POST /api/search`** — пошук документів (`mode`: `"keyword"` або `"expanded"`)
  * Запит: `{"query": string, "mode": string}`
  * Відповідь (`200 OK`): `{"results": [{"doc_id": string, "score": number, "snippet": string}], "time_ms": int}`

* **`GET /api/stats`** — статистика побудованого індексу
  * Відповідь (`200 OK`): `{"documents": int, "index_build_time_ms": int, "threads_used": int}`

### WebSocket `/ws/search`

* **Запит клієнта (JSON):** `{"query": string}`
* **Повідомлення сервера (JSON, послідовно 3 етапи):**
  1. `{"stage": "query_expansion", "data": {"original": string, "expanded_terms": [string]}}`
  2. `{"stage": "keyword_search", "data": {"results": [{"doc_id": string, "score": number, "snippet": string}], "time_ms": int}}`
  3. `{"stage": "done"}`

Приклад через `curl`:
```bash
curl -X POST http://localhost:8080/api/index -H "Content-Type: application/json" -d '{"threads": 4}'
curl -X POST http://localhost:8080/api/search -H "Content-Type: application/json" -d '{"query": "quantum", "mode": "keyword"}'
```

---

## Бенчмаркінг та масштабованість

Для вимірювання часу при `1, 2, 4, 8, 12` потоках та пропускної здатності:
```bash
pip install requests matplotlib
python benchmark.py
```
Результати у вигляді графіків в `docs/`:
* `docs/benchmark_indexing.png`
* `docs/benchmark_throughput.png`
