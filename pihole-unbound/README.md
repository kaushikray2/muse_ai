# Pi-hole + Unbound — ad-blocking DNS with private recursive resolution

Traffic flow: `your devices → Pi-hole (blocks ads/trackers) → Unbound (recurses
directly to authoritative servers, DNSSEC-validated) → internet`

One folder, everything inside it:

```
pihole-unbound/
├── docker-compose.yml
├── unbound-config/
│   └── unbound.conf        # Unbound resolver config
├── etc-pihole/             # Pi-hole settings DB (created on first start)
└── etc-dnsmasq.d/          # Pi-hole dnsmasq config (created on first start)
```

## Deploy on your Ubuntu server

```bash
# 1. Before first start, set the Pi-hole web password in docker-compose.yml:
#    FTLCONF_webserver_api_password: "your-strong-password"

# 2. Copy this folder to the server, e.g. /opt/pihole-unbound, then:
cd /opt/pihole-unbound
docker compose up -d

# 3. Check both are running:
docker ps
docker logs unbound
docker logs pihole
```

Pi-hole web UI: `http://<server-lan-ip>:8080`

## Point clients at it

Set your LAN clients' (or DHCP server's) DNS to this server's LAN IP,
then test:

```bash
dig example.com @<server-lan-ip>              # should resolve
# open a known ad domain in a browser -> Pi-hole blocks it, query log shows it
```

## Port notes (this box runs mailcow)

- **53/udp+tcp** → Pi-hole. Make sure nothing else on the server holds
  port 53 (mailcow doesn't; Ubuntu's `systemd-resolved` stub on
  127.0.0.53 usually doesn't conflict, but see below).
- **8080** → Pi-hole web UI. Port 80 is taken by mailcow, hence 8080.
- Unbound publishes **no** host ports — only Pi-hole reaches it over the
  internal Docker network.

If port 53 is busy, find the culprit with `sudo ss -lunp | grep ':53 '`.
For `systemd-resolved`:

```bash
sudo mkdir -p /etc/systemd/resolved.conf.d
printf '[Resolve]\nDNSStubListener=no\n' | sudo tee /etc/systemd/resolved.conf.d/no-stub.conf
sudo systemctl restart systemd-resolved
```

## Firewall

```bash
sudo ufw allow 53/udp
sudo ufw allow 53/tcp
sudo ufw allow 8080/tcp
```

## Notes

- Unbound is **recursive**: no upstream provider (Google/Cloudflare/ISP)
  sees your queries. Pi-hole only filters; every allowed query is
  resolved and DNSSEC-validated by Unbound.
- Unbound's DNSSEC trust anchor (`unbound-config/var/root.key`) is
  created automatically on first start.
- Blocklists are managed in the Pi-hole web UI (default lists are fine
  to start with).
- Do **not** expose port 53 to the internet.
