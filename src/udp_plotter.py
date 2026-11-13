
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




while(1):
    message, address = serverSocket.recvfrom(10)
    #contents = struct.unpack('1f', message)[0]
    print(f'{time.time()},{message.decode()}')
    # plt.scatter(i, contents)

#     plt.stem([i], [contents])
#     plt.pause(0.2)

# plt.show()