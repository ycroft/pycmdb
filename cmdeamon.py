# coding: utf-8

import argparse
import os
import pickle
import socket
import sys

class BadWorkspace(Exception): pass
class BadArgument(Exception): pass
class BadPacket(Exception): pass

class CONFIG:
    DB_FILE = '/deamon.db'
    DEFAULT_IP = '127.0.0.1'
    DEFAULT_PORT = 8808
    SOCKET_BUF = 4096

class Console:
    @classmethod
    def log(cls, *x):
        t = [str(i) for i in x]
        print(' '.join(t))

class Deamon(object):

    def __init__(self):

        self.init_from_args()

        if not os.path.exists(self.ws):
            Console.log('expect path:', self.ws)
            raise BadWorkspace()

        self.kv = {}

        self.init_db()
        self.serve()

    def init_from_args(self):
        parser = argparse.ArgumentParser(description='cmdb deamon')
        parser.add_argument('--workspace', '-w', required=True,
            help='database file directory')
        parser.add_argument('--addr', '-a', required=False,
            help='set ip:port such as "127.0.0.1:8800"')

        args = parser.parse_args()

        self.ws = args.workspace
        self.addr = args.addr

    def init_db(self):
        
        db_file = self.ws + CONFIG.DB_FILE

        if os.path.exists(db_file):
            self.load_db(db_file)
        else:
            self.create_db(db_file)

    def load_db(self, path):
        f = open(path, 'rb')
        serial = f.read()

        self.kv = pickle.loads(serial)

        Console.log('db load:\n', self.kv)

    def create_db(self, path):
        self.kv['local'] = [self.ws, ]
        serial = pickle.dumps(self.kv)

        f = open(path, 'wb')
        f.write(serial)
        f.close()

        Console.log('empty db created')

    def serve(self):

        if self.addr:
            vt = self.addr.split(':')
            ip = vt[0]
            port = int(vt[1])
        else:
            ip = CONFIG.DEFAULT_IP
            port = CONFIG.DEFAULT_PORT

        Console.log('bind with', ip, port)
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind((ip, port))

        self.deamon_run = True

        while self.deamon_run:
            data, client = s.recvfrom(CONFIG.SOCKET_BUF)

            if not data:
                raise BadPacket()

            self.deal_pkt(data, client)

    def deal_pkt(data, client):
        pass

if __name__ == '__main__':
    Deamon()

