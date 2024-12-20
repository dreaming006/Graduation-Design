import threading
import serial
import paho.mqtt.client as mqtt
import time
from queue import Queue
from datetime import datetime

loraA_Addr = [0X02,0XCA]
loraA_Chan = [0x17]

Car_A = loraA_Addr+loraA_Chan
Car_B = [0X0A, 0X29, 0X17]
CT_A = [0X0A, 0X9F, 0X17]

MsgfromApp_Q = Queue(10)
MsgToApp_Q  = Queue(10)
MsgfromLora_Q = Queue(10)
MsgToLora_Q = Queue(10)

current_label = ""

class MQTT():
    def __init__(self,lora_service):
        self.client = None
        self.broker = "10wv1pa465244.vicp.fun"
        self.port = 12562
        self.keepalive = 60
        self.PUB_TOPIC = "Catstatus"
        self.SUB_TOPIC = "Catcontrol"
        self.APP_data = None
        self.status = 'disconnect'
        self.lora_service = lora_service 

        # 当客户端收到服务器的CONNACK响应时的回调
    def Start_MQTT_Service(self):
        try:
            self.client = mqtt.Client()
            self.client.username_pw_set("gateway2", "hn123456")
            self.client.on_connect = self.on_connect
            self.client.on_message = self.on_message
            self.client.on_publish = self.on_publish
            self.client.on_disconnect = self.on_disconnect
            print("Connecting to MQTT Broker...")
            self.client.connect(self.broker, self.port, self.keepalive)
        except Exception as e:
            print("error:",e)

        self.client.loop_start()

        # t1 = threading.Thread(target = self.MQTT_Publish, args=())  
        # t1.start()
        print("MQTT Service Started")
    # 定义回调函数，当客户端收到服务器的 CONNACK 响应时的回调
    def on_connect(self,client, userdata, flags, rc):
        print(f"Connected with result code {rc}")
        if rc == 0:
            print("Connected to MQTT Broker!")
            # 连接成功后订阅主题
            self.client.subscribe(self.SUB_TOPIC)
        else:
            print(f"Failed to connect, return code {rc}")

    # 定义回调函数，当从服务器收到 PUBLISH 消息时的回调
    def on_message(self,client, userdata, msg):
        self.APP_data = msg.payload.decode('UTF-8')[0:-2]
        device = self.APP_data[0]
        if int(self.APP_data[1:3]) < 13:
            target_label = self.APP_data[1:3]
            if current_label < target_label:        # 比较当前标签与目标标签
                self.APP_data = device + '31' + target_label        #前进
            else:
                self.APP_data = device + '30' + target_label        #后退
        MsgToLora_Q.put(self.APP_data)
        print(f"Received message from topic '{msg.topic}': {msg.payload.decode()[0:-2]}")
    # 定义回调函数，当客户端发布消息成功时的回调
    def on_publish(self,client, userdata, mid):
        current_time = datetime.now()
        formatted_time = current_time.strftime('%H:%M:%S.%f')  # %f 表示微秒（6 位）
        formatted_time = formatted_time[:-3]  # 截取前 3 位，得到毫秒
        print(f"Message published (mid: {mid})(time: {formatted_time})")
        print()
    # 定义回调函数，当客户端断开连接时的回调
    def on_disconnect(self,client, userdata, rc):
        print("Disconnected from MQTT Broker")
        print("连接断开，尝试重连...")
        self.reconnect()
    # 重连函数
    def reconnect(self):
        while True:
            try:
                print("尝试重新连接...")
                self.client.reconnect()  # 重新连接
                break
            except Exception as e:
                print(f"重连失败: {e}")
                time.sleep(5)  # 等待 5 秒后重试
    
    def MQTT_Publish(self):         # 发布消息到MQTT服务器
        while True:
            if not MsgToApp_Q.empty():
                data = MsgToApp_Q.get()
                self.client.publish(self.PUB_TOPIC, data)
                
                
class LORA():
    def __init__(self,mqtt_service):
        self.Com = '/dev/ttyUSB0'
        self.Bps = 115200
        self.timeout = 0.5
        self.connectflag = False
        self.status = 'disconnect'
        self.t1flag = False
        self.t2flag = False                
        self.exit_event = threading.Event()
        self.mqtt_service = mqtt_service 
    def Start_Lora_Service(self):
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
        t1 = threading.Thread(target = self.recv_serial_info, args=(self.Serial_port,))
        t2 = threading.Thread(target = self.send_serial_info, args=(self.Serial_port,))
        t1.start()
        t2.start()
    
    #接收lora数据    
    def recv_serial_info(self, handle):
        global MsgToApp_Q
        global current_label
        while True:
            if handle.isOpen():
                try:
                    rsv_data = handle.readline()
                    if rsv_data != b'':
                        if rsv_data[:3] == bytes(Car_A):        #判断数据来自哪个车
                            current_time = datetime.now()
                            formatted_time = current_time.strftime('%H:%M:%S.%f')  # %f 表示微秒（6 位）
                            formatted_time = formatted_time[:-3]  # 截取前 3 位，得到毫秒
                            print(f'Recv from Lora A::{rsv_data[3:]}(time:{formatted_time})')
                            rsv_data = rsv_data[3:].decode('UTF-8')[:-2]
                            current_label = rsv_data[5:7]
                            self.mqtt_service.client.publish(self.mqtt_service.PUB_TOPIC, rsv_data)
                            #MsgToApp_Q.put(rsv_data)          # 将接收到的数据放入共享队列
                except Exception as e:
                    print(e)
            else:
                self.connectflag = False
                print("串口未打开")
                break       
        return rsv_data  
    #发送数据到lora
    def send_serial_info(self, handle):
        data = []
        end = [0X0D, 0X0A]
        while True:
            if handle.isOpen():
                if not MsgToLora_Q.empty():
                    info = MsgToLora_Q.get()
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
                        current_time = datetime.now()
                        formatted_time = current_time.strftime('%H:%M:%S.%f')  # %f 表示微秒（6 位）
                        formatted_time = formatted_time[:-3]  # 截取前 3 位，得到毫秒
                        print(f'Msg send to Lora:{info}(time:{formatted_time})')
                        data.clear()
                    except Exception as e:
                            print(e)   
            else:
                self.connectflag = False
                print("串口未打开")
                break


MQTT_Service = MQTT(None) 
Lora_Service = LORA(MQTT_Service)
MQTT_Service.lora_service = Lora_Service


Lora_Process = threading.Thread(target=Lora_Service.Start_Lora_Service,args=())
MQTT_Process = threading.Thread(target=MQTT_Service.Start_MQTT_Service,args=())

Lora_Process.start()
MQTT_Process.start()