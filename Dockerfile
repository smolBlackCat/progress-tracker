FROM debian:13 AS builder

RUN apt update && apt install -y build-essential dpkg-dev file cmake libgtkmm-4.0-dev \
	libadwaita-1-dev libtinyxml2-dev libspdlog-dev gettext ninja-build

WORKDIR /workspace
COPY . .

RUN cmake -S . -B build -GNinja -DDEVELOPMENT=OFF -DCMAKE_INSTALL_PREFIX=/usr
RUN cmake --build build
RUN cd build && cpack

FROM debian:13 AS runtime

COPY --from=builder /workspace/build/*.deb /app/progress.deb
RUN apt update
RUN apt install -f -y /app/progress.deb

RUN useradd --create-home progress
USER progress

CMD ["progress"]