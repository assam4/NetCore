# Test Report: autoindex

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/autoindex/config.conf`
- **Result:** ✅ PASS (4/4)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / (listing) | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET /about.html | `GET` | `/about.html` | 200 | 200 | ✅ PASS |
| 3 | GET /readme.txt | `GET` | `/readme.txt` | 200 | 200 | ✅ PASS |
| 4 | POST / → 405 | `POST` | `/` | 405 | 405 | ✅ PASS |
