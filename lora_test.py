import threading
import serial
import paho.mqtt.client as mqtt
import time
import math
from queue import Queue
from datetime import datetime, timedelta

class CoDelQueue:
    def __init__(self, max_size, target_delay=0.2, interval=0.1):
        self.queue = []
        self.enqueue_time = []
        self.max_size = max_size
        self.target_delay = target_delay  # 目标延迟，单位秒
        self.interval = interval  # 检查间隔，单位秒
        self.drop_count = 0
        self.next_drop_time = time.time()

    def enqueue(self, item):
        if len(self.queue) >= self.max_size:
            self.drop()
            return False
        self.queue.append((item, time.time()))
        return True

    def dequeue(self):
        """
        数据包出队，并检查是否需要丢包。
        :return: 出队的数据包（如果未被丢弃）。
        """
        now = time.time()

        # 检查是否到达丢包检查时间
        if now >= self.next_drop_time:
            self.check_drop(now)

        # 出队第一个数据包
        if self.queue:
            item, enqueue_time = self.queue.pop(0)
            self.enqueue_time.append(enqueue_time)
            return item
        
        return None
    def check_drop(self, now):
        enqueue_time = self.queue[0][1]
        sojourn_time = now - enqueue_time

        # 如果停留时间超过目标延迟，触发丢包
        if sojourn_time > self.target_delay:
            self.drop_count += 1
            print(f"Drop pack(cnt:{self.drop_count})")
            # 丢弃队列中的一个数据包
            if self.queue:
                self.queue.pop(0)
                print(f"Stay time: {sojourn_time:.6f}")

        # 更新下一次丢包检查时间
        self.next_drop_time += self.interval/math.sqrt(self.drop_count)
    def getcmd(self):
        if not self.queue:
            return None
        item, enqueue_time = self.queue.pop(0)
        current_time = time.time()
        sojourn_time = current_time - enqueue_time
        if sojourn_time > self.target_delay:
            print(f'{item} is dropped')
            return None
        else: 
            self.enqueue_time.append(enqueue_time)
            return item

    def gettime(self):
        enqueue_time = self.enqueue_time.pop(0)
        return enqueue_time

    def deltime(self):
        self.enqueue_time.pop(0)
        return
    def empty(self):
        return len(self.queue) == 0

LoraControlCmd_Q = CoDelQueue(10)
DataUploadCmd_Q = CoDelQueue(10)

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

recv_ctrolcmd = False
recv_datauploadcmd = False

class TCPVegas:
    def __init__(self):
        self.ssthresh = 64  # 慢启动阈值
        self.min_rtt = float('inf')  # 最小 RTT
        self.current_rtt = None  # 当前 RTT
        self.cwnd = 1  # 拥塞窗口
        self.alpha = 1  # Vegas 参数 α
        self.beta = 3  # Vegas 参数 β
        self.packets_unacked = 0  # 发送未确认的包数
    def update_rtt(self,rtt):
        # RTT 更新
        self.current_rtt = rtt
        self.min_rtt = min(self.min_rtt, rtt)
        #print(f'current_rtt:{self.current_rtt} min_rtt:{self.min_rtt}')
    def adjust_cwnd(self):
        if self.min_rtt is None or self.current_rtt is None:
            return
        
        # 计算期望吞吐量和实际吞吐量
        expected_throughput = self.cwnd / self.min_rtt
        actual_throughput = self.cwnd / self.current_rtt
        diff = (expected_throughput - actual_throughput)*self.min_rtt
        print(f'diff:{diff}')
        
        # 调整 cwnd
        if diff < self.alpha:
            self.cwnd += 1  # 网络未拥塞，增加 cwnd
        elif diff > self.beta:
            self.cwnd -= 1  # 网络拥塞，减少 cwnd
        # 如果 alpha <= diff <= beta，保持 cwnd 不变
        #print(f'cwnd:{self.cwnd}')

class MQTT():
    def __init__(self,lora_service,TCPVegas):
        self.client = None
        self.broker = "10wv1pa465244.vicp.fun"
        self.port = 24725
        #self.port = 18219
        #self.broker = "192.168.137.34"
        #self.port = 1883
        self.keepalive = 60
        self.PUB_TOPIC = "Catstatus"
        self.SUB_TOPIC = "Catcontrol"
        self.APP_data = None
        self.status = 'disconnect'
        self.lora_service = lora_service 
        self.TCPVegas = TCPVegas 
        self.RecvTime = None
        self.PublishTime = None
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
            DataUploadCmd_Q.enqueue(addr+DataUploadCmd+end)
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
            LoraControlCmd_Q.enqueue(msgtolora)

        data.clear()
        RecvAppPackCnt += 1
        #self.RecvTime.append(datetime.now())
        self.lora_service.send_event.set()
        formatted_time = datetime.now().strftime('%m-%d %H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
        print(f"Recv msg from APP:{msg.payload.decode()[0:-2]}(time: {formatted_time})")
    # 定义回调函数，当客户端发布消息成功时的回调
    def on_publish(self,client, userdata, mid):
        global RecvAppPackCnt,SendPackCnt,RecvLoraAckCnt
        global Average_response_time,sum_time
        global recv_ctrolcmd
        global recv_datauploadcmd
        self.PublishTime = time.time()
        formatted_time = datetime.now().strftime('%m-%d %H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
        print(f"Message published (mid: {mid})(time: {formatted_time})")
        print(f"RecvAppPackCnt:{RecvAppPackCnt} SendPackCnt:{SendPackCnt} RecvLoraAckCnt:{RecvLoraAckCnt} ")
        print(f"Packet loss rate:{((SendPackCnt-RecvLoraAckCnt)/SendPackCnt)*100:.2f}%")
        if recv_ctrolcmd == True:
            self.RecvTime = LoraControlCmd_Q.gettime()
        elif recv_datauploadcmd == True:
            self.RecvTime = DataUploadCmd_Q.gettime() 
        current_rtt = (self.PublishTime - self.RecvTime)*1000
        self.TCPVegas.update_rtt(current_rtt)       # 更新 RTT
        self.TCPVegas.adjust_cwnd()                 # 调整拥塞窗口
        sum_time  = (sum_time + current_rtt)
        Average_response_time = sum_time/float(RecvAppPackCnt)
        print(f'Current/Average :{current_rtt:.2f}/{Average_response_time:.2f}')
        recv_ctrolcmd = False
        recv_datauploadcmd = False
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
class LORA(TCPVegas):
    def __init__(self,mqtt_service,TCPVegas):
        self.Com = '/dev/ttyUSB0'
        self.Bps = 115200
        self.timeout = 0.5
        self.connectflag = False
        self.current_label = ""
        self.control_cmd = ""
        self.DataUploadCmd = "00"    
        self.ControlCmdResend_cnt = 0
        self.DataUploadCmdResend_cnt = 0
        self.packets_unacked = 0  # 发送未确认的包数量
        self.ControlCmdSendTime = datetime.now()
        self.DataUploadCmdSendTime = datetime.now()
        self.ControlCmdRecvTime = datetime.now()
        self.DataUploadCmdRecvTime = datetime.now()                     
        self.recv_event = threading.Event()
        self.send_event = threading.Event()
        self.recv_control_ack = threading.Event()
        self.recv_dataupload_ack = threading.Event()
        self.mqtt_service = mqtt_service 
        self.TCPVegas = TCPVegas
    #接收lora数据    
    def recv_serial_info(self, handle):
        global RecvLoraAckCnt
        global recv_ctrolcmd
        global recv_datauploadcmd
        while True:
            self.recv_event.wait()
            #print('recv_serial_info')
            if handle.isOpen():
                rsv_data = handle.readline()
                if rsv_data != b'':
                    #print(rsv_data)
                    if rsv_data[:3] == bytes(DeviceAddr_List['A']+LoraChan):        #判断数据来自哪个车
                        rsv_data = rsv_data[3:].decode('UTF-8')[:-2]
                        formatted_time = datetime.now().strftime('%m-%d %H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                        print(f'Recv from Lora A:{rsv_data}(time:{formatted_time})')
                        self.current_label = rsv_data[5:7]
                        LoraAckData = rsv_data[:5]
                        if self.control_cmd in ControlCmdAck_Dict and LoraAckData in ControlCmdAck_Dict[self.control_cmd]:
                            self.recv_control_ack.set()
                            self.ControlCmdResend_cnt = 0
                            self.packets_unacked -= 1
                            RecvLoraAckCnt += 1
                            recv_ctrolcmd = True  
                        elif LoraAckData in DataUploadCmdAck_Dict[self.DataUploadCmd]:
                            self.recv_dataupload_ack.set()
                            self.DataUploadCmdResend_cnt = 0
                            self.packets_unacked -= 1
                            RecvLoraAckCnt += 1
                            recv_datauploadcmd = True
                        else:
                            recv_ctrolcmd = False
                            recv_datauploadcmd = False
                        self.mqtt_service.client.publish(self.mqtt_service.PUB_TOPIC, rsv_data)
                        
                        
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
                if self.packets_unacked < self.TCPVegas.cwnd:
                    if self.recv_control_ack.is_set():
                        current_time = datetime.now()
                        if (current_time - self.DataUploadCmdSendTime).total_seconds()*1000 > 200 or self.recv_dataupload_ack.is_set():
                            if not LoraControlCmd_Q.empty():
                                LoraControlCmd = LoraControlCmd_Q.dequeue()                   
                                if LoraControlCmd != None:
                                    try:
                                        handle.write(LoraControlCmd)
                                        SendPackCnt += 1
                                        self.packets_unacked += 1
                                        self.ControlCmdSendTime = datetime.now()
                                        formatted_time = self.ControlCmdSendTime.strftime('%m-%d %H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                                        #print(f'Lora Control Cmd send :{LoraControlCmd}(time:{formatted_time})')
                                        print(f'Lora Control Cmd send(time:{formatted_time})')
                                        self.recv_control_ack.clear()
                                        self.send_event.clear()
                                        self.recv_event.set()
                                        continue 
                                    except Exception as e:
                                            print(f'Serial wirte error:{e}')
                                else:
                                    #print('LoraControlCmd is dropped')
                                    continue
                    else:
                        current_time = datetime.now()
                        timediff = (current_time - self.ControlCmdSendTime).total_seconds()
                        if timediff > 1 and self.ControlCmdResend_cnt < 2:
                            handle.write(LoraControlCmd)
                            self.ControlCmdSendTime = datetime.now()
                            self.ControlCmdResend_cnt += 1
                            formatted_time = self.ControlCmdSendTime.strftime('%m-%d %H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                            #print(f'ReSend Control Cmd:{LoraControlCmd}(time:{formatted_time})')
                            print(f'ReSend Control Cmd(time:{formatted_time})')
                        elif self.ControlCmdResend_cnt == 2:
                            self.ControlCmdResend_cnt = 0
                            self.recv_control_ack.set()
                            LoraControlCmd_Q.deltime()                  
                            
                            
                        self.send_event.clear()
                        self.recv_event.set()
                        continue 

                
                if self.packets_unacked < self.TCPVegas.cwnd:
                    if self.recv_dataupload_ack.is_set():
                        if not DataUploadCmd_Q.empty() :    
                            DataUploadCmd = DataUploadCmd_Q.dequeue()
                            if DataUploadCmd != None:
                                try:
                                    handle.write(DataUploadCmd)
                                    SendPackCnt += 1
                                    self.packets_unacked += 1    
                                    self.DataUploadCmdSendTime = datetime.now()
                                    formatted_time = self.DataUploadCmdSendTime.strftime('%m-%d %H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                                    #print(f'Data Upload Cmd send :{DataUploadCmd}(time:{formatted_time})')
                                    print(f'Data Upload Cmd send(time:{formatted_time})')
                                    self.recv_dataupload_ack.clear()
                                except Exception as e:
                                        print(f'Serial wirte error:{e}')
                            else:
                                print('DataUploadCmd is dropped')
                                
                    else:
                        current_time = datetime.now()
                        timediff = (current_time - self.DataUploadCmdSendTime).total_seconds()
                        if timediff > 1 and self.DataUploadCmdResend_cnt < 2:
                            handle.write(DataUploadCmd)
                            self.DataUploadCmdSendTime = datetime.now()
                            self.DataUploadCmdResend_cnt += 1
                            formatted_time = self.DataUploadCmdSendTime.strftime('%m-%d %H:%M:%S.%f')[:-3]  # 截取到倒数第3位，得到毫秒
                            #print(f'ReSend Data Upload Cmd:{DataUploadCmd}(time:{formatted_time})')
                            print(f'ReSend Data Upload Cmd(time:{formatted_time})')
                        elif self.DataUploadCmdResend_cnt == 2:
                            self.DataUploadCmdResend_cnt = 0
                            self.recv_dataupload_ack.set()
                            DataUploadCmd_Q.deltime()
                        
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
        
TCPVegas = TCPVegas()
MQTT_Service = MQTT(None,TCPVegas) 
LORA_Service = LORA(MQTT_Service,TCPVegas)
MQTT_Service.lora_service = LORA_Service


Lora_Process = threading.Thread(target=LORA_Service.Start_Lora_Service,args=())
MQTT_Process = threading.Thread(target=MQTT_Service.Start_MQTT_Service,args=())

Lora_Process.start()
MQTT_Process.start()