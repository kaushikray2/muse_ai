#!/usr/bin/env bash
# smoke-test.sh — read-only verification for the Pi-hole + Unbound stack
# (also works for the standalone Unbound stack).
#
# Run on the Ubuntu server after `docker compose up -d`, then paste the
# full output back. Nothing here changes the system: it only reads.
set -u
PASS=0; FAIL=0; SKIP=0
ok()   { PASS=$((PASS+1)); echo "  [PASS] $1"; }
bad()  { FAIL=$((FAIL+1)); echo "  [FAIL] $1"; }
skip() { SKIP=$((SKIP+1)); echo "  [SKIP] $1"; }

have() { command -v "$1" >/dev/null 2>&1; }

echo "=== 1. Docker and compose stacks ==="
if ! have docker; then
  bad "docker not found on this host"
else
  ok "docker is present ($(docker --version 2>/dev/null | head -1))"
fi

echo "=== 2. Expected containers ==="
want_unbound=true
want_pihole=false
if docker ps --format '{{.Names}}' 2>/dev/null | grep -qx 'pihole'; then
  want_pihole=true
fi
for c in unbound pihole; do
  if [ "$c" = "pihole" ] && [ "$want_pihole" = false ]; then
    skip "pihole container not present (standalone Unbound deploy?)"
    continue
  fi
  if docker ps --format '{{.Names}}' 2>/dev/null | grep -qx "$c"; then
    state=$(docker inspect -f '{{.State.Health.Status}} {{.State.Status}}' "$c" 2>/dev/null)
    ok "container '$c' is running ($state)"
  else
    bad "container '$c' is not running (check: cd <folder> && docker compose up -d)"
  fi
done

echo "=== 3. Port 53 ownership on the host ==="
if have ss; then
  holder=$(ss -lunp 2>/dev/null | grep -E ':53 ' | awk '{print $NF}' | sort -u | tr '\n' ' ')
  if [ -n "$holder" ]; then
    ok "port 53/udp is held by: $holder"
  else
    bad "nothing is listening on port 53/udp"
  fi
else
  skip "ss not available"
fi

echo "=== 4. Container logs (last 30 lines, error scan) ==="
for c in unbound pihole; do
  if docker ps --format '{{.Names}}' 2>/dev/null | grep -qx "$c"; then
    if docker logs --tail 30 "$c" 2>&1 | grep -Ei 'fatal|failed to|error: .*bind|address already in use|cannot|denied' | grep -vi 'no error' >/dev/null; then
      bad "'$c' logs show errors — see: docker logs $c"
    else
      ok "'$c' logs look clean"
    fi
  fi
done

echo "=== 5. DNS resolution from the host ==="
if ! have dig; then
  skip "dig not installed (sudo apt install -y dnsutils) — resolution checks skipped"
else
  if dig @127.0.0.1 example.com +short +time=5 +tries=2 2>/dev/null | grep -E '^[0-9.]+$' >/dev/null; then
    ok "example.com resolves via 127.0.0.1 (end-to-end through Pi-hole/Unbound)"
  else
    bad "example.com did NOT resolve via 127.0.0.1"
  fi

  # NXDOMAIN handling
  if dig @127.0.0.1 this-domain-should-not-exist-xyz123.com +short +time=5 +tries=1 2>/dev/null | grep -q .; then
    bad "bogus domain returned an answer (expected NXDOMAIN)"
  else
    ok "bogus domain correctly returns NXDOMAIN"
  fi

  # DNSSEC validation, tested directly against the Unbound container
  UBIP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' unbound 2>/dev/null)
  if [ -n "$UBIP" ]; then
    if dig "@$UBIP" example.com +dnssec +time=8 +tries=2 2>/dev/null | grep -q 'flags:.* ad '; then
      ok "Unbound ($UBIP) validates DNSSEC (AD flag set)"
    else
      bad "Unbound ($UBIP) did not return the AD flag — DNSSEC validation may be off"
    fi
  else
    skip "could not get unbound container IP"
  fi
fi

echo "=== 6. Pi-hole -> Unbound internal link ==="
if docker ps --format '{{.Names}}' 2>/dev/null | grep -qx 'pihole'; then
  UBHOST=$(docker exec pihole getent hosts unbound 2>/dev/null | awk '{print $1}')
  if [ -n "$UBHOST" ]; then
    ok "pihole can resolve 'unbound' to $UBHOST on the internal network"
  else
    bad "pihole cannot resolve hostname 'unbound' (check the shared docker network)"
  fi
else
  skip "pihole container not running"
fi

echo
echo "=========================================="
echo "RESULT: $PASS passed, $FAIL failed, $SKIP skipped"
echo "=========================================="
[ "$FAIL" -eq 0 ]
