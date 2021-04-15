FROM balenalib/aarch64-debian:buster-build
LABEL io.balena.device-type="raspberrypi3-64"

RUN apt-get update && apt-get install -y --no-install-recommends \
		less \
		kmod \
		nano \
		net-tools \
		ifupdown \
		iputils-ping \
		i2c-tools \
		usbutils \
        git \
        cmake \
		libfluidsynth-dev \
		socat \
		python3 \
		python3-pip \
        wget \
        sudo 

RUN pip3 install websocket-client \
&& sudo apt-get update \
&& sudo apt-get upgrade -y \
&& sudo apt-get dist-upgrade -y \
&& rm -rf /var/lib/apt/lists/*

RUN [ ! -d /.balena/messages ] && mkdir -p /.balena/messages; echo 'Here are a few details about this Docker image (For more information please visit https://www.balena.io/docs/reference/base-images/base-images/): \nArchitecture: ARM v7 \nOS: Debian Bullseye \nVariant: build variant \nDefault variable(s): UDEV=off \nExtra features: \n- Easy way to install packages with `install_packages <package-name>` command \n- Run anywhere with cross-build feature  (for ARM only) \n- Keep the container idling with `balena-idle` command \n- Show base image details with `balena-info` command' > /.balena/messages/image-info

RUN echo '#!/bin/sh.real\nbalena-info\nrm -f /bin/sh\ncp /bin/sh.real /bin/sh\n/bin/sh "$@"' > /bin/sh-shim \
	&& chmod +x /bin/sh-shim \
	&& cp /bin/sh /bin/sh.real \
	&& mv /bin/sh-shim /bin/sh

RUN adduser --disabled-password \--gecos '' romi \
&& adduser romi sudo \
&& echo "romi:romi" | chpasswd \
&& echo "root:root" | chpasswd \
&& echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Disable leak sanitizer in Qemu
ENV ASAN_OPTIONS=detect_leaks=0

USER romi
WORKDIR /home/romi
# Disable leak sanitizer in Qemu
ENV ASAN_OPTIONS=detect_leaks=0
CMD [ "balena-idle" ]