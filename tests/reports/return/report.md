# Test Report: return

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/return/config.conf`
- **Result:** ✅ PASS (3/3)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / → 200 | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET /old → 301 | `GET` | `/old` | 301 | 301 | ✅ PASS |
| 3 | GET /new/ → 200 | `GET` | `/new/` | 200 | 200 | ✅ PASS |
