#! /usr/bin/env bash

set -e

apt update -y --quiet

#Build Tools
DEBIAN_FRONTEND=noninteractive apt -y --quiet install \
	build-essential \
	ccache \
	cmake \
	cppcheck \
	file \
	g++ \
	gcc \
	gdb \
	git \
	libfuse2 \
	make \
	ninja-build \
	rsync \
	binutils \
	locales \
	patchelf \
	pkgconf \
	libtool \
	appstream \
	python3 \
	python3-pip

#Qt Required
DEBIAN_FRONTEND=noninteractive apt -y --quiet install \
	libgl1-mesa-dev \
	libpulse-dev \
	libxcb-glx0 \
	libxcb-icccm4 \
	libxcb-image0 \
	libxcb-keysyms1 \
	libxcb-randr0 \
	libxcb-render-util0 \
	libxcb-render0 \
	libxcb-shape0 \
	libxcb-shm0 \
	libxcb-sync1 \
	libxcb-util1 \
	libxcb-xfixes0 \
	libxcb-xinerama0 \
	libxcb1 \
	libxkbcommon-dev \
	libxkbcommon-x11-0 \
	libxcb-xkb-dev \
	libxcb-cursor0 \
	libdrm-dev \
	libzstd-dev

#QGC
DEBIAN_FRONTEND=noninteractive apt -y --quiet install \
	libsdl2-dev \
	libspeechd2 \
	flite \
	speech-dispatcher \
	speech-dispatcher-flite

#GStreamer
DEBIAN_FRONTEND=noninteractive apt -y --quiet install \
	libgstreamer1.0-dev \
	libgstreamer-gl1.0-0 \
	libgstreamer-plugins-base1.0-dev \
	libgstreamer-plugins-good1.0-dev \
	libgstreamer-plugins-bad1.0-dev \
	gstreamer1.0-plugins-base \
	gstreamer1.0-plugins-good \
	gstreamer1.0-plugins-bad \
	gstreamer1.0-plugins-ugly \
	gstreamer1.0-plugins-rtp \
	gstreamer1.0-libav \
	gstreamer1.0-tools \
	gstreamer1.0-x \
	gstreamer1.0-alsa \
	gstreamer1.0-gl \
	gstreamer1.0-gtk3 \
	gstreamer1.0-gl \
	gstreamer1.0-vaapi \
	gstreamer1.0-rtsp
