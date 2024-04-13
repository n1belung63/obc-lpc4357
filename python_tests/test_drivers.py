import pytest, time, logging, logging.config, os, json
import yaml
import inspect


from config import Config 
from request import *

from typing import List

if not os.path.exists("logs"):
    os.makedirs("logs")

with open(
    os.path.join(os.path.dirname(__file__), "logging_config.yaml"), "rt"
) as f:
    config = yaml.safe_load(f.read())

logging.config.dictConfig(config)
logger = logging.getLogger(__name__)

def log(cmd_name : str):
        def inner(func):
            # logger.info(f"[TST] - {test_name}")
            logger.info(f"[CMD] - {cmd_name}")

            def try_func(*args, **kwargs):
                try:
                    logger.info(f"[INP] - {json.dumps(args[1], cls=CDataJSONEncoder)}")
                    data = func(*args, **kwargs) 
                    if data is not None:
                        logger.info(f"[OUT] - {json.dumps(data, cls=CDataJSONEncoder)}")
                        return (True, data)
                    else:
                        logger.info(f"[OUT] - Ok")
                        return (True, None)

                except Exception as err:
                    # logger.warning(f"[TST] - {test_name}")
                    logger.warning(f"[ERR] - {err}")
                    return (False, None)        

            return try_func
        return inner

def get_test_name(cls):
    func_name = inspect.getouterframes( inspect.currentframe() )[1][3]
    return f"{cls.__class__.__name__}.{func_name}"

@pytest.fixture(scope = "module")
def req_inst():
    COM_VID = 0x0403
    COM_PID = 0x6011
    ports = get_ports(COM_VID, COM_PID)
    req : Request = Request(SerialPort(ports[0]), Config())
    yield req 
    # close connection


class Test_SdCard():
    class Support_SdCard:
        QUATERS_COUNT = 4
        PAGE_SIZE = 512
        PAGE_QUATER_SIZE = 128

        def write_page(self, req : Request, sd_num : int, addr : int, inp_buf : List[int]):
            request_body_write = Config.WriteSdRequest()
            request_body_write.sd_num = sd_num
            request_body_write.addr = addr
            for q in range(self.QUATERS_COUNT):
                request_body_write.quarter = q
                for i in range(len(request_body_write.data)):
                    request_body_write.data[i] = inp_buf[self.PAGE_QUATER_SIZE * q + i]
                cmd = Config.Commands.WRITE_SD
                (flag, res) = log(cmd.name)(req.post)(cmd, request_body_write)
                assert(flag)
                time.sleep(0.5)
            return flag

        def read_page(self, req : Request, sd_num : int, addr : int) -> List[int]:
            request_body_read = Config.ReadSdRequest()
            request_body_read.sd_num = sd_num
            request_body_read.addr = addr
            out_buf = [0]*self.PAGE_SIZE
            for q in range(self.QUATERS_COUNT):
                request_body_read.quarter = q
                cmd = Config.Commands.READ_SD
                (flag, data) = log(cmd.name)(req.get)(cmd, request_body_read)
                assert(flag)
                for i in range(sizeof(data)):
                    out_buf[self.PAGE_QUATER_SIZE * q + i] = data.data[i]
                time.sleep(0.5)
            return out_buf

    def setup_class(self):
        self.sup : Test_SdCard.Support_SdCard = self.Support_SdCard()

    def test_get_status(self, req_inst : Request):
        pass

    def test_read_sd(self, req_inst : Request): 
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
    
        SD_NUM = 1
        ADDR = 0
        out_buf = self.sup.read_page(req_inst, SD_NUM, ADDR)

        logger.info(f"[RES] - PASSED\n")

        pass

    def test_write_sd(self, req_inst : Request):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        SD_NUM = 1
        ADDR = 0

        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))
        self.sup.write_page(req_inst, SD_NUM, ADDR, inp_buf)

        logger.info(f"[RES] - PASSED\n")

        pass

    def test_write_and_read_sd(self, req_inst : Request):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
          
        SD_NUM = 1
        ADDR = 0

        inp_buf = [0]*self.sup.PAGE_SIZE
        self.sup.write_page(req_inst, SD_NUM, ADDR, inp_buf) 
        out_buf = self.sup.read_page(req_inst, SD_NUM, ADDR)    

        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))
        self.sup.write_page(req_inst, SD_NUM, ADDR, inp_buf) 
        out_buf = self.sup.read_page(req_inst, SD_NUM, ADDR)

        assert(inp_buf == out_buf)

        logger.info(f"[RES] - PASSED\n")

        pass

    def test_erase_sd(self, req_inst : Request):    
        SD_NUM = 1
        PAGE_COUNT = 3
    
        request_body_erase = Config.EraseSdRequest()
        request_body_erase.sd_num = SD_NUM

        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))
        print(inp_buf)
        

        for addr in range(PAGE_COUNT):
            self.sup.write_page(req_inst, SD_NUM, addr, inp_buf)   
            print("smth")
            out_buf = self.sup.read_page(req_inst, SD_NUM, addr)    
            print(out_buf)

        request_body_erase.addr_start = 0
        request_body_erase.addr_end = PAGE_COUNT - 1
        req_inst.post(Config.Commands.ERASE_SD, request_body_erase)
        
        for addr in range(PAGE_COUNT):
            out_buf = self.sup.read_page(req_inst, SD_NUM, addr)    
            print(out_buf)

        pass


class Test_Sensor():
    def test_read_mpu(self, req_inst : Request):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
        cmd = Config.Commands.READ_MPU

        request_body = Config.ReadMpuRequest()
        request_body.mpu_num = 0
        (flag, res) = log(cmd.name)(req_inst.get)(cmd, request_body)
        if not flag: 
            logger.info(f"[RES] - FAILED\n")
        assert(flag and res is not None)

        request_body = Config.ReadMpuRequest()
        request_body.mpu_num = 1
        (flag, res) = log(cmd.name)(req_inst.get)(cmd, request_body)
        assert(not flag and res is None)
        if flag: 
            logger.info(f"[RES] - FAILED\n")

        logger.info(f"[RES] - PASSED\n")

        pass