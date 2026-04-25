#!/bin/sh
# test_dispatch.sh — black-box test of the readline dispatcher.
#
# Pipes lines into ./algocli and asserts on stdout/stderr. We don't
# attempt PAM-authenticated paths — those need a configured PAM stack
# and an interactive password. We only exercise:
#   - help / unknown commands / dispatch tree
#   - input validators (every shell metachar still gets rejected)
#   - read-only commands run without auth
#   - state-changing commands trigger the auth gate (and bounce off
#     it because PAM is unconfigured under test)
#
# Exit 0 on success; prints the failing case and exits 1 otherwise.

set -eu

ALGOCLI="${ALGOCLI:-./algocli}"
[ -x "$ALGOCLI" ] || { echo "no executable at $ALGOCLI"; exit 2; }

failures=0

run_case() {
    desc=$1; input=$2; expect=$3
    actual=$(printf '%s\nexit\n' "$input" | "$ALGOCLI" 2>&1)
    if ! printf '%s\n' "$actual" | grep -Fq "$expect"; then
        echo "FAIL: $desc"
        echo "  input:    $input"
        echo "  expected: $expect"
        echo "  actual (last 5 lines):"
        printf '%s\n' "$actual" | tail -5 | sed 's/^/    /'
        failures=$((failures + 1))
    fi
}

# -- help / discovery ------------------------------------------------
run_case "help lists all commands"         "help"        "accessctrl"
run_case "? is alias for help"             "?"           "accessctrl"
run_case "unknown command shows fallback"  "wibble"      "Supported AlgoOS commands"

# -- dispatch tree ---------------------------------------------------
run_case "bp shows subcommands"            "bp"          "bp create"
run_case "fs shows quota subcommand"       "fs"          "fs quota"
run_case "fs quota shows leaf subcommands" "fs quota"    "quota enable"
run_case "system shows subcommands"        "system"      "system reboot"
run_case "log shows subcommands"           "log"         "log list"

# -- auth gate fires *before* the command body runs. Validators are
# unit-tested in test_validate.c and test_registry.c. Here we just
# confirm injection-style inputs don't reach the spawn() path —
# they're stopped at the auth gate. (When PAM is configured at deploy
# time and the user authenticates, the validator is the next layer.)
run_case "bp create with metachar still gates"   "bp create a;b"    "User verification failed"
run_case "bp create with command-sub still gates" "bp create a\$(b)" "User verification failed"

# -- not-implemented stubs ------------------------------------------
run_case "syshealth says not implemented"  "syshealth"   "not implemented"
run_case "repl says not implemented"       "repl"        "not implemented"
run_case "cifs says not implemented"       "cifs"        "not implemented"

# -- auth gate fires on state-changing ops --------------------------
run_case "bp create gates on auth"         "bp create good_name" "User verification failed"
run_case "system reboot gates on auth"     "system reboot"       "User verification failed"
run_case "nfs enable gates on auth"        "nfs enable"          "User verification failed"

# -- too many tokens rejected ---------------------------------------
many=$(printf 'help %.0s' $(seq 1 80))
run_case "long token list rejected"        "$many"       "Too many arguments"

if [ "$failures" -gt 0 ]; then
    echo
    echo "$failures failure(s)"
    exit 1
fi
echo "OK — all dispatch tests passed"
