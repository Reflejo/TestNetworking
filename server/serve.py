#!/usr/bin/env python3

import socket
import sys
import time

HOST = "0.0.0.0"
PORT = 1337


def send_forever(conn):
    while True:
        data = b"Baby shark do do do do do do\n"
        print("Sending %s" % data)
        try:
            conn.sendall(data)
            time.sleep(1)
        except KeyboardInterrupt:
            sys.exit(0)

        except (BrokenPipeError, ConnectionResetError):
            return


def bind():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((HOST, PORT))
        sock.listen()

        while True:
            print("Accepting new connections")
            connection, address = sock.accept()
            with connection:
                print("Connected by", address)
                data = connection.recv(1024)
                if not data:
                    continue

                print("Received %s" % data)
                send_forever(connection)


if __name__ == "__main__":
    bind()
