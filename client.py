import socket         					#导入socket模块
s = socket.socket()						#创建套接字
host = '127.0.0.1'						#IP
port = 8888								#端口
s.connect((host,port))					#主动初始化TCP服务器连接

while True:
  send_data = input('请输入要发送的数据')		#提示用户输入数据
  if send_data == 'quit':
    break
  s.send(send_data.encode())					#发送TCP数据

  #接受对方发送过来的数据，最大接受1024字节
  recvData = s.recv(1024).decode()
  print('接收到的数据为：', recvData)
#关闭套接字
s.close()
