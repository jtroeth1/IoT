#!/usr/bin/env python3

'''Jora server opens a tcp socket on port 7878 and listens for traffic. The Jora client will connect to this. Any messages send are stored in a local file, that will be picked up by the jora gateway code and used to send messages to nodes.
'''
import socket
import time
import os

fileName = "/home/master/IoT/Jora/Jora_server/Downlink.txt"
ledOn = 'L1'
ledOff = 'L0'

CONN_CLOSE = b'c'

s = socket.socket()
port = 7878
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

s.bind(('', port))	#any ip
print('Bound to', port)

s.listen(5)
print('Listening...')

c, addr = s.accept()
print('Connected to', addr[0])

def writeMessage(data):
	f = open(fileName, 'a')
	print('Writing', data )
	f.write(data + '#' + '\n')
	f.close()
try:
	os.remove(fileName)
except:
	#print('No such file')
	pass

def receiveMessage():
	try:
		data = str(c.recv(1024))
		print(data)
		d = data.split('\'', 2)
		print(d)

	except KeyboardInterrupt:
		print('Caught Ctrl-C!')		
		if c:
			print('Closing Connection...')
			c.send(CONN_CLOSE)
			c.close()
	except:
		print('Caught Exception!')
		if c:
			print('Closing Connection...')
			c.send(CONN_CLOSE)
			c.close()

	return d[1]

while True:

	data = receiveMessage()
		
	if '$' in data:
		print(data)
		cmd = data.split('@', 1)
		print(cmd)

	if len(cmd) > 0:
		if cmd[1] == ledOn:
			print('Setting LED On...')
			writeMessage(data)
			c.send(ledOn.encode('utf-8'))	#ACK
		elif cmd[1] == ledOff:
			print('Setting LED Off...')
			writeMessage(data)
			c.send(ledOff.encode('utf-8'))
		else:
			print('Invalid data received...')
			
#c.close()
