# Test Report: client_max_body_size

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/client_max_body_size/config.conf`
- **Result:** ✅ PASS (5/5)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / → 200 | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | POST small (1 KB) → 201 | `POST` | `/upload` | 201 | 201 | ✅ PASS |
| 3 | POST large (150 KB) → 413 | `POST` | `/upload` | 413 | 413 | ✅ PASS |
| 4 | POST at exact 100 KB limit → 201 | `POST` | `/upload` | 201 | 201 | ✅ PASS |
| 5 | POST 1 byte over 100 KB → 413 | `POST` | `/upload` | 413 | 413 | ✅ PASS |
