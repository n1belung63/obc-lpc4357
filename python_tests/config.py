from enum import Enum
from ctypes import Structure, c_ubyte, c_uint, c_short, c_ushort

class Config():
    class Commands(Enum):
        GET_SD_STATUS = 1
        READ_SD = 2
        WRITE_SD = 3
        ERASE_SD = 4

        READ_MPU = 5

    class GetSdStatusRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sd_num", c_ubyte)
        ]

    class GetSdStatusResponse(Structure):
        pass

    class ReadSdRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sd_num", c_ubyte),
            ("addr", c_uint),
            ("quarter", c_ubyte)
        ]

    class ReadSdResponse(Structure):
        _pack_ = 1
        _fields_ = [
            ("data", c_ubyte * 128)
        ]

    class WriteSdRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sd_num", c_ubyte),
            ("addr", c_uint),
            ("quarter", c_ubyte),
            ("data", c_ubyte * 128)
        ]

    class EraseSdRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sd_num", c_ubyte),
            ("addr_start", c_uint),
            ("addr_end", c_uint),
        ]

    class ReadMpuRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("mpu_num", c_ubyte)
        ]

    class ReadMpuResponse(Structure):
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


if __name__ == '__main__':
    tst : Config.ReadSdRequest = Config.ReadSdRequest()
    tst.addr = 2
    tst.sd_num = 1

    pass