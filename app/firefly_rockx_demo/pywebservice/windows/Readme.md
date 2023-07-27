# 介绍

本分支为了识别 RV1126 车牌识别做的 http 服务端和客户端。为了让 windows 和 Linux 平台都可以用，使用了跨平台语言 python。

依赖:
	tkinter
	flask
	pywin32

请使用 pip3 自行安装以上环境。

# 运行

打开两个终端，根据设备 ip 配置 python 代码的 ip 和端口:
	python draw.py
	python Pywebservice.py

然后使用 chttprequest 在设备端发送 http 请求就可以了。

# 注：
```
bin 目录存放打包好的可执行 exe 文件
src 目录存放 Python 源码
修改 ip_config.txt 配置服务器的 ip 地址和端口号
```
