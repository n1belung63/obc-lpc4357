import serial
from serial.tools import list_ports
from ctypes import memmove, pointer, sizeof, Array, Structure, Union, _Pointer, _SimpleCData, c_int
from json import JSONEncoder

from config import Config 

from typing import Any, Union


def get_ports(VID : int, PID : int):
    list_str = []
    for port in list_ports.comports():
        if (port.vid == VID and port.pid == PID):
            list_str.append(port.device)

    list_int = [int(x[3:]) for x in list_str]
    list_int.sort()

    list_str = []
    for com_num in list_int:
        list_str.append('COM' + str(com_num))
    return list_str


class CDataJSONEncoder(JSONEncoder):
    def default(self, obj):
        if isinstance(obj, (Array, list)):
            return [self.default(e) for e in obj]

        if isinstance(obj, _Pointer):
            return self.default(obj.contents) if obj else None

        if isinstance(obj, _SimpleCData):
            return self.default(obj.value)

        if isinstance(obj, (bool, int, float, str)):
            return obj

        if obj is None:
            return obj

        if isinstance(obj, (Structure, Union)):
            result : dict = {}
            anonymous = getattr(obj, '_anonymous_', [])

            for key, *_ in getattr(obj, '_fields_', []):
                value = getattr(obj, key)

                # private fields don't encode
                if key.startswith('_'):
                    continue

                if key in anonymous:
                    result.update(self.default(value))
                else:
                    result[key] = self.default(value)

            return result

        return JSONEncoder.default(self, obj)


class SerialPort(serial.Serial):    
    def __init__(self, ser_name : str, timeout : int = 10):
        super(serial.Serial, self).__init__(ser_name, baudrate = 115200, timeout = timeout, inter_byte_timeout=0.001)

    def write(self, data):
        for byte in data:
            super(self.__class__, self).write(bytes([byte]))

    def read(self, read_length):
        data = super(self.__class__, self).read(read_length)
        if len(data) != read_length: 
            raise Exception("Can't read enough data from com port")
        return data

    
    def write_read(self, data : bytes, read_length : int) -> bytes:
        self.write(data)
        return self.read(read_length)

    
class Request():
    ACK = 0x06
    NACK = 0x15
    ERR = 0x45
    WRONG = 0x57

    ERR_SIZE = 4

    def __init__(self, serial : SerialPort, config : Config):
        self.serial = serial
        self.config = config


    def post(self, command : Config.Commands, request_data = None, req_size : Union[int, None] = None):
        data = bytes([command.value])
        if request_data != None:
            data += self.__request_data_to_bytes__(request_data, req_size)

        res = self.serial.write_read(data, 1)

        if res:
            if res[0] == self.WRONG:
                raise Exception('Wrong request')
            elif res[0] == self.NACK:
                data = self.serial.read(self.ERR_SIZE)
                err_inst = c_int(0)
                memmove(pointer(err_inst), data, self.ERR_SIZE)
                raise Exception(f'No response, error is {hex(err_inst.value)}')
            elif res[0] == self.ERR:
                raise Exception('Wrong parameters')

    
    def get(self, command : Config.Commands, request_data = None, req_size : Union[int, None] = None, resp_size : Union[int, None] = None) -> Any:      
        data = bytes([command.value])
        if request_data != None:
            data += self.__request_data_to_bytes__(request_data, req_size)

        resp_type = self.__get_response_type__(command)
        resp_inst = resp_type()

        res = self.serial.write_read(data, 1)

        if res:
            if res[0] == self.WRONG:
                raise Exception('Wrong request')
            elif res[0] == self.NACK:
                data = self.serial.read(self.ERR_SIZE)
                err_inst = c_int(0)
                memmove(pointer(err_inst), data, self.ERR_SIZE)
                raise Exception(f'No response, error is {hex(err_inst.value)}')
            elif res[0] == self.ERR:
                raise Exception('Wrong parameters')
            elif res[0] == self.ACK:
                if resp_size is None:
                    resp_size : int = self.__get_body_size__(resp_type)
                res = self.serial.read(resp_size)
                memmove(pointer(resp_inst), res, resp_size)
                return resp_inst

        return None

    
    def __request_data_to_bytes__(self, tme_body, req_size : Union[int, None] = None):
        if isinstance(tme_body, bytes):
            data_bytes : bytes = tme_body
        else:
            if req_size is not None:
                data_bytes : bytes = bytes(req_size)
            else:
                data_bytes : bytes = bytes(sizeof(tme_body))   

            memmove(data_bytes, pointer(tme_body), len(data_bytes))
        return data_bytes


    def __get_body_size__(self, item):
        size = 0
        if item:         
            for field in item._fields_:
                size += getattr(item, field[0]).size
            return size
        else:
            return size


    def __get_response_type__(self, command : Config.Commands):
        words = command.name.split("_")
        capitalized_words = [word.capitalize() for word in words]
        sentense = ''.join(capitalized_words)
        return getattr(Config, sentense + 'Response')