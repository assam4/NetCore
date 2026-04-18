# Test Report: allow_methods

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/allow_methods/config.conf`
- **Result:** ❌ FAIL (5/6)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / → 200 | `GET` | `/` | 200 | 404 (exp 200) | ❌ FAIL |
| 2 | POST / → 405 | `POST` | `/` | 405 | 405 | ✅ PASS |
| 3 | DELETE / → 405 | `DELETE` | `/` | 405 | 405 | ✅ PASS |
| 4 | GET /upload → 405 | `GET` | `/upload` | 405 | 405 | ✅ PASS |
| 5 | DELETE /upload → 405 | `DELETE` | `/upload` | 405 | 405 | ✅ PASS |
| 6 | POST /upload → 201 | `POST` | `/upload` | 201 | 201 | ✅ PASS |
