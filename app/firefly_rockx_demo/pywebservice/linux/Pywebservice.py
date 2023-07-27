import flask
import os

fifo_count_name = 'count'
fifo_color_name = 'color'
fifo_plate_name = 'plate'
fifo_type_name = 'type'

fifo_file_folder="fifo_file"
os.makedirs(fifo_file_folder, exist_ok=True)

fifo_file_path = os.path.join(os.getcwd(), fifo_file_folder)

fifo_count_path = os.path.join(fifo_file_path, fifo_count_name)
fifo_color_path = os.path.join(fifo_file_path, fifo_color_name)
fifo_plate_path = os.path.join(fifo_file_path, fifo_plate_name)
fifo_type_path = os.path.join(fifo_file_path, fifo_type_name)

server=flask.Flask(__name__)#__name__代表当前的python文件。把当前的python文件当做一个服务启动
@server.route('/license_plate',methods=['get','post'])
def loadfile():
    p_count = flask.request.values.get('count')
    p_num = flask.request.values.get('num')
    p_color = flask.request.values.get('color')
    p_type = flask.request.values.get('type')
    with open(fifo_count_path, 'w') as fifocount:
    	print("get count:" + p_count);
    	fifocount.write(p_count)
    	fifocount.flush()
    with open(fifo_color_path, 'w') as fifocolor:
    	print("get color:" + p_color);
    	fifocolor.write(p_color)
    	fifocolor.flush()
    with open(fifo_plate_path, 'w') as fifonum:
    	print("get num:" + p_num);
    	fifonum.write(p_num)
    	fifonum.flush()
    with open(fifo_type_path, 'w') as fifotype:
    	print("get type:" + p_type);
    	fifotype.write(p_type)
    	fifotype.flush()
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
