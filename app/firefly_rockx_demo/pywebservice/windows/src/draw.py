import tkinter
import _thread
import time
from tkinter import END,INSERT
import os,sys
import win32file
import win32pipe
import pywintypes
width=360
height=120
dst_x=780
dst_y=480
font_width=width/2
font_height=height/2
canvas_t=[]
text_t=[]
line_t=[]

count_pipe = 0
color_pipe = 0
plate_pipe = 0
type_pipe = 0

def update():
        global count_pipe
        global color_pipe
        global plate_pipe
        global type_pipe
        print("waiting for count server")
        win32pipe.ConnectNamedPipe(count_pipe, None)
        print("got count_pipe")
        print("waiting for color server")
        win32pipe.ConnectNamedPipe(color_pipe, None)
        print("got color_pipe")
        print("waiting for plate server")
        win32pipe.ConnectNamedPipe(plate_pipe, None)
        print("got plate_pipe")
        print("waiting for type server")
        win32pipe.ConnectNamedPipe(type_pipe, None)
        print("got type_pipe")
        while 1 :
                rc,count = win32file.ReadFile(count_pipe, 64*1024)
                count = int(count)
                #print(count)
                rc,colorall = win32file.ReadFile(color_pipe, 64*1024)
                colorall = (colorall.decode('gbk'))
                #print(colorall)
                rc,numall = win32file.ReadFile(plate_pipe, 64*1024)
                numall = (numall.decode('gbk'))
                #print(numall)
                rc,typeall = win32file.ReadFile(type_pipe, 64*1024)
                typeall = (typeall.decode('gbk'))
                #print(typeall)
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
                #time.sleep(0.2)

#窗口初始化
if __name__ == '__main__':

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
        count_pipe = win32pipe.CreateNamedPipe(
                        r'\\.\pipe\count',
                        win32pipe.PIPE_ACCESS_DUPLEX,
                        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
                        1, 65536, 65536,
                        0,
                        None)
        color_pipe = win32pipe.CreateNamedPipe(
                        r'\\.\pipe\color',
                        win32pipe.PIPE_ACCESS_DUPLEX,
                        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
                        1, 65536, 65536,
                        0,
                        None)
        plate_pipe = win32pipe.CreateNamedPipe(
                        r'\\.\pipe\plate',
                        win32pipe.PIPE_ACCESS_DUPLEX,
                        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
                        1, 65536, 65536,
                        0,
                        None)
        type_pipe = win32pipe.CreateNamedPipe(
                        r'\\.\pipe\type',
                        win32pipe.PIPE_ACCESS_DUPLEX,
                        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
                        1, 65536, 65536,
                        0,
                        None)
        _thread.start_new_thread( update,()  )
        t.mainloop()
