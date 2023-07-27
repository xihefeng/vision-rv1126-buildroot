import flask
import os
import win32file
import win32pipe
import time

server_init_flag = 1
fifocount = 0
fifocolor = 0
fifonum = 0
fifotype = 0

server=flask.Flask(__name__)#__name__代表当前的python文件。把当前的python文件当做一个服务启动
@server.route('/license_plate',methods=['get','post'])
def loadfile():
    global server_init_flag
    global fifocount
    global fifocolor
    global fifonum
    global fifotype
    if server_init_flag == 1 :
        server_init_flag=0
        fifocount = win32file.CreateFile(
                r'\\.\pipe\count',
                win32file.GENERIC_WRITE,
                0,
                None,
                win32file.OPEN_EXISTING,
                0,
                None)
        print(fifocount)
        fifocolor = win32file.CreateFile(
                r'\\.\pipe\color',
                win32file.GENERIC_WRITE,
                0,
                None,
                win32file.OPEN_EXISTING,
                0,
                None)
        print(fifocolor)
        fifonum = win32file.CreateFile(
                r'\\.\pipe\plate',
                win32file.GENERIC_WRITE,
                0,
                None,
                win32file.OPEN_EXISTING,
                0,
                None)
        print(fifonum)
        fifotype = win32file.CreateFile(
                r'\\.\pipe\type',
                win32file.GENERIC_WRITE,
                0,
                None,
                win32file.OPEN_EXISTING,
                0,
                None)
        print(fifotype)

    p_count = ((flask.request.values.get('count')).encode('gbk'))
    p_num = ((flask.request.values.get('num')).encode('gbk'))
    p_color = ((flask.request.values.get('color')).encode('gbk'))
    p_type = ((flask.request.values.get('type')).encode('gbk'))

    print(p_count)
    print(p_num)
    print(p_color)
    print(p_type)

    win32file.WriteFile(fifocount, p_count)
    win32file.WriteFile(fifocolor, p_color)
    win32file.WriteFile(fifonum, p_num)
    win32file.WriteFile(fifotype, p_type)
    return "ok\r\n"
    # port可以指定端口，默认端口是5000
    # host默认是服务器，默认是127.0.0.1
    # debug=True 修改时不关闭服务

if __name__ == '__main__':
    txt={}
    i = 0

    with open("ip_config.txt",'r') as f:
        for line in f:
            txt[i] = line
            i += 1

    txt_ip_address = txt[0].strip('\n')
    txt_port = txt[1].strip('\n')
    print(txt_ip_address)
    print(txt_port)

    server.run(debug=True,host=txt_ip_address,port=txt_port)
