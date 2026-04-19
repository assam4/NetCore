# Test Report: bad_port (invalid config)

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/bad_port/config.conf`
- **Result:** ✅ PASS

## Expected Behaviour

Server must exit with a **non-zero exit code** on config validation failure.

| Expectation | exit code | Result |
|-------------|-----------|--------|
| exit code ≠ 0 | 1 | ✅ |

### stderr output
```
Server initialization failed: Parsing error: Port must be in in range 1-65535.
```
