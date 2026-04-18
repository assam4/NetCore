# Test Report: errors

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/errors/config.conf`
- **Result:** ❌ FAIL (6/7)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | GET / → 200 | `GET` | `/` | 200 | 200 | ✅ PASS |
| 2 | POST /post-no-store (no upload_store) → 501 | `POST` | `/post-no-store` | 501 | 404 (exp 501) | ❌ FAIL |
| 3 | GET /noindex/ (autoindex off, no index) → 403 | `GET` | `/noindex/` | 403 | 403 | ✅ PASS |
| 4 | malformed request line → 400 | `RAW` | `<raw>` | 400 | 400 | ✅ PASS |
| 5 | HTTP/1.1 missing Host header → 400 | `RAW` | `<raw>` | 400 | 400 | ✅ PASS |
| 6 | LF-only line endings (no CR) → 400 | `RAW` | `<raw>` | 400 | 400(close) | ✅ PASS — server closed connection (acceptable for 400) |
| 7 | bare CRLF (empty request) → 400 or close | `RAW` | `<raw>` | 400 | 400(close) | ✅ PASS — server closed connection (acceptable for 400) |
