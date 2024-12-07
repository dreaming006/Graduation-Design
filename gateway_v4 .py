import sys
import time
import multiprocessing
import threading
import serial
import paho.mqtt.client as mqtt
from queue import Queue
#time.sleep(30)
model  = ''
nowlabel_q = Queue(1)
nowlabel = '00'
nowdata_dict = {'A':''}
device_list = ['A']
status = 'disconnect'

Car_B = [0X0A, 0X29, 0X17]
Car_A = [0X0E, 0X4E, 0X17]
CT_A = [0X0A, 0X9F, 0X17]

ATOG_Q = Queue(1)
GTOA_Q = Queue(1)
LTOG_Q = Queue(1)
GTOL_Q = Queue(1)
status_q = Queue(1)
STOP_Q = Queue(1)
autostop_q = Queue(1)
mqtt_q = Queue(1)
lock = threading.Lock()
event=threading.Event()
sevent=threading.Event()
aevent=threading.Event()
class LORA():
    def __init__(self):
        self.Com = '/dev/ttyUSB0'
        self.Bps = 9600
        self.timeout = 0.5
        self.connectflag = False
        self.status = 'disconnect'
        self.t1flag = False
        self.t2flag = False                
        
    def Start_Lora_Service(self):
        #lora_lock = threading.Lock()
        #while True:
        self.Serial_port = serial.Serial(self.Com, self.Bps, timeout = self.timeout)
        if self.Serial_port.isOpen():
            self.connectflag = True
            print("串口已连接")
        else:
            try:
                self.Serial_port.open()
            except Exception as e:
                print(e)
        
        self.t1flag = True
        self.t2flag = True
        t1 = threading.Thread(target = self.LoRa_FGTL, args=(self.Serial_port,))
        t2 = threading.Thread(target = self.LoRa_FLTG, args=(self.Serial_port,))
        t1.start()
        t2.start()
        
    def recv_serial_info(self, handle):
        while True:
            if handle.isOpen():
                try:
                    rsv_data = handle.readline()
                    if rsv_data == b'':
                        break
                    else:
                        rsv_data = rsv_data.decode('UTF-8')
                        rsv_data = str(rsv_data[0:-2])
                        current_time = time.strftime('%Y-%m-%d %H:%M:%S')
                        print(f'Lora received --> {current_time} - {rsv_data}')
                        break
                except Exception as e:
                    print(e)
            else:
                self.connectflag = False
                print("串口未打开")          
        return rsv_data
    
    def send_serial_info(self, handle, info):
        data = []
        end = [0X0D, 0X0A]
        if handle.isOpen():
            if info[0] == 'A':
                addr = Car_A
            elif info[0] == 'B':
                addr = Car_B
            elif info[0] == 'C':
                addr = CT_A
            for i in range(len(info)-1):
                data.append(int(info[i+1],base=16)+48)
            msg = addr + data + end
            try:
                handle.write(msg)
                #current_time = time.strftime('%Y-%m-%d %H:%M:%S')
                #print(f'Lora send --> {current_time} - {msg}')
            except Exception as e:
                    print(e)
        else:
            self.connectflag = False
            print("串口未打开")    
    #From Gateway To Lora    
    def LoRa_FGTL(self, ser):
        global GTOL_Q
        while True:
            num = GTOL_Q.qsize()
            if num > 0:
                data = GTOL_Q.get()
                #print('LoRa_FGTL',data)
                self.send_serial_info(ser, data)
            
        #print(f'lora: fatl thread over')
    #From Lora To Gateway   
    def LoRa_FLTG(self, ser):
        global LTOG_Q
        while True:
            lora_data = self.recv_serial_info(ser)
            if len(lora_data) != 0:
                #nowdata_dict[lora_data[0]] = lora_data
                LTOG_Q.queue.clear()
                LTOG_Q.put(lora_data)         
        #print(f'lora: flta thread over')        
        
class MQTT():
    def __init__(self):
        self.client = None
        self.broker = "192.168.6.1"
        self.port = 1883
        self.keepalive = 6000
        self.topic1 = "Catstause"
        self.topic2 = "Catcontrol"
        self.APP_data = None
        self.status = 'disconnect'
    # 当客户端收到服务器的CONNACK响应时的回调
    def Start_MQTT_Service(self):
        while True:
            try:
                self.client = mqtt.Client()
                self.client.username_pw_set("gateway2", "hn123456")
                self.client.on_connect = self.on_connect
                self.client.on_message = self.on_message
                self.client.connect(self.broker, self.port, self.keepalive)
                self.client.subscribe(self.topic2)
                time.sleep(1)
                if self.status == 'connect':
                    break   
            except Exception as e:
                print("error:",e)
                continue      
            t1 = threading.Thread(target = self.MQTT_FATG, args=())
            t2 = threading.Thread(target = self.MQTT_FGTA, args=())                                                                        
            t1.start()
            t2.start()
            
        #while True:
           
                #pass
            #else:
                #break
            
    def on_connect(self, client, userdata,flags, rc):
        global status_q
        print("Connected with result code "+str(rc))
        if rc==0:
            self.status = 'connect'
            status_q.put(self.status) 
        # 在on_connect()中订阅意味着，如果我们失去连接并重新连接，那么订阅将被更新。
        #client.subscribe(topic)    
    # 当从服务器收到PUBLISH消息时的回调。
    def on_message(self, client, userdata, msg):
        global ATOG_Q
        self.APP_data = msg.payload.decode('UTF-8')[0:-2]
        if self.APP_data != 'quit':
            ATOG_Q.queue.clear()
            ATOG_Q.put(self.APP_data)
            event.set()
            #num = ATOG_Q.qsize()
            #print(num) 
            print(msg.topic+" "+ self.APP_data)     
        return self.APP_data
    
    #From App To Gateway   
    def MQTT_FATG(self):
        self.client.loop_forever()
               
    #From Gateway To App        
    def MQTT_FGTA(self):
        global GTOA_Q
        while True:
            GTOA_Q_num = GTOA_Q.qsize()
            if GTOA_Q_num > 0:
                data = GTOA_Q.get()
                #print('MQTT_FGTA',data)
                self.client.publish(self.topic1, data)
                
     
            #lock.acquire()
            #if self.connectflag == False:
            #    self.t2flag = False
            #    lock.release()
            #    break
            #lock.release()
        #print(f'mqtt: flta thread over')   


    
def STOP():
    global ATOG_Q,GTOL_Q,LTOG_Q,GTOA_Q,STOP_Q,lock,autostop_q
    print('STOP stat')
    STOP_Q.queue.clear()
    autostop_q.queue.clear()
    while True:
        event.wait(1)       
        if event.isSet():
            ATOG_Q_num = ATOG_Q.qsize()
            if ATOG_Q_num > 0:
                print('stop ATOG_Q_num:',ATOG_Q_num)
                lock.acquire()
                try:
                    app_data = ATOG_Q.get(timeout=0.5)                    
                    if app_data[1:3] == '90':
                        GTOL_Q.queue.clear()
                        GTOL_Q.put(app_data)
                        lora_data = LTOG_Q.get()
                        print('stop lora_data',lora_data)
                        GTOA_Q.put(lora_data)
                        nowdata_dict['A'] = lora_data
                        STOP_Q.put('STOP')
                        #print('stop appdata:'+app_data)
                        event.clear()
                        sevent.set()
                        lock.release()
                        break
                    else:
                        print('s1',app_data)
                        event.clear()
                        lock.release()     
                except:
                    print('stop:null')
                    event.clear()
                    lock.release()
            else:                
                event.clear()   
        aevent.wait(1)
        if aevent.isSet():
            autostop_q_num = autostop_q.qsize()
            if autostop_q_num !=0:
                data = autostop_q.get()
                if data == 'STOP':
                    aevent.clear()
                    break
            aevent.clear()
        
        model = nowdata_dict['A'][1:5]
        if model == '0000' or model == '9911'or model == '9811':
            print('ss:',model)
            break
        else:
            pass
    print('STOP out')
    
    
def Control_run():
    global STOP_Q,lock
    STOP_P = threading.Thread(target=STOP,args=())
    STOP_P.start()
    if STOP_P.is_alive():
        print('stop_p:',STOP_P.is_alive())
    else:
        print('stop wait')
        while True:
            time.sleep(0.1)
            if STOP_P.is_alive():
                break
    while True:
        #event.clear()
        #sevent.clear()
        #aevent.clear()
        if event.isSet()==False:
            Polling()
        lock.acquire()    
        lora_data = nowdata_dict['A']
        if lora_data[1:5] == '9911' or lora_data[1:5] == '9090' :
            autostop_q.put('STOP')
            aevent.set()
            lock.release()
            break
        lock.release()
        sevent.wait(1)
        if sevent.isSet():
            STOP_Q_num = STOP_Q.qsize()
            if STOP_Q_num != 0:
                data = STOP_Q.get()
                if data == 'STOP':
                    sevent.clear()
                    break
            sevent.clear()
    #Model_Control()
    
def Auto_run():
    global nowdata_dict,autostop_q,GTOA_Q,STOP_Q,lock
    STOP_P = threading.Thread(target=STOP,args=())
    STOP_P.start()
    while True:
        #event.clear()
        #sevent.clear()
        if event.isSet()==False:
            Polling()
        lora_data = nowdata_dict['A']
        if lora_data[1:5] == '9811' or lora_data[1:5] == '9090' :
            GTOA_Q.put(lora_data)
            autostop_q.put('STOP')
            aevent.set()
            break
        if event.isSet()==False:
            Polling()
        sevent.wait(1)
        if sevent.isSet():
            STOP_Q_num = STOP_Q.qsize()
            if STOP_Q_num != 0:
                lock.acquire()
                data = STOP_Q.get()
                if data == 'STOP':
                    print('STOP')
                    lock.release()
                    sevent.clear()
                    break
                else:
                    lock.release()
                    sevent.clear()
            sevent.clear()
        
    #Model_Auto()
        
    
def Model_Control():
    global ATOG_Q,GTOL_Q,LTOG_Q,GTOA_Q,STOP_Q,lock
    while True:
        if event.isSet()==False:
            Polling()
        model = nowdata_dict['A'][1:5]
        if model == '0000':
            break
        elif model == '1111' or model == '1010':
            Control_run() 
        #event.clear()
        event.wait(1)
        f=event.isSet()
        if event.isSet():
            print('cont:',f)
            ATOG_Q_num = ATOG_Q.qsize()
            print('anum:',ATOG_Q_num)
            if ATOG_Q_num!=0:
                lock.acquire()
                try:
                    app_data = ATOG_Q.get(timeout=0.5)
                    if app_data[1:3] == '11' or app_data[1:3] == '10' or app_data[1:3] == '99':
                        GTOL_Q.queue.clear()
                        GTOL_Q.put(app_data)
                        lora_data = LTOG_Q.get()
                        print('GTOL_Q',app_data)
                        nowdata_dict['A'] = lora_data
                        GTOA_Q.put(lora_data)
                        print('GTOA_Q',lora_data)
                        if lora_data[1:5] == '1111' or lora_data[1:5] == '1010':
                            print('run')
                            event.clear()
                            lock.release()
                            Control_run()
                            print('?')
                        elif lora_data[1:5] == '9901':
                            event.clear()
                            lock.release()
                            break
                        else:
                            print('1',app_data)
                            event.clear()
                            lock.release()                            
                    else:
                        print('2',app_data)
                        event.clear()
                        lock.release()
                        
                except:
                    print('con:null')
                    lock.release()
                    event.clear()
            event.clear()
        
def Model_Auto():
    global ATOG_Q,GTOL_Q,LTOG_Q,GTOA_Q,STOP_Q,nowdata_dict,autostop_q,lock
    while True:
        if event.isSet()==False:
            Polling()
        model = nowdata_dict['A'][1:5]
        if model == '0000':
            break
        
        event.wait(1)
        if event.isSet():
            ATOG_Q_num = ATOG_Q.qsize()
            if ATOG_Q_num != 0:
                lock.acquire()
                try:
                    app_data = ATOG_Q.get(timeout=0.5)
                    if app_data[1:3] == '98':
                        GTOL_Q.put(app_data)
                        lora_data = LTOG_Q.get()
                        if lora_data[1:5] == '9801':
                            nowdata_dict['A'] = lora_data
                            GTOA_Q.put(lora_data)
                            event.clear()
                            lock.release()
                            break
                        else:
                            event.clear()
                            lock.release()
                            
                    elif int(app_data[1:3]) < 13:
                        device = app_data[0]
                        Car_A = nowdata_dict['A'] 
                        now =Car_A[5:7]
                        destination_label = app_data[1:3]                       
                        if int(now)<int(destination_label):
                            data = device + '11' + destination_label
                        else:
                            data = device + '10' + destination_label
                        GTOL_Q.put(data)
                        lora_data = LTOG_Q.get()
                        nowdata_dict['A'] = lora_data
                        GTOA_Q.put(lora_data)
                        event.clear()
                        lock.release()
                        print('auto')
                        Auto_run()
                    else:
                        event.clear()
                        lock.release()                       
                except:
                    event.clear()
                    lock.release()                    
            else:
                time.sleep(0.1)
            event.clear()
    print('auto back')                      
def Polling():
    global GTOL_Q,LTOG_Q,GTOA_Q,device_list, nowdata_dict,lock
    for x in device_list:
        time.sleep(0.1)
        lock.acquire()
        flag = event.isSet()
        if flag:
            print(flag)
            lock.release()
            break
        else:
            GTOL_Q.put((x+'00'))
            try:
                lora_data = LTOG_Q.get(timeout=0.5)
                nowdata_dict[lora_data[0]] = lora_data
                #lora_data=nowdata_dict[x]
                lock.release()
                if status == 'connect':
                    GTOA_Q.put(lora_data)
            except:
                lock.release()    
            print('P:',x)
            

if __name__ == '__main__':
    
    MQTT_Service = MQTT()   
    Lora_Service = LORA()
    MQTT_Process = threading.Thread(target=MQTT_Service.Start_MQTT_Service,args=())
    Lora_Process = threading.Thread(target=Lora_Service.Start_Lora_Service,args=())   
    MQTT_Process.start()
    Lora_Process.start()
    event.clear()
    while True:
        status_q_num = status_q.qsize()
        if status_q_num > 0:
            status = status_q.get()
            if status == 'connect':
                #Polling_th = threading.Thread(target=Polling,args=())
                #Polling_th.start()
                while True:
                    event.clear()
                    time.sleep(0.1)
                    if event.isSet()==False:
                        Polling()
                    model = nowdata_dict['A'][1:5]                    
                    if model == '0000':
                        while True:
                            if event.isSet()==False:
                                Polling()
                            model = nowdata_dict['A'][1:5]
                            if model == '9911':
                                Model_Control()
                            elif model == '9811':
                                Model_Auto()
                            event.wait(1)
                            if event.isSet():
                                ATOG_Q_num = ATOG_Q.qsize()
                                if ATOG_Q_num > 0:
                                    lock.acquire()
                                    try:
                                        app_data = ATOG_Q.get(timeout=0.5)
                                        print(app_data)
                                        if app_data[1:3] == '99' or app_data[1:3] == '98':
                                            GTOL_Q.queue.clear()
                                            GTOL_Q.put(app_data)
                                            lora_data = LTOG_Q.get()
                                            nowdata_dict['A'] = lora_data
                                            print(lora_data)
                                            GTOA_Q.put(lora_data)
                                            print('main')
                                            if lora_data[1:5] == '9911':
                                                lock.release()
                                                event.clear()
                                                Model_Control()
                                            elif lora_data[1:5] == '9811':
                                                lock.release()
                                                event.clear()
                                                Model_Auto()
                                            else:
                                                lock.release()
                                                event.clear()
                                        else:
                                            print('m1',app_data)
                                            lock.release()
                                            event.clear()
                                    except:
                                        lock.release()
                                        event.clear()
                                else:
                                    time.sleep(0.1)
                                event.clear()
                    elif model == '9911':
                        Model_Control()
                    elif model == '9811':
                        Model_Auto()
                    elif model == '1111' or model == '1010':
                        Control_run()
                    elif model == '2222' or model == '2020':
                        Auto_run()
        
