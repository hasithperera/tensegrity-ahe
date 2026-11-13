import socket

ARDUINO_IP = "192.168.0.255"  # broad cast id
ARDUINO_PORT = 52311

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET,socket.SO_BROADCAST,1)


while True:
    data = [65,65,78,78,78,78,78,78,78,78] #default cmd
    msg=bytes(data)
    message = input("Enter message (q to quit): ")

   
    if message.lower() == "q":
        break

    if message.lower() == "s":
        data = [65, 30, 67, 67, 68,68,78,78,78,78]
        msg=bytes(data)

    if message.lower() == "f":
        data = [180, 30, 67, 67, 68,68,78,78,78,78]
        msg=bytes(data)

    if message.lower() == "r":
        data = [40, 30, 67, 67, 68,68,78,78,78,78]
        msg=bytes(data)

    # Send UDP packet
    sock.sendto(
        msg,
        (ARDUINO_IP, ARDUINO_PORT)
    )


sock.close()

