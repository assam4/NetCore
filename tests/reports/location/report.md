# Test Report: location

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/location/config.conf`
- **Result:** ✅ PASS (2/2)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET /missing | `GET` | `/missing` | 404 | 404 | ✅ PASS |
