#!/usr/bin/python3 -u

import socket
import threading
import time
import datetime
import sys
import os
import getopt

opts, args = getopt.getopt(sys.argv[1:], 'i:p:')

my_port = 5000
my_ip = ''

for o, a in opts:
	if o == '-p':
		my_port = int( a )
	elif o == '-i':
		my_ip = a

u = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
u.bind( (my_ip, my_port) )

while True:
	try:
		data, address = u.recvfrom(8192)
	except:
		print('[receive error]')
		continue

	S = data.decode()
	print( datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'), address[0] + ':' + str(address[1]), '<' + S + '>' )

	try:
		u.sendto( data, address )
	except:
		print('[send error]')
		continue
