# Event Recording and Replay

Simulation inputs are recorded in a stable line-oriented text format:

```text
SUBMIT <order-id> <instrument-id> <side> <type> <quantity> <price-or-zero>
CANCEL <order-id>
MODIFY <order-id> <quantity> <price-or-zero>
```

Side and type use deterministic numeric values. A zero price represents no
price for market orders or an unchanged modification price. `EventLog` can
write/read streams and files, and it rejects malformed or unknown records.

`aegisx-sim --record FILE` records generated inputs. `aegisx-replay --input
FILE` applies those inputs to a fresh engine and reports the resulting trade
and resting-order counts. Replay tests verify stable round trips and matching
final book state.
