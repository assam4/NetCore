# Test Report: nested_locations

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/nested_locations/config.conf`
- **Result:** ✅ PASS (3/3)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET /images/ (autoindex) | `GET` | `/images/` | 200 | 200 | ✅ PASS |
| 3 | GET /images/diagram.svg | `GET` | `/images/diagram.svg` | 200 | 200 | ✅ PASS |
