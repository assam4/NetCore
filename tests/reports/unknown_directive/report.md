# Test Report: unknown_directive (invalid config)

- **Date:** 2026-04-18 12:57:03
- **Config:** `configs/unknown_directive/config.conf`
- **Result:** ✅ PASS

## Expected Behaviour

Server must exit with a **non-zero exit code** on config validation failure.

| Expectation | exit code | Result |
|-------------|-----------|--------|
| exit code ≠ 0 | 1 | ✅ |

### stderr output
```
Config parse failed: Syntax error: Unexpected value in this context. (previous is not a property).
```
