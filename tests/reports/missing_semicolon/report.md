# Test Report: missing_semicolon (invalid config)

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/missing_semicolon/config.conf`
- **Result:** ✅ PASS

## Expected Behaviour

Server must exit with a **non-zero exit code** on config validation failure.

| Expectation | exit code | Result |
|-------------|-----------|--------|
| exit code ≠ 0 | 1 | ✅ |

### stderr output
```
Config parse failed: Syntax error: Missing semicolon before next property.
```
