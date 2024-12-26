import threading
import serial
import paho.mqtt.client as mqtt
import time
from queue import Queue
from datetime import datetime, timedelta


LoraControlCmd_Q = Queue(10)

DataUploadCmd_Q = Queue(10)

DeviceAddr_List ={
    "A": [0X02,0XCA],
    "B": [0X0A, 0X29],
    "C": [0X0A, 0X9F],
}

LoraChan = [0x17]

"""
99为控制模式指令
98为预设模式指令
31为前进
30为后退
90为停止            
"""
ControlCmdAck_Dict = {
    "99": {"C9911", "C9901"},     #进入/退出控制模式应答
    "98": {"C9811", "C9801"},     #进入/退出预设模式应答
    "31": {"C1111", "C2222"},     #控制和预设模式的前进应答
    "30": {"C1010", "C2020"},     #控制和预设模式的后退应答
    "90": {"C9090"},              #停止应答
}

"""
00为数据上传指令
"""
DataUploadCmdAck_Dict = {
    "00": {"D9911",                 #处于控制模式，未移动           
           "D9811",                 #处于预设模式，未移动
           "D1111", "D1010",        #处于控制模式的前进/后退状态
           "D2222", "D2020",        #处于预设模式的前进/后退状态
           "D0000"},                #无动作应答
}


RecvAppPackCnt = 0
RecvLoraAckCnt = 0
SendPackCnt = 0

Average_response_time = 0.0
sum_time = 0.0

response_flag = False

class MQTT():
    def __init__(self,lora_service):
        self.client = None
        self.broker = "10wv1pa465244.vicp.fun"
        self.port = 12562
        #self.broker = "192.168.137.34"
        #self.port = 1883
        self.keepalive = 60
        self.PUB_TOPIC = "Catstatus"
        self.SUB_TOPIC = "Catcontrol"
        self.APP_data = None
        self.status = 'disconnect'
        self.lora_service = lora_service 
        self.RecvTime = datetime.now()
        self.PublishTime = datetime.now()
 
        # 当客户端收到服务器的CONNACK响应时的回调
    def Start_MQTT_Service(self):
        #try:
            self.client = mqtt.Client()
            self.client.username_pw_set("gateway2", "hn123456")
            self.client.on_connect = self.on_connect
            self.client.on_message = self.on_message
            self.client.on_publish = self.on_publish
            self.client.on_disconnect = self.on_disconnect
            print("Connecting to MQTT Broker...")
            self.client.connect(self.broker, self.port, self.keepalive)
        # except Exception as e:
        #     print("Start_MQTT_Service error:",e)

            self.client.loop_start()
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
        global RecvAppPackCnt
        RecvMsg = msg.payload.decode('UTF-8')[0:-2]
        data = []
        DataUploadCmd = [0x30,0x30]
        end = [0X0D, 0X0A]    
        device = RecvMsg[0]
        addr = DeviceAddr_List[device] + LoraChan    #根据设备和信道生成地址
        if RecvMsg[1:3]=="00":  
            DataUploadCmd_Q.put(addr+DataUploadCmd+end)
        else:    
            if RecvMsg[1:4] == "tag":                                       #预设目标标签       
                target_label = RecvMsg[4:6]
                if self.lora_service.current_label < target_label:          # 比较当前标签与目标标签
                    ControlCmd = '31' + target_label                        #前进
                    self.lora_service.control_cmd = '31'
                elif self.lora_service.current_label > target_label:   
                    ControlCmd = '30' + target_label                        #后退
                    self.lora_service.control_cmd = '30'
                elif self.lora_service.current_label == target_label:
                    #self.client.publish(self.PUB_TOPIC, f"{RecvMsg[0]}8080{target_label}")    #回复APP已抵达目标标签
                    return
            else:                                                           #控制指令
                ControlCmd = RecvMsg[1:3]
                self.lora_service.control_cmd = ControlCmd
            for i in range(len(ControlCmd)):
                data.append(int(ControlCmd[i],base=16)+48)                
            msgtolora = addr + data + end    
            LoraControlCmd_Q.put(msgtolora)

        data.clear()
        RecvAppPackCnt += 1
        self.RecvTime = datetime.now()
        self.lora_service.send_event.set()
        formatted_time = self.RecvTime.strftime('%H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
        print(f"Recv msg from APP:{msg.payload.decode()[0:-2]}(time: {formatted_time})")
    # 定义回调函数，当客户端发布消息成功时的回调
    def on_publish(self,client, userdata, mid):
        global RecvAppPackCnt,SendPackCnt,RecvLoraAckCnt
        global Average_response_time,sum_time
        global response_flag
        self.PublishTime = datetime.now()
        formatted_time = self.PublishTime.strftime('%H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
        print(f"Message published (mid: {mid})(time: {formatted_time})")
        print(f"RecvAppPackCnt:{RecvAppPackCnt} SendPackCnt:{SendPackCnt} RecvLoraAckCnt:{RecvLoraAckCnt} ")
        print(f"Packet loss rate:{((SendPackCnt-RecvAppPackCnt)/SendPackCnt)*100:.2f}%")
        if(response_flag == True):
            sum_time  = (sum_time + (self.PublishTime - self.RecvTime).total_seconds()*1000)
            Average_response_time = sum_time/float(RecvAppPackCnt)
        print(f'Average response time:{Average_response_time:.2f}')
        response_flag = False
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
         
                
class LORA():
    def __init__(self,mqtt_service):
        self.Com = '/dev/ttyUSB0'
        self.Bps = 115200
        self.timeout = 0.5
        self.connectflag = False
        self.current_label = ""
        self.control_cmd = ""
        self.DataUploadCmd = "00"    
        self.ControlCmdResend_cnt = 0
        self.DataUploadCmdResend_cnt = 0
        self.ControlCmdSendTime = datetime.now()
        self.DataUploadCmdSendTime = datetime.now()
        self.ControlCmdRecvTime = datetime.now()
        self.DataUploadCmdRecvTime = datetime.now()                     
        self.recv_event = threading.Event()
        self.send_event = threading.Event()
        self.recv_control_ack = threading.Event()
        self.recv_dataupload_ack = threading.Event()
        self.mqtt_service = mqtt_service 
    
    #接收lora数据    
    def recv_serial_info(self, handle):
        global RecvLoraAckCnt
        global response_flag
        while True:
            self.recv_event.wait()
            #print('recv_serial_info')
            if handle.isOpen():
                rsv_data = handle.readline()
                if rsv_data != b'':
                    #print(rsv_data)
                    if rsv_data[:3] == bytes(DeviceAddr_List['A']+LoraChan):        #判断数据来自哪个车
                        rsv_data = rsv_data[3:].decode('UTF-8')[:-2]
                        current_time = datetime.now()
                        formatted_time = current_time.strftime('%H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                        print(f'Recv from Lora A:{rsv_data}(time:{formatted_time})')
                        self.current_label = rsv_data[5:7]
                        LoraAckData = rsv_data[:5]
                        if self.control_cmd in ControlCmdAck_Dict and LoraAckData in ControlCmdAck_Dict[self.control_cmd]:
                            self.recv_control_ack.set()
                            self.ControlCmdResend_cnt = 0
                            RecvLoraAckCnt += 1
                            response_flag = True  
                        elif LoraAckData in DataUploadCmdAck_Dict[self.DataUploadCmd]:
                            self.recv_dataupload_ack.set()
                            self.DataUploadCmdResend_cnt = 0
                            RecvLoraAckCnt += 1
                            response_flag = True
                        else:
                            response_flag = False
                        self.mqtt_service.client.publish(self.mqtt_service.PUB_TOPIC, rsv_data)
                        current_time = datetime.now()
                        
            else:
                self.connectflag = False
                print("串口未打开")
            self.recv_event.clear()
            self.send_event.set()
    #发送数据到lora
    def send_serial_info(self, handle):
        global SendPackCnt
        while True:
            self.send_event.wait()
            if handle.isOpen():
                if self.recv_control_ack.is_set():
                    current_time = datetime.now()
                    if (current_time - self.DataUploadCmdSendTime).total_seconds()*1000 > 200 or self.recv_dataupload_ack.is_set():
                        if not LoraControlCmd_Q.empty():
                            LoraControlCmd = LoraControlCmd_Q.get()                   
                            try:
                                handle.write(LoraControlCmd)
                                SendPackCnt += 1
                                self.ControlCmdSendTime = datetime.now()
                                formatted_time = self.ControlCmdSendTime.strftime('%H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                                print(f'Lora Control Cmd send :{LoraControlCmd}(time:{formatted_time})')
                                self.recv_control_ack.clear()
                                self.send_event.clear()
                                self.recv_event.set()
                                continue 
                            except Exception as e:
                                    print(f'Serial wirte error:{e}')
                else:
                    current_time = datetime.now()
                    timediff = (current_time - self.ControlCmdSendTime).total_seconds()
                    if timediff > 1.5 and self.ControlCmdResend_cnt < 2:
                        handle.write(LoraControlCmd)
                        self.ControlCmdSendTime = datetime.now()
                        self.ControlCmdResend_cnt += 1
                        formatted_time = self.ControlCmdSendTime.strftime('%H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                        print(f'ReSend Control Cmd:{LoraControlCmd}(time:{formatted_time})')

                    if self.ControlCmdResend_cnt == 2:
                        self.ControlCmdResend_cnt = 0
                        self.recv_control_ack.set()
                        
                    self.send_event.clear()
                    self.recv_event.set()
                    continue 

                if self.recv_dataupload_ack.is_set():
                    if not DataUploadCmd_Q.empty() :    
                        DataUploadCmd = DataUploadCmd_Q.get()
                        try:
                            handle.write(DataUploadCmd)
                            SendPackCnt += 1
                            self.DataUploadCmdSendTime = datetime.now()
                            formatted_time = self.DataUploadCmdSendTime.strftime('%H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                            print(f'Data Upload Cmd send :{DataUploadCmd}(time:{formatted_time})')
                            self.recv_dataupload_ack.clear()
                        except Exception as e:
                                print(f'Serial wirte error:{e}')

                else:
                    current_time = datetime.now()
                    timediff = (current_time - self.DataUploadCmdSendTime).total_seconds()
                    if timediff > 1.5 and self.DataUploadCmdResend_cnt < 2:
                        handle.write(DataUploadCmd)
                        self.DataUploadCmdSendTime = datetime.now()
                        self.DataUploadCmdResend_cnt += 1
                        formatted_time = self.DataUploadCmdSendTime.strftime('%H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                        print(f'ReSend Data Upload Cmd:{DataUploadCmd}(time:{formatted_time})')

                    if self.DataUploadCmdResend_cnt == 2:
                        self.DataUploadCmdResend_cnt = 0
                        self.recv_dataupload_ack.set()
                        
                
                self.send_event.clear()
                self.recv_event.set()
            else:
                self.connectflag = False
                print("串口未打开")
                break


    def Start_Lora_Service(self):
        self.Serial_port = serial.Serial(self.Com, self.Bps, timeout = self.timeout)
        if self.Serial_port.isOpen():
            self.connectflag = True
            print("串口已连接")
        else:
            try:
                self.Serial_port.open()
            except Exception as e:
                print(f'Serial open error:{e}')

        self.send_event.set()
        self.recv_event.clear()
        self.recv_control_ack.set()
        self.recv_dataupload_ack.set()
        t1 = threading.Thread(target = self.recv_serial_info, args=(self.Serial_port,))
        t2 = threading.Thread(target = self.send_serial_info, args=(self.Serial_port,))
        t1.start()
        t2.start()
        

MQTT_Service = MQTT(None) 
LORA_Service = LORA(MQTT_Service)
MQTT_Service.lora_service = LORA_Service


Lora_Process = threading.Thread(target=LORA_Service.Start_Lora_Service,args=())
MQTT_Process = threading.Thread(target=MQTT_Service.Start_MQTT_Service,args=())

Lora_Process.start()
MQTT_Process.start()