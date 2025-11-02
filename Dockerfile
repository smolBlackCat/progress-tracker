FROM debian:13

RUN apt update && apt install -y build-essential dpkg-dev file cmake libgtkmm-4.0-dev \
	libadwaita-1-dev libtinyxml2-dev libspdlog-dev gettext catch2 ninja-build

WORKDIR /workspace
