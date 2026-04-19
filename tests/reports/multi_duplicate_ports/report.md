# Test Report: multi_duplicate_ports (invalid config)

- **Date:** 2026-04-19 16:56:33
- **Config:** `tests/reports/_runtime_configs/configs/multi_duplicate_ports/config.conf`
- **Result:** ✅ PASS

## Expected Behaviour

Server must exit with a **non-zero exit code** on config validation failure.

| Expectation | exit code | Result |
|-------------|-----------|--------|
| exit code ≠ 0 | 1 | ✅ |

### stderr output
```
Config parse failed: Syntax error: Unexpected token keyword detected in this context.
```
