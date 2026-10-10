# Apache + PHP web server

Single-folder Docker Compose package. Everything lives under this folder.

## Run

```bash
cd apache-php-web
docker compose up -d
curl http://localhost:8080/
```

## Change the host port

Edit `.env`:

```env
HOST_PORT=8080
```

## Stop and remove

```bash
docker compose down
```
