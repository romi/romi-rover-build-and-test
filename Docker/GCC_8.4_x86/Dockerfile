FROM ubuntu:18.04

ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

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
		socat \
		python3 \
		python3-pip \
        wget \
        sudo \
		libfluidsynth-dev \
		libjpeg-dev \
		zlib1g-dev \
		libpng-dev

RUN pip3 install websocket-client \
&& sudo apt-get update \
&& sudo apt-get upgrade -y \
&& sudo apt-get dist-upgrade -y 

RUN apt-get update && apt-get install -y \
apt-transport-https \
ca-certificates gnupg \
software-properties-common

RUN wget -qO - https://apt.kitware.com/keys/kitware-archive-latest.asc | apt-key add - \
&& apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' \
&& apt-get update \
&& apt-get install -y cmake 

RUN apt-get update ; apt-get upgrade -y ; apt-get install -y gcc-8 g++-8 \
&& update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8 \
&& update-alternatives --config gcc \
&& rm -rf /var/lib/apt/lists/*

RUN adduser --disabled-password \--gecos '' romi \
&& adduser romi sudo \
&& echo "romi:romi" | chpasswd \
&& echo "root:root" | chpasswd \
&& echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers


USER romi
WORKDIR /home/romi

CMD tail -f /dev/null 