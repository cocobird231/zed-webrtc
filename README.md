# zed-webrtc

Web version: https://hackmd.io/@cocobird231/rJ7DKFcdT

*`Updated: 2024/01/09`*

## Description
This repo demonstrates the application of ZED camera streaming via WebRTC.

## Environment
- OS: Ubuntu 20.04
- Hardware: ZED Box Orin NX, Jetson AGX Orin Developer Kit
- ZED Camera: ZED X series, ZED 2i
- ZED SDK: 4.0

## Prerequisite
1. **CUDA Toolkit**
    If using ZED Box or Jetson AGX Orin, the OS were pre-installed, follow the [official guide](https://docs.nvidia.com/jetson/jetpack/install-jetpack/index.html) to install JetPack.
    ```bash
    sudo apt update
    sudo apt install nvidia-jetpack
    ```
    After JetPack installed, add following lines under `~/.bashrc` for CUDA environment setup.
    ```md
    export CUDA_HOME=/usr/local/cuda
    export LD_LIBRARY_PATH=${CUDA_HOME}/lib64:${LD_LIBRARY_PATH}
    export PATH=${CUDA_HOME}/bin:${PATH}
    ```
    Re-open the terminal and test CUDA setting.
    ```bash
    nvcc -V
    ```
2. **ZED SDK (CUDA required)**
    Download ZED SDK from [here](https://www.stereolabs.com/developers/release) and run installation.
    ```bash
    sudo chmod a+x <zed_sdk_version>.run
    ./<zed_sdk_version>.run
    ```
3. **ZED X Driver (Only ZED X series required)**
    Download ZED X Driver form [here](https://www.stereolabs.com/developers/release#drivers) and run installation.
    ```bash
    sudo dpkg -i <the deb file path>.deb
    ```
    Enable the driver while start-up
    ```bash
    sudo systemctl enable zed_x_daemon
    ```
4. **zed-gstreamer**
    Follow the [installation guide](https://github.com/stereolabs/zed-gstreamer?tab=readme-ov-file#linux-installation). First install the GStreamer and dependencies.
    ```bash!
    sudo apt install libgstreamer1.0-0 gstreamer1.0-libav libgstrtspserver-1.0-0 gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio libgstreamer1.0-dev libgstrtspserver-1.0-dev libgstreamer-plugins-base1.0-0 libgstreamer-plugins-base1.0-dev libgstreamer-plugins-good1.0-0 libgstreamer-plugins-good1.0-dev libgstreamer-plugins-bad1.0-0 libgstreamer-plugins-bad1.0-dev
    ```
    Then install `zed-gstreamer`.
    ```bash
    git clone https://github.com/stereolabs/zed-gstreamer.git
    cd zed-gstreamer
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    sudo make install
    ```
    Check `zed-gstreamer`. The window will start showing the images if installation succeed.
    ```
    gst-launch-1.0 zedsrc ! queue ! autovideoconvert ! queue ! fpsdisplaysink
    ```

## Installation
Clone repo and build.
```bash
git clone https://github.com/cocobird231/zed-webrtc.git
cd zed-webrtc
. install.sh
```

## Usage
In this example, the viewer need to be executed first.

- **Viewer**
    ```bash
    ./sendrecvzed --our-id=0 --remote-offerer
    ```

- **Camera**
    ```bash
    ./sendrecvzed --cam-id=0 --our-id=1 --peer-id=0
    ```
    Use `--help` to get more information.
    
    Camera arguments:
    - `cam-id`
        ZED camera ID.
    - `stream-type`
        Same as `stream-type` under `zed-gstreamer`.
    - `out-width`
        The width of streaming video.
    - `out-height`
        The height of streaming video.
    - `x264-tune`
        h.264 encode param. See [GstX264EncTune](https://gstreamer.freedesktop.org/documentation/x264/index.html#GstX264EncTune).
    - `x264-qp`
        h.264 encode param. See [quantizer](https://gstreamer.freedesktop.org/documentation/x264/index.html#x264enc:quantizer).
    - `x264-preset`
        h.264 encode param. See [GstX264EncPreset](https://gstreamer.freedesktop.org/documentation/x264/index.html#GstX264EncPreset).

## Reference
[JetPack Install](https://docs.nvidia.com/jetson/jetpack/install-jetpack/index.html)
[ZED SDK](https://www.stereolabs.com/developers/release)
[zed-gstreamer](https://github.com/stereolabs/zed-gstreamer)
[gstreamer-webrtc](https://github.com/GStreamer/gst-examples)
[x264enc](https://gstreamer.freedesktop.org/documentation/x264/index.html?gi-language=c)