# Test Report: basic

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/basic/config.conf`
- **Result:** ✅ PASS (9/9)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET /missing | `GET` | `/missing` | 404 | 404 | ✅ PASS |
| 3 | POST / → 405 | `POST` | `/` | 405 | 405 | ✅ PASS |
| 4 | DELETE / → 405 | `DELETE` | `/` | 405 | 405 | ✅ PASS |
| 5 | HEAD / → 501 | `HEAD` | `/` | 501 | 501 | ✅ PASS |
| 6 | PUT / → 501 | `PUT` | `/` | 501 | 501 | ✅ PASS |
| 7 | PATCH / → 501 | `PATCH` | `/` | 501 | 501 | ✅ PASS |
| 8 | GET /../../etc/passwd → 404 | `GET` | `/../../etc/passwd` | 404 | 404 | ✅ PASS |
| 9 | Content-Type text/html on GET / | `GET` | `/` | 200 | 200 | ✅ PASS |
