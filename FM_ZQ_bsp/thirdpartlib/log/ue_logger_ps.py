#!/usr/bin/env python3
"""
基础版日志解析脚本 - 用于解析嵌入式设备传输的二进制日志数据
需要对应的日志定义文件来重建原始消息
"""
import socket
import struct
import sys
import json
import re
import os
from datetime import datetime
from typing import Dict, List, Any, Optional, Tuple
import threading

LOG_SIZE = 1024 * 1024 * 2         # 日志文件大小
UE_LOG_FILE_L1C = "l1c"         # L1C日志文件
UE_LOG_FILE_PS  = "ue_stack"    # 协议栈日志文件

class UDPServer:
    def __init__(self, host='localhost', port=9999):
        self.host = host
        self.port = port
        self.socket = None
        self.running = False

    def start(self):
        """启动 UDP 服务器"""
        try:
            # 创建 UDP socket
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            # 绑定地址和端口
            self.socket.bind((self.host, self.port))
            self.running = True
            
            print(f"UDP 日志服务器启动在 {self.host}:{self.port}")
            print("等待接收数据... (输入 'quit' 退出)")
            
            # 启动接收线程
            receive_thread = threading.Thread(target=self._receive_loop)
            receive_thread.daemon = True
            receive_thread.start()
            
            # 主线程处理用户输入
            while self.running:
                cmd = input().strip().lower()
                if cmd == 'quit':
                    break
                    
        except Exception as e:
            print(f"服务器错误: {e}")
        finally:
            self.stop()
    
    def _receive_loop(self):
        """接收数据循环"""
        now = datetime.now()
        # 格式化的输出
        formatted_date = now.strftime("%Y%m%d")
        formatted_time = now.strftime("%H%M%S")

        current_directory = os.getcwd()

        log_file_name = UE_LOG_FILE_PS
        log_file_name += "_"
        log_file_name += formatted_time
        log_file_name += ".log"
        print(f"日志文件: {log_file_name}")

        if not os.path.exists(formatted_date):
            os.mkdir(formatted_date)
        total_size = 0
        log_file = "./" + formatted_date + "/" + log_file_name
        log_file_p = open(log_file, 'w')
        print(f"日志文件: {log_file}")
        #LOG_SIZE = 1024 
        ram_size = 0
        while self.running:
            try:
                now = datetime.now()

                # 接收数据，最大缓冲区 1024 字节
                data, addr = self.socket.recvfrom(1024)
                
                # 解码数据
                total_size += len(data)
                ram_size += len(data)
                message = data.decode('utf-8')
                timestamp = now.strftime("%Y-%m-%d %H:%M:%S")
                line_message ="[" + timestamp + "] " + message
                log_file_p.write(line_message)

                if ram_size > 256:
                    ram_size = 0
                    log_file_p.flush()

                #print(f"[{timestamp}] 来自 {addr[0]}:{addr[1]} 的消息: {message}")
                #print(f"total size: {total_size}")
                print(f"[{timestamp}]: {message}", end='')

                if total_size > LOG_SIZE:
                    log_file_p.close()

                    total_size = 0
                    formatted_date = now.strftime("%Y%m%d")
                    formatted_time = now.strftime("%H%M%S")
                    log_file_name = UE_LOG_FILE_PS
                    log_file_name += "_"
                    log_file_name += formatted_time
                    log_file_name += ".log"
                    if not os.path.exists(formatted_date):
                        os.mkdir(formatted_date)
                    log_file = "./" + formatted_date + "/" + log_file_name
                    log_file_p = open(log_file, 'w')
                    print(f"日志文件: {log_file}")
                
            except socket.error as e:
                if self.running:
                    print(f"接收错误: {e}")
            except UnicodeDecodeError:
                print(f"收到来自 {addr} 的二进制数据: {data}")
                # 回应二进制数据
                #self.socket.sendto(b"Received binary data", addr)
    
    def stop(self):
        """停止服务器"""
        self.running = False
        if self.socket:
            self.socket.close()
        print("UDP 服务器已停止")


def main():
    server = UDPServer(host='192.168.255.100', port=9999)
    server.start()


if __name__ == "__main__":
    main()


