# Test Report: multi_host

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/multi_host/config.conf`
- **Result:** ✅ PASS (2/2)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET 127.0.0.1:8080/ | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | GET 127.0.0.1:8081/ | `GET` | `/` | 200 | 200 | ✅ PASS |
