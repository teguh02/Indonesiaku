# ===============================
# Dockerfile - Indonesiaku Image
# ===============================
FROM alpine:3.20

LABEL maintainer="Teguh Rijanandi <teguhrijanandi02@gmail.com>"
LABEL description="Docker image for Bahasa Pemrograman Indonesiaku (Python-like language built in C)"
LABEL version="0.1.1"

# Install dependencies
RUN apk add --no-cache \
    build-base \
    bash \
    git \
    make

# Copy project source
WORKDIR /opt/indonesiaku
COPY src ./src
COPY include ./include
COPY Makefile .
COPY examples ./examples

# Build the compiler
RUN make clean && make

# Add compiler binary to PATH
RUN cp indk /usr/local/bin/indk && chmod +x /usr/local/bin/indk

# Default command
ENTRYPOINT ["indk"]
CMD ["--help"]

# ===============================
# End of Dockerfile
# ===============================
