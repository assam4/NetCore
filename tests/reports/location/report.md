# Test Report: location

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/location/config.conf`
- **Result:** ✅ PASS (2/2)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET /missing | `GET` | `/missing` | 404 | 404 | ✅ PASS |
