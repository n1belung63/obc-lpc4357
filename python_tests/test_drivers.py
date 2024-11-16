import pytest, time, logging, logging.config, os, json
import yaml
import inspect


from config import Config 
from request import *
from parser import Parser, parse

from typing import List, Union

if not os.path.exists("logs"):
    os.makedirs("logs")

with open(os.path.join(os.path.dirname(__file__), "logging_config.yaml"), "rt") as f:
    config = yaml.safe_load(f.read())

logging.config.dictConfig(config)
logger = logging.getLogger(__name__)
parser = Parser()

def exec(func, command : Config.Commands, *args, **kwargs) -> tuple[bool, Any]:
    try:
        request_data = kwargs['request_data'] if 'request_data' in kwargs.keys() else args[0]
        cmd_dct = {
            "cmd" : command.name,
            "args" : json.loads(json.dumps(request_data, cls=CDataJSONEncoder))
        }
        logger.info(f"[CMD] - {json.dumps(cmd_dct)}")
        data = func(command, *args, **kwargs) 
        if data is not None:
            if command in parser.need_parse.keys():
                data = parse( data, parser.need_parse[ command ](request_data) )
            res_dct = {"res": json.loads(json.dumps(data, cls=CDataJSONEncoder))}
            logger.info(f"[OUT] - {json.dumps(res_dct)}")
            return (True, data)
        else:
            res_dct = {"res": "Ok"}
            logger.info(f"[OUT] - {json.dumps(res_dct)}")
            return (True, None)
    except Exception as err:
        err_dct = {"err": err.args[0]}
        logger.warning(f"[ERR] - {json.dumps(err_dct)}")
        return (False, None)  

def get_test_name(cls):
    func_name = inspect.getouterframes( inspect.currentframe() )[1][3]
    return f"{cls.__class__.__name__}.{func_name}"

@pytest.fixture(scope = "module")
def req():
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
            request_body = Config.WriteSdRequest(sd_num=sd_num, addr=addr)
            for q in range(self.QUARTERS_COUNT):
                request_body.quarter = q
                for i in range(len(request_body.data)):
                    request_body.data[i] = inp_buf[self.PAGE_QUARTER_SIZE * q + i]
                (flag, res) = exec(req.post, Config.Commands.WRITE_SD, request_body)
                assert(flag == resp_flag)
                if not flag:
                    assert(res == None)
                    return flag
                time.sleep(4)
            return flag

        def read_page(self, req : Request, sd_num : int, addr : int, resp_flag : bool) -> tuple[bool, Union[List[int],None]]:
            request_body = Config.ReadSdRequest(sd_num=sd_num, addr=addr)
            out_buf = [0]*self.PAGE_SIZE
            for q in range(self.QUARTERS_COUNT):
                request_body.quarter = q
                (flag, data) = exec(req.get, Config.Commands.READ_SD, request_body)
                assert(flag == resp_flag)
                if not flag:
                    assert(data == None)
                    return (flag, None)
                else:
                    for i in range(sizeof(data)):
                        out_buf[self.PAGE_QUARTER_SIZE * q + i] = data.data[i]
                    time.sleep(4)
            return (flag, out_buf)

    def setup_class(self):
        self.sup : Test_SdCard.Support_SdCard = self.Support_SdCard()

    SD0, SD1 = 0, 1
    test_data = [(SD0, False),(SD1, True)]

    # @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    # def test_get_status(self, req : Request, sd_num : int, resp_flag : bool):
    #     pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_read_sd(self, req : Request, sd_num : int, resp_flag : bool): 
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
    
        ADDR = 0x00200000

        (flag, out_buf) = self.sup.read_page(req, sd_num, ADDR, resp_flag)

        assert(flag == resp_flag)
        if not flag:
            assert(out_buf == None)

        logger.info(f"[RES] - PASSED")

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_write_sd(self, req : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        ADDR = 0x00200000

        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))
        flag = self.sup.write_page(req, sd_num, ADDR, inp_buf, resp_flag)
        
        assert(flag == resp_flag)

        logger.info(f"[RES] - PASSED")

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_write_and_read_sd(self, req : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")
          
        ADDR = 0x00200000

        inp_buf = [0]*self.sup.PAGE_SIZE
        flag = self.sup.write_page(req, sd_num, ADDR, inp_buf, resp_flag)
        assert(flag == resp_flag)
        (flag, out_buf) = self.sup.read_page(req, sd_num, ADDR, resp_flag)    
        assert(flag == resp_flag)
        if not flag:
            assert(out_buf == None)

        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))

        flag = self.sup.write_page(req, sd_num, ADDR, inp_buf, resp_flag) 
        assert(flag == resp_flag)
        (flag, out_buf) = self.sup.read_page(req, sd_num, ADDR, resp_flag)
        assert(flag == resp_flag)
        if not flag:
            assert(out_buf == None)
        else:
            assert(inp_buf == out_buf)

        logger.info(f"[RES] - PASSED")

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_stop_writing_sd(self, req : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        (flag, res) = exec(req.post, Config.Commands.BLOCK_SD, Config.BlockSdRequest(sd_num=sd_num))

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)

        logger.info(f"[RES] - PASSED")

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_start_writing_sd(self, req : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        (flag, res) = exec(req.post, Config.Commands.UNBLOCK_SD, Config.UnblockSdRequest(sd_num=sd_num))

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)

        logger.info(f"[RES] - PASSED")
        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_erase_sd(self, req : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        PAGE_COUNT = 1
    
        inp_buf = list(range(self.sup.PAGE_SIZE // 2)) + list(range(self.sup.PAGE_SIZE // 2))        

        for addr in range(PAGE_COUNT):
            flag = self.sup.write_page(req, sd_num, addr, inp_buf, resp_flag)  
            assert(flag == resp_flag) 
            (flag, out_buf) = self.sup.read_page(req, sd_num, addr, resp_flag)
            assert(flag == resp_flag)
            if not flag:
                assert(out_buf == None)
            else:
                assert(out_buf == inp_buf)


        (flag, res) = exec(req.post, Config.Commands.ERASE_SD, Config.EraseSdRequest(sd_num=sd_num, addr_start=0, addr_end=PAGE_COUNT-1))

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)
        
        for addr in range(PAGE_COUNT):
            (flag, out_buf) = self.sup.read_page(req, sd_num, addr, resp_flag)
            assert(flag == resp_flag)
            if not flag:
                assert(out_buf == None)
            else:
                assert(out_buf == [0]*self.sup.PAGE_SIZE)
            print(out_buf)

        pass

    @pytest.mark.parametrize("sd_num,resp_flag", test_data)
    def test_erase_sd_sector(self, req : Request, sd_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        PAGE_COUNT = 0x00200000

        (flag, res) = exec(req.post, Config.Commands.ERASE_SD, Config.EraseSdRequest(sd_num=sd_num, addr_start=0, addr_end=PAGE_COUNT-1))

        assert(flag == resp_flag)
        if not flag:
            assert(res == None)

        time.sleep(5)
        
        for addr in [0, 1000000, 2000000]:
            (flag, out_buf) = self.sup.read_page(req, sd_num, addr, resp_flag)
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
    def test_read_mpu(self, req : Request, mpu_num : int, resp_flag : bool):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        (flag, res) = exec(req.get, Config.Commands.READ_MPU, Config.ReadMpuRequest(mpu_num=mpu_num))

        try:
            assert(flag == resp_flag)
            if not flag:
                assert(res == None)
            logger.info(f"[RES] - PASSED")
        except AssertionError:
            logger.info(f"[RES] - FAILED")

        pass


class Test_ReadData():
    def test_read_tme_by_time(self, req : Request):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        (flag, res) = exec(req.get, Config.Commands.READ_TME, request_data=Config.ReadTmeRequest(sector_num=0, time=5), resp_size=44)

        try:
            assert(flag == True)
            if not flag:
                assert(res == None)
            logger.info(f"[RES] - PASSED")
        except AssertionError:
            logger.info(f"[RES] - FAILED")

        pass

    def test_read_tme_bunch(self, req : Request):
        test_name = get_test_name(self)
        logger.info(f"[TST] - {test_name}")

        (flag, res) = exec(req.get, Config.Commands.READ_TME_BUNCH, request_data=Config.ReadTmeBunchRequest(sector_num=0, time=5, step=3, qty=10), resp_size=44*10)

        try:
            assert(flag == True)
            if not flag:
                assert(res == None)
            logger.info(f"[RES] - PASSED")
        except AssertionError:
            logger.info(f"[RES] - FAILED")

        pass