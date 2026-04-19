# Test Report: full

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/full/config.conf`
- **Result:** ✅ PASS (9/9)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET /files/ (autoindex) | `GET` | `/files/` | 200 | 200 | ✅ PASS |
| 3 | GET /files/notes.txt | `GET` | `/files/notes.txt` | 200 | 200 | ✅ PASS |
| 4 | DELETE /files/notes.txt | `DELETE` | `/files/notes.txt` | 204 | 204 | ✅ PASS |
| 5 | GET /files/notes.txt (gone) | `GET` | `/files/notes.txt` | 404 | 404 | ✅ PASS |
| 6 | GET /old → 301 | `GET` | `/old` | 301 | 301 | ✅ PASS |
| 7 | GET /missing → 404 | `GET` | `/missing` | 404 | 404 | ✅ PASS |
| 8 | GET /files (no slash) → 301 | `GET` | `/files` | 301 | 301 | ✅ PASS |
| 9 | GET /files/sample.txt still 200 after DELETE | `GET` | `/files/sample.txt` | 200 | 200 | ✅ PASS |
