# Test Report: cgi

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/cgi/config.conf`
- **Result:** ✅ PASS (3/3)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET /cgi-bin/hello.py | `GET` | `/cgi-bin/hello.py` | 200 | 200 | ✅ PASS |
| 2 | GET /cgi-bin/hello.py?name=42 | `GET` | `/cgi-bin/hello.py?name=42` | 200 | 200 | ✅ PASS |
| 3 | GET /cgi-bin/test.php | `GET` | `/cgi-bin/test.php` | 200 | 200 | ✅ PASS |
