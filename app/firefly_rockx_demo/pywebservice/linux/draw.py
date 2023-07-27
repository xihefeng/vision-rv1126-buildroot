import tkinter
import _thread
import time
from tkinter import END,INSERT
import os,sys

width=360
height=120
dst_x=780
dst_y=480
font_width=width/2
font_height=height/2
canvas_t=[]
text_t=[]
line_t=[]

fifo_count_name = 'count'
fifo_color_name = 'color'
fifo_plate_name = 'plate'
fifo_type_name = 'type'

fifo_file_folder='fifo_file'
os.makedirs(fifo_file_folder, exist_ok=True)
fifo_file_path = os.path.join(os.getcwd(), fifo_file_folder)
fifo_count_path = os.path.join(fifo_file_path, fifo_count_name)
fifo_color_path = os.path.join(fifo_file_path, fifo_color_name)
fifo_plate_path = os.path.join(fifo_file_path, fifo_plate_name)
fifo_type_path = os.path.join(fifo_file_path, fifo_type_name)

def update():
	while 1 :
		with open(fifo_count_path) as fifocount:
			count=int(fifocount.read())
		with open(fifo_color_path) as fifocolor:
			colorall=fifocolor.read()
		with open(fifo_plate_path) as fifonum:
			numall=fifonum.read()
		with open(fifo_type_path) as fifotype:
			typeall=fifotype.read()
		num=numall.split(",")
		color=colorall.split(",")
		p_type=typeall.split(",")
		for i in range(0,count):
			if ( color[i] == 'yellow' and num[i] != 'unknow' and p_type[i] != 'unknow'):
				canvas_t[i].config(bg='yellow')
				canvas_t[i].itemconfig(text_t[i],text=num[i],fill="black")
				canvas_t[i].itemconfig(line_t[i],fill="black")
			elif ( color[i] == 'blue' and num[i] != 'unknow' and p_type[i] != 'unknow'):
				canvas_t[i].config(bg='blue')
				canvas_t[i].itemconfig(text_t[i],text=num[i],fill="white")
				canvas_t[i].itemconfig(line_t[i],fill="white")
			elif ( color[i] == 'green' and num[i] != 'unknow' and p_type[i] != 'unknow'):
				canvas_t[i].config(bg='green')
				canvas_t[i].itemconfig(text_t[i],text=num[i],fill="black")
				canvas_t[i].itemconfig(line_t[i],fill="black")
			else:
				canvas_t[i].config(bg='red')
				canvas_t[i].itemconfig(text_t[i],text="数据无效",fill="black")
		for i in range(count,9):
			canvas_t[i].config(bg='gray')
			canvas_t[i].itemconfig(text_t[i],text="Firefly",fill="black")
			canvas_t[i].itemconfig(line_t[i],fill="gray")
		print(num)
		print(color)
		print(p_type)
		time.sleep(0.5)

#窗口初始化
if __name__ == '__main__':
	try:
		os.mkfifo(fifo_count_path)
	except:pass
	try:
		os.mkfifo(fifo_color_path)
	except:pass
	try:
		os.mkfifo(fifo_plate_path)
	except:pass
	try:
		os.mkfifo(fifo_type_path)
	except:pass
	t = tkinter.Tk()
	t.title("License plate recognition system")
	t.geometry("%dx%d+%d+%d" % (width+width/72,height+width/72,dst_x,dst_y))
	cwidth=width/3
	cheight=height/3
	ratex=24
	ratey=8
	for i in range(0,9):
		canvas_t.append(tkinter.Canvas(t,width=cwidth,height=cheight,bg='gray'))
		canvas_t[i].grid(row=int(i/3),column=int(i%3))
		text_t.append(canvas_t[i].create_text(font_width/3, font_height/3, text='Firefly',font=("Helvetica",15)))
		line_t.append(canvas_t[i].create_line(cwidth/ratex,cwidth/ratex,cwidth/ratex,cheight/ratey*(ratey-1),cwidth/ratex*(ratex-1),cheight/ratey*(ratey-1),cwidth/ratex*(ratex-1),cwidth/ratex,cwidth/ratex,cwidth/ratex,fill='gray',width=cwidth/60))
	t.wait_visibility(t) #这是兼容linux的方法
	t.wm_attributes('-topmost', True) #窗口置顶
	_thread.start_new_thread( update,()  )
	t.mainloop()
