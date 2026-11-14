
# import matplotlib.pyplot as plt
import random
import socket
import struct
import time

port = 5000
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
serverSocket.bind(('0.0.0.0', port))

print(f'Listening on port: {port}')
print('--------------------------------------------')


log_file = './data/PID_log_{}.csv'
exp_id_log = './data/expid.log'

def fwd_exp_id():
    exp = []
    with open(exp_id_log,'r') as fp:
        try:
            exp = fp.readline();
            print("content:",exp)
        except:
            print('error')

    with open(exp_id_log,'w') as fp:
        fp.write(f'{int(exp)+1}')
        
    print('exp_id:',int(exp)+1)
    return int(exp)+1


exp_id = fwd_exp_id()

with open(log_file.format(exp_id),'w+') as fp:
    fp.write('#time,enc,set,speed,opt\n')
    while(1):
        message, address = serverSocket.recvfrom(24)
        #contents = struct.unpack('1f', message)[0]
        print(f'{time.time()},{message.decode()}')

        fp.write(f'{time.time()},{message.decode()}\n')

