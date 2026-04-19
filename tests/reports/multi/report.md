# Test Report: multi

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/multi/config.conf`
- **Result:** ✅ PASS (3/3)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET :8080/ → site1 | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET :8081/ → site2 | `GET` | `/` | 200 | 200 | ✅ PASS |
| 3 | GET :8080/missing | `GET` | `/missing` | 404 | 404 | ✅ PASS |
