# Test Report: allow_methods

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/allow_methods/config.conf`
- **Result:** ✅ PASS (6/6)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / → 200 | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | POST / → 405 | `POST` | `/` | 405 | 405 | ✅ PASS |
| 3 | DELETE / → 405 | `DELETE` | `/` | 405 | 405 | ✅ PASS |
| 4 | GET /upload → 405 | `GET` | `/upload` | 405 | 405 | ✅ PASS |
| 5 | DELETE /upload → 405 | `DELETE` | `/upload` | 405 | 405 | ✅ PASS |
| 6 | POST /upload → 201 | `POST` | `/upload` | 201 | 201 | ✅ PASS |
