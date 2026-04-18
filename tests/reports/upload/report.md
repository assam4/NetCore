# Test Report: upload

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/upload/config.conf`
- **Result:** ❌ FAIL (7/9)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | POST /upload → 201 | `POST` | `/upload` | 201 | 201 | ✅ PASS |
| 3 | GET /files/ (file listed) | `GET` | `/files/` | 200 | 200 | ✅ PASS |
| 4 | DELETE /files/webserv_test_42.txt | `DELETE` | `/files/webserv_test_42.txt` | 204 | 204 | ✅ PASS |
| 5 | GET /files/webserv_test_42.txt → 404 | `GET` | `/files/webserv_test_42.txt` | 404 | 404 | ✅ PASS |
| 6 | DELETE /files/nonexistent → 404 | `DELETE` | `/files/does_not_exist.txt` | 404 | 404 | ✅ PASS |
| 7 | POST /upload no Content-Type → 400 | `POST` | `/upload` | 400 | 201 (exp 400) | ❌ FAIL |
| 8 | POST /upload wrong Content-Type → 400 | `POST` | `/upload` | 400 | 201 (exp 400) | ❌ FAIL |
| 9 | POST /upload no boundary → 400 | `POST` | `/upload` | 400 | 400 | ✅ PASS |
