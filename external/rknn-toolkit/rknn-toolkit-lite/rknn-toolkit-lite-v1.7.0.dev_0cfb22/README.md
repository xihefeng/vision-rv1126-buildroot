# firefly debian固件RKNN Toolkit Lite安装步骤

## 安装依赖
1. 安装numpy / psutils / ruamel.yaml
```
# 如果没有装pip，请先用sudo apt-get update && sudo apt-get install python3-pip装下
pip3 install numpy==1.16.3
pip3 install psutil==5.6.2
pip3 install ruamel.yaml==0.15.81
```

2. 安装opencv-python。用pip3安装一直失败，所以直接在网上找了个包。
```
# 先装以下依赖， wget用到的两个deb包已经放在rknn-toolkit-lite-v1.7.0.dev_0cfb22/requires/目录下
sudo apt-get install multiarch-support
wget http://security.debian.org/debian-security/pool/updates/main/j/jasper/libjasper1_1.900.1-debian1-2.4+deb8u6_armhf.deb
sudo dpkg -i libjasper1_1.900.1-debian1-2.4+deb8u6_armhf.deb
wget http://security.debian.org/debian-security/pool/updates/main/j/jasper/libjasper-dev_1.900.1-debian1-2.4+deb8u6_armhf.deb
sudo dpkg -i libjasper-dev_1.900.1-debian1-2.4+deb8u6_armhf.deb
sudo apt-get install libhdf5-dev
sudo apt-get install libatlas-base-dev
sudo apt-get install libqtgui4
sudo apt-get install libqt4-test
pip3 install rknn-toolkit-lite-v1.7.0.dev_0cfb22/requires/opencv_python-4.0.1.24-cp37-cp37m-linux_armv7l.whl
```

## 安装RKNN Toolkit Lite
使用以下命令安装RKNN Toolkit Lite
```
pip3 install rknn-toolkit-lite-v1.7.0.dev_0cfb22/packages/rknn_toolkit_lite-1.7.0.dev_0cfb22-cp37-cp37m-linux_armv7l.whl
```

## 跑example
```
cd rknn-toolkit-lite-v1.7.0.dev_0cfb22/examples-lite/inference_with_lite
python3 test.py
```
