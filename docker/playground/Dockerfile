# ===============================
# Image: situs dokumentasi + playground Indonesiaku
# Satu image, dua peran (lihat docker-compose.yml):
#   web    → nginx menyajikan website/ dan mem-proxy /api/ ke runner
#   runner → python menjalankan indk di sandbox
# ===============================
FROM alpine:3.20 AS build

RUN apk add --no-cache build-base
WORKDIR /src
COPY src ./src
COPY Makefile ./
RUN make clean && make && strip indk

# ---------- runtime ----------
FROM alpine:3.20

RUN apk add --no-cache nginx python3 su-exec \
 && adduser -D -H -u 10001 runner

LABEL maintainer="Teguh Rijanandi <teguhrijanandi02@gmail.com>"
LABEL description="Dokumentasi + playground bahasa pemrograman Indonesiaku"
LABEL version="1.0.0"

# Interpreter + pustaka standar (untuk `impor "pustaka/..."`).
COPY --from=build /src/indk /usr/local/bin/indk
COPY pustaka /opt/indonesiaku/pustaka
RUN chmod -R a-w /opt/indonesiaku/pustaka

# Situs statis (dokumentasi, panduan, playground).
COPY website /usr/share/nginx/html

# Layanan playground.
COPY docker/playground/server.py /opt/playground/server.py
COPY docker/playground/nginx.conf /etc/nginx/nginx.conf

RUN mkdir -p /var/cache/nginx /var/run /tmp \
 && chown -R runner:runner /tmp

EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
