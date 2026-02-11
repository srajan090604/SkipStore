import socket

# Connect to our C++ Server
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('localhost', 8080))
print("Connected to Server!")

while True:
    # Get user input
    msg = input("Enter command (e.g., SET user 1): ")
    
    # Send to C++
    client.send(msg.encode())
    
    # Get response
    response = client.recv(1024).decode()
    print("Server said:", response)