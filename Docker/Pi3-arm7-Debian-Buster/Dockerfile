FROM balenalib/armv7hf-debian:buster-build
LABEL io.balena.device-type="raspberrypi3"
RUN echo "deb http://archive.raspbian.org/raspbian buster main contrib non-free rpi firmware" >>  /etc/apt/sources.list \
	&& apt-key adv --batch --keyserver ha.pool.sks-keyservers.net  --recv-key 0x9165938D90FDDD2E \
	&& echo "deb http://archive.raspberrypi.org/debian buster main ui" >>  /etc/apt/sources.list.d/raspi.list \
	&& apt-key adv --batch --keyserver ha.pool.sks-keyservers.net  --recv-key 0x82B129927FA3303E


RUN apt-get update && apt-get install -y --no-install-recommends \
		less \
		libraspberrypi-bin \
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
		# libjpeg-dev \
		# libglib2.0-dev // In case if including fluidsynth from source.

RUN pip3 install websocket-client \
&& sudo apt-get update \
&& sudo apt-get upgrade -y \
&& sudo apt-get dist-upgrade -y 

# specifically at the end as we only want 3.13 betweem 3.14 and 3.18 wont work in a Qemu docker container.
RUN apt install cmake-data=3.13.4-1 && apt install cmake=3.13.4-1

RUN rm -rf /var/lib/apt/lists/*

RUN [ ! -d /.balena/messages ] && mkdir -p /.balena/messages; echo 'Here are a few details about this Docker image (For more information please visit https://www.balena.io/docs/reference/base-images/base-images/): \nArchitecture: ARM v7 \nOS: Debian Buster \nVariant: build variant \nDefault variable(s): UDEV=off \nExtra features: \n- Easy way to install packages with `install_packages <package-name>` command \n- Run anywhere with cross-build feature  (for ARM only) \n- Keep the container idling with `balena-idle` command \n- Show base image details with `balena-info` command' > /.balena/messages/image-info

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