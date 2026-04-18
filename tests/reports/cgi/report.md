# Test Report: cgi

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/cgi/config.conf`
- **Result:** ❌ FAIL (0/1)

## Test Cases

| # | Test | Method | Path | Expected | Actual | Result |
|---|------|--------|------|----------|--------|--------|
| 1 | server startup | `—` | `—` | running | crashed (rc=1) | ❌ FAIL — stderr: Server initialization failed: Parsing error: Incorrect cgi extension: '/usr/bin/php-cgi'. |
