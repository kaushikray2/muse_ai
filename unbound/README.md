# Unbound — self-hosted recursive DNS

One folder, everything inside it:

```
unbound/
├── docker-compose.yml   # the stack
├── config/
│   └── unbound.conf     # resolver configuration
└── data/                # reserved for future state (currently unused)
```

`var/root.key` (DNSSEC trust anchor) is created automatically inside `config/`
on first start and refreshed by Unbound itself.

## Deploy on your Ubuntu server

```bash
# 1. Copy this folder to the server, e.g. /opt/unbound, then:
cd /opt/unbound
docker compose up -d

# 2. Check it's running:
docker ps
docker logs unbound
```

## Free up port 53 (Ubuntu)

Ubuntu's `systemd-resolved` stub listener usually occupies port 53.
Unbound needs it. Options:

```bash
# Option A: disable the stub listener (keeps resolved running)
sudo mkdir -p /etc/systemd/resolved.conf.d
printf '[Resolve]\nDNSStubListener=no\n' | sudo tee /etc/systemd/resolved.conf.d/no-stub.conf
sudo systemctl restart systemd-resolved

# Option B: point the server itself at Unbound afterwards
# /etc/systemd/resolved.conf -> DNS=127.0.0.1
```

## Firewall

```bash
sudo ufw allow 53/udp
sudo ufw allow 53/tcp
```

## Point clients at it

Set your LAN clients' (or DHCP server's) DNS to this server's LAN IP,
then test:

```bash
dig example.com @<server-lan-ip>      # should resolve
dig example.com @<server-lan-ip> +dnssec  # AD flag = DNSSEC validated
```

## Notes

- This is a **recursive** resolver: it talks to root/TLD/authoritative
  servers directly, no upstream provider sees your queries.
- Only private LAN ranges are answered (see `access-control` in
  `config/unbound.conf`). Do **not** expose port 53 to the internet —
  an open resolver will be abused.
- Tune `num-threads` / cache sizes in `unbound.conf` to the box.
