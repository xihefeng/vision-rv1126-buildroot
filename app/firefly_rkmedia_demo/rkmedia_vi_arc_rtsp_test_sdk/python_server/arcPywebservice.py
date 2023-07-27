import flask
import os

fifo_count_name = 'count'
fifo_MacAddress_name = 'MacAddress'
fifo_IpAddress_name = 'IpAddress'
fifo_Faceid_name = 'Faceid'
fifo_FaceName_name = 'name'
fifo_img_name = 'img'

fifo_file_folder="fifo_file"
os.makedirs(fifo_file_folder, exist_ok=True)

fifo_file_path = os.path.join(os.getcwd(), fifo_file_folder)

fifo_count_path = os.path.join(fifo_file_path, fifo_count_name)
fifo_MacAddress_path = os.path.join(fifo_file_path, fifo_MacAddress_name)
fifo_IpAddress_path = os.path.join(fifo_file_path, fifo_IpAddress_name)
fifo_Faceid_path = os.path.join(fifo_file_path, fifo_Faceid_name)
fifo_FaceName_path = os.path.join(fifo_file_path, fifo_FaceName_name)
fifo_img_path = os.path.join(fifo_file_path, fifo_img_name)

server=flask.Flask(__name__)#__name__代表当前的python文件。把当前的python文件当做一个服务启动
@server.route('/arc_face',methods=['get','post'])
def loadfile():
    p_count = flask.request.values.get('count')
    p_MacAddress = flask.request.values.get('MacAddress')
    p_IpAddress = flask.request.values.get('IpAddress')
    p_Faceid = flask.request.values.get('Faceid')
    p_name = flask.request.values.get('name')
    p_img = flask.request.values.get('img')
    with open(fifo_count_path, 'w') as fifocount:
    	print("get count:" + p_count);
    	fifocount.write(p_count)
    	fifocount.flush()
    with open(fifo_MacAddress_path, 'w') as fifoMacAddress:
    	print("get MacAddress:" + p_MacAddress);
    	fifoMacAddress.write(p_MacAddress)
    	fifoMacAddress.flush()
    with open(fifo_IpAddress_path, 'w') as fifoIpAddress:
    	print("get IpAddress:" + p_IpAddress);
    	fifoIpAddress.write(p_IpAddress)
    	fifoIpAddress.flush()
    with open(fifo_Faceid_path, 'w') as fifoFaceid:
    	print("get Faceid:" + p_Faceid);
    	fifoFaceid.write(p_Faceid)
    	fifoFaceid.flush()
    with open(fifo_FaceName_path, 'w') as fifoFaceName:
    	print("get name:" + p_name);
    	fifoFaceName.write(p_name)
    	fifoFaceName.flush()
    with open(fifo_img_path, 'w') as fifoImg:
    	print("get img:" + p_img);
    	fifoImg.write(p_img)
    	fifoImg.flush()
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
