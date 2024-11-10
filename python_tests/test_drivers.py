import pytest, time, logging, logging.config, os, json
import yaml
import inspect


from config import Config 
from request import *
from parser import Parser, parse

from typing import List

if not os.path.exists("logs"):
    os.makedirs("logs")

with open(os.path.join(os.path.dirname(__file__), "logging_config.yaml"), "rt") as f:
    config = yaml.safe_load(f.read())

logging.config.dictConfig(config)
logger = logging.getLogger(__name__)
parser = Parser()

def log(cmd_name : str):
    def inner(func):
        logger.info(f"[CMD] - {cmd_name}")

        def try_func(*args, **kwargs):
            try:
                logger.info(f"[INP] - {json.dumps(args[1], cls=CDataJSONEncoder)}")

                data = func(*args, **kwargs) 
                if data is not None:
                    if args[0] in parser.need_parse.keys():
                        data = parse( data, parser.need_parse[ args[0] ](args[1]) )
                    logger.info(f"[OUT] - {json.dumps(data, cls=CDataJSONEncoder)}")
                    return (True, data)
                else:
                    logger.info(f"[OUT] - Ok")
                    return (True, None)

            except Exception as err:
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
        QUARTERS_COUNT = 4
        PAGE_SIZE = 512
        PAGE_QUARTER_SIZE = 128

        def write_page(self, req : Request, sd_num : int, addr : int, inp_buf : List[int], resp_flag : bool) -> bool:
            request_body_write = Config.WriteSdRequest()
            request_body_write.sd_num = sd_num
            request_body_write.addr = addr
            for q in range(self.QUARTERS_COUNT):
                request_body_write.quarter = q
                for i in range(len(request_body_write.data)):
                    request_body_write.data[i] = inp_buf[self.PAGE_QUARTER_SIZE * q + i]
                cmd = Config.Commands.WRITE_SD
                (flag, res) = log(cmd.name)(req.post)(cmd, request_body_write)
                assert(flag == resp_flag)
                if not flag:
                    assert(res == None)
                    return flag
                time.sleep(2)
            return flag

        def read_page(self, req : Request, sd_num : int, addr : int, resp_flag : bool) -> tuple[bool, List[int]]:
            request_body_read = Config.ReadSdRequest()
            request_body_read.sd_num = sd_num
            request_body_read.addr = addr
            out_buf = [0]*self.PAGE_SIZE
            for q in range(self.QUARTERS_COUNT):
                request_body_read.quarter = q
                cmd = Config.Commands.READ_SD
                (flag, data) = log(cmd.name)(req.get)(cmd, request_body_read)
                assert(flag == resp_flag)
                if not flag:
                    assert(data == None)
                    return (flag, None)
                else:
                    for i in range(sizeof(data)):
                        out_buf[self.PAGE_QUARTER_SIZE * q + i] = data.data[i]
                    time.sleep(2)
            return (flag, out_buf)

    def setup_class(self):
        self.sup : Test_SdCard.Support_SdCard = self.Support_SdCard()

    SD0, SD1 = 0, 1
    test_data = [(SD0, False),(SD1, True)]

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_get_status(self, req_inst : Request, sd_num : int, resp_flag : bool):
        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_read_sd(self, req_inst : Request, sd_num : int, resp_flag : bool): 
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
    
        ADDR = 0
        (flag, out_buf) = self.sup.read_page(req_inst, sd_num, ADDR, resp_flag)

        assert(flag == resp_flag)
        if not flag:
            assert(out_buf == None)

        logger.info(f"[RES] - PASSED\n")

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_write_sd(self, req_inst : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        ADDR = 0

        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))
        flag = self.sup.write_page(req_inst, sd_num, ADDR, inp_buf, resp_flag)
        
        assert(flag == resp_flag)

        logger.info(f"[RES] - PASSED\n")

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_write_and_read_sd(self, req_inst : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
          
        ADDR = 0

        inp_buf = [0]*self.sup.PAGE_SIZE
        flag = self.sup.write_page(req_inst, sd_num, ADDR, inp_buf, resp_flag)
        assert(flag == resp_flag)
        (flag, out_buf) = self.sup.read_page(req_inst, sd_num, ADDR, resp_flag)    
        assert(flag == resp_flag)
        if not flag:
            assert(out_buf == None)

        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))

        flag = self.sup.write_page(req_inst, sd_num, ADDR, inp_buf, resp_flag) 
        assert(flag == resp_flag)
        (flag, out_buf) = self.sup.read_page(req_inst, sd_num, ADDR, resp_flag)
        assert(flag == resp_flag)
        if not flag:
            assert(out_buf == None)
        else:
            assert(inp_buf == out_buf)

        logger.info(f"[RES] - PASSED\n")

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_stop_writing_sd(self, req_inst : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        cmd = Config.Commands.BLOCK_SD

        request_body = Config.BlockSdRequest()
        request_body.sd_num = sd_num
        (flag, res) = log(cmd.name)(req_inst.post)(cmd, request_body)

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)

        logger.info(f"[RES] - PASSED\n")

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_start_writing_sd(self, req_inst : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        cmd = Config.Commands.UNBLOCK_SD

        request_body = Config.UnblockSdRequest()
        request_body.sd_num = sd_num
        (flag, res) = log(cmd.name)(req_inst.post)(cmd, request_body)

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)

        logger.info(f"[RES] - PASSED\n")
        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_erase_sd(self, req_inst : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
        cmd = Config.Commands.ERASE_SD

        PAGE_COUNT = 1
    
        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))        

        for addr in range(PAGE_COUNT):
            flag = self.sup.write_page(req_inst, sd_num, addr, inp_buf, resp_flag)  
            assert(flag == resp_flag) 
            (flag, out_buf) = self.sup.read_page(req_inst, sd_num, addr, resp_flag)
            assert(flag == resp_flag)
            if not flag:
                assert(out_buf == None)
            else:
                assert(out_buf == inp_buf)


        request_body = Config.EraseSdRequest()
        request_body.sd_num = sd_num
        request_body.addr_start = 0
        request_body.addr_end = PAGE_COUNT - 1
        (flag, res) = log(cmd.name)(req_inst.post)(cmd, request_body)

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)
        
        for addr in range(PAGE_COUNT):
            (flag, out_buf) = self.sup.read_page(req_inst, sd_num, addr, resp_flag)
            assert(flag == resp_flag)
            if not flag:
                assert(out_buf == None)
            else:
                assert(out_buf == [0]*self.sup.PAGE_SIZE)
            print(out_buf)

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_erase_sd_sector(self, req_inst : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
        cmd = Config.Commands.ERASE_SD

        PAGE_COUNT = 0x00200000

        request_body = Config.EraseSdRequest()
        request_body.sd_num = sd_num
        request_body.addr_start = 0
        request_body.addr_end = PAGE_COUNT - 1
        (flag, res) = log(cmd.name)(req_inst.post)(cmd, request_body)

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)

        time.sleep(5)
        
        for addr in [0, 1000000, 2000000]:
            (flag, out_buf) = self.sup.read_page(req_inst, sd_num, addr, resp_flag)
            assert(flag == resp_flag)
            if not flag:
                assert(out_buf == None)
            else:
                assert(out_buf == [0]*self.sup.PAGE_SIZE)
            print(out_buf)
            time.sleep(2)

        pass


class Test_Sensor():
    MPU0, MPU1 = 0, 1

    @pytest.mark.parametrize("mpu_num,resp_flag", [(MPU0, True),(MPU1, False)])
    def test_read_mpu(self, req_inst : Request, mpu_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
        cmd = Config.Commands.READ_MPU

        request_body = Config.ReadMpuRequest()
        request_body.mpu_num = mpu_num
        (flag, res) = log(cmd.name)(req_inst.get)(cmd, request_body)

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)

        logger.info(f"[RES] - PASSED\n")

        pass


class Test_ReadData():
    def test_read_tme_by_time(self, req_inst : Request):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
        cmd = Config.Commands.READ_TME

        request_body = Config.ReadTmeRequest()
        request_body.sector_num = 0
        request_body.time = 5

        (flag, res) = log(cmd.name)(req_inst.get)(cmd, request_body, None, 44)

        assert(flag == True)

        logger.info(f"[RES] - PASSED\n")

        pass

    def test_read_tme_bunch(self, req_inst : Request):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
        cmd = Config.Commands.READ_TME_BUNCH

        request_body = Config.ReadTmeBunchRequest()
        request_body.sector_num = 0
        request_body.time = 5
        request_body.step = 3
        request_body.qty = 10

        (flag, res) = log(cmd.name)(req_inst.get)(cmd, request_body, None, 44*10)

        assert(flag == True)

        logger.info(f"[RES] - PASSED\n")

        pass