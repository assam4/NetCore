# Test Report: missing_semicolon (invalid config)

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/missing_semicolon/config.conf`
- **Result:** ✅ PASS

## Expected Behaviour

Server must exit with a **non-zero exit code** on config validation failure.

| Expectation | exit code | Result |
|-------------|-----------|--------|
| exit code ≠ 0 | 1 | ✅ |

### stderr output
```
bind() failed for 0.0.0.0:80
Server initialization failed: Not have listener
```
