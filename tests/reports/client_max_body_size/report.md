# Test Report: client_max_body_size

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/client_max_body_size/config.conf`
- **Result:** ❌ FAIL (3/5)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / → 200 | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | POST small (1 KB) → 201 | `POST` | `/upload` | 201 | 201 | ✅ PASS |
| 3 | POST large (150 KB) → 413 | `POST` | `/upload` | 413 | 201 (exp 413) | ❌ FAIL |
| 4 | POST at exact 100 KB limit → 201 | `POST` | `/upload` | 201 | 201 | ✅ PASS |
| 5 | POST 1 byte over 100 KB → 413 | `POST` | `/upload` | 413 | 201 (exp 413) | ❌ FAIL |
