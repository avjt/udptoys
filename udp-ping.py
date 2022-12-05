#!/usr/bin/python3 -u

import socket
import threading
import time
import datetime
import sys
import os
import getopt

# -- Receive Loop ------------------------------------------------------------
def do_receive(u):
	while True:
		try:
			data, address = u.recvfrom(8192)
		except KeyboardInterrupt:
			print('[interrupted]')
			sys.exit(1)
		except:
			print('[receive error]')
			continue
		S = data.decode()
		print( datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'), address[0] + ':' + str(address[1]), '<' + S + '>' )
	

# -- Sending (Main) Thread ---------------------------------------------------
opts, args = getopt.getopt(sys.argv[1:], 'i:p:t:')

timeout = 1
my_port = 4999
my_ip = ''

for o, a in opts:
	if o == '-t':
		timeout = int( a )
	elif o == '-p':
		my_port = int( a )
	elif o == '-i':
		my_ip = a

if len(args) != 2:
	print("ip & port?")
	sys.exit(1)

target_ip = args[0]
target_port = int(args[1])

u = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
u.bind( (my_ip, my_port) )

threading.Thread(target=do_receive, args=(u,), daemon=True).start()

while True:
	try:
		u.sendto( bytes('ping', 'ascii'), (target_ip, target_port) )
	except KeyboardInterrupt:
		print('[interrupted]')
		sys.exit(1)
	except:
		print('[send error]')
		continue

	time.sleep(timeout)
