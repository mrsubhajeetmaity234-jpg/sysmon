#!/bin/sh
# Smoke test: run the monitor for 2 samples and check that every section is printed.
BIN=./bin/sysmon
LOG=$(mktemp)
OUT=$($BIN -n 2 -i 1 --no-clear -l "$LOG") || { echo "FAIL: program exited with error"; exit 1; }

fail=0
for pattern in "System / Architecture" "== CPU ==" "== Memory ==" "Disk & Network" "Top Processes" "Monitor stopped"; do
    echo "$OUT" | grep -q "$pattern" || { echo "FAIL: missing '$pattern'"; fail=1; }
done

lines=$(wc -l < "$LOG")
[ "$lines" -eq 3 ] || { echo "FAIL: expected 3 CSV lines (header + 2 rows), got $lines"; fail=1; }

rm -f "$LOG"
[ $fail -eq 0 ] && echo "All smoke tests passed."
exit $fail
