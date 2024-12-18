from enum import Enum
from ctypes import Structure, c_ubyte, c_uint, c_short, c_ushort

class Config():
    class Commands(Enum):
        GET_SD_STATUS = 1
        READ_SD = 2
        WRITE_SD = 3
        ERASE_SD = 4
        BLOCK_SD = 5
        UNBLOCK_SD = 6
        
        READ_MPU = 7

        READ_TME = 8
        READ_TME_BUNCH = 9

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
        _SD_PAGE_QUARTER_LENGTH = 128
        _pack_ = 1
        _fields_ = [
            ("data", c_ubyte * _SD_PAGE_QUARTER_LENGTH)
        ]

    class WriteSdRequest(Structure):
        _SD_PAGE_QUARTER_LENGTH = 128
        _pack_ = 1
        _fields_ = [
            ("sd_num", c_ubyte),
            ("addr", c_uint),
            ("quarter", c_ubyte),
            ("data", c_ubyte * _SD_PAGE_QUARTER_LENGTH)
        ]

    class EraseSdRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sd_num", c_ubyte),
            ("addr_start", c_uint),
            ("addr_end", c_uint),
        ]

    class BlockSdRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sd_num", c_ubyte)
        ]

    class UnblockSdRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sd_num", c_ubyte)
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

    class ReadTmeRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sector_num", c_ubyte),
            ("time", c_uint)
        ]

    class ReadTmeResponse(Structure):
        _SD_PAGE_QUARTER_LENGTH = 128
        _pack_ = 1
        _fields_ = [
            ("data", c_ubyte * _SD_PAGE_QUARTER_LENGTH)
        ]

    class ReadTmeBunchRequest(Structure):
        _pack_ = 1
        _fields_ = [
            ("sector_num", c_ubyte),
            ("time", c_uint),
            ("step", c_uint),
            ("qty", c_uint)
        ]

    class ReadTmeBunchResponse(Structure):
        _SD_PAGE_LENGTH = 512
        _pack_ = 1
        _fields_ = [
            ("data", c_ubyte * _SD_PAGE_LENGTH)
        ]

if __name__ == '__main__':
    tst : Config.ReadSdRequest = Config.ReadSdRequest()
    tst.addr = 2
    tst.sd_num = 1

    pass