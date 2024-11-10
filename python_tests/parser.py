from enum import Enum
from ctypes import Structure, c_ubyte, c_uint, c_short, c_ushort, memmove, pointer, sizeof

from config import Config

class Sectors(Enum):
    ObcSensors = 0

class Mpu(Structure):
    _pack_ = 1
    _fields_ = [
        ("B_X", c_short),
        ("B_Y", c_short),
        ("B_Z", c_short),
        ("G_X", c_short),
        ("G_Y", c_short),
        ("G_Z", c_short),
        ("A_X", c_short),
        ("A_Y", c_short),
        ("A_Z", c_short),
        ("T", c_ushort)
    ]

class ObcSensorsTme(Structure):
    _pack_ = 1
    _fields_ = [
        ("TIME", c_uint),
        ("MPU", Mpu * 2)
    ]

class Parser():
    def __init__(self) -> None:
        self.need_parse = {
            Config.Commands.READ_TME : self._resolve_read_tme_parser,
            Config.Commands.READ_TME_BUNCH : self._resolve_read_tme_bunch_parser,
        }

    Sectors = Sectors
    ObcSensorsTme = ObcSensorsTme

    def _resolve_read_tme_parser(self, request_body):
        out_type_name = Parser.Sectors(request_body.sector_num).name + 'Tme'
        return getattr(Parser, out_type_name)
    
    def _resolve_read_tme_bunch_parser(self, request_body):
        out_type_name = Parser.Sectors(request_body.sector_num).name + 'Tme'
        out_type = getattr(Parser, out_type_name)
        return out_type * request_body.qty

def parse(data, out_type):
    parsed_data = out_type()
    memmove(pointer(parsed_data), pointer(data), sizeof(parsed_data))
    return parsed_data