#include <application/data_storage.h>
#include <application/data_storage_config_api.h>
#include <system_abstraction/data_storage_config.h>
#include <pool-allocator/pool_allocator_port_api.h>
#include <cpp_tests/mock_board.h>
#include <singelton.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <rapidjson/document.h>

#include <fstream>
#include <iostream>
#include <array>

using namespace board;
using namespace app;
using namespace allocator;
using namespace std;
using namespace rapidjson;

volatile void _delay_ms(uint32_t delay) {
	static_cast<void>(0);
}

using ::testing::AnyNumber;


class PoolAllocatorPortMock : public Singleton<IPoolAllocatorPort>, public IPoolAllocatorPort {
friend class Singleton<IPoolAllocatorPort>;
public:
    MOCK_METHOD(void, AllocatorPortEnterCriticalSection, (), (override));
    MOCK_METHOD(void, AllocatorPortExitCriticalSection, (), (override));
};

class DataStorageTestHelper: public ::testing::Test
{
public:
    DataStorageTestHelper()
    : mock_port_(Singleton<PoolAllocatorPortMock>::GetInstance())
    {
        EXPECT_CALL(mock_port_, AllocatorPortEnterCriticalSection()).Times(AnyNumber());
        EXPECT_CALL(mock_port_, AllocatorPortExitCriticalSection()).Times(AnyNumber());
    }
protected:
    PoolAllocatorPortMock& mock_port_;
};

class DataStorageSeededDataTest : public DataStorageTestHelper {
protected:
    struct MockDataStorageConfig {
    private:
        static constexpr uint32_t SECTORS_COUNT_ = 5;
        
        static constexpr uint32_t SECTOR_CASE1_FILLED_PAGE_RANGE_START_ = 0x00200000;
        static constexpr uint32_t SECTOR_CASE1_FILLED_PAGE_RANGE_LENGTH_ = 0x00000064; // 50kB
        static constexpr uint32_t SECTOR_CASE1_FILLED_PAGE_SIZE_ = sizeof(uint32_t);
        static constexpr uint32_t SECTOR_CASE1_FILLED_PAGE_MAX_NUM_ = board::SD_PAGE_SIZE / SECTOR_CASE1_FILLED_PAGE_SIZE_;
        
        static constexpr uint32_t SECTOR_CASE2_NOT_FILLED_PAGE_RANGE_START_ = 0x00300000;
        static constexpr uint32_t SECTOR_CASE2_NOT_FILLED_PAGE_RANGE_LENGTH_ = 0x00000064; // 50kB
        static constexpr uint32_t SECTOR_CASE2_NOT_FILLED_PAGE_SIZE_ = sizeof(uint32_t);
        static constexpr uint32_t SECTOR_CASE2_NOT_FILLED_PAGE_MAX_NUM_ = board::SD_PAGE_SIZE / SECTOR_CASE2_NOT_FILLED_PAGE_SIZE_;

        static constexpr uint32_t SECTOR_CASE3_FILLED_PAGE_RANGE_START_ = 0x00400000;
        static constexpr uint32_t SECTOR_CASE3_FILLED_PAGE_RANGE_LENGTH_ = 0x00000064; // 50kB
        static constexpr uint32_t SECTOR_CASE3_FILLED_PAGE_SIZE_ = sizeof(uint32_t);
        static constexpr uint32_t SECTOR_CASE3_FILLED_PAGE_MAX_NUM_ = board::SD_PAGE_SIZE / SECTOR_CASE3_FILLED_PAGE_SIZE_;
        
        static constexpr uint32_t SECTOR_CASE4_NOT_FILLED_PAGE_RANGE_START_ = 0x00500000;
        static constexpr uint32_t SECTOR_CASE4_NOT_FILLED_PAGE_RANGE_LENGTH_ = 0x00000064; // 50kB
        static constexpr uint32_t SECTOR_CASE4_NOT_FILLED_PAGE_SIZE_ = sizeof(uint32_t);
        static constexpr uint32_t SECTOR_CASE4_NOT_FILLED_PAGE_MAX_NUM_ = board::SD_PAGE_SIZE / SECTOR_CASE4_NOT_FILLED_PAGE_SIZE_;

        static constexpr uint32_t SECTOR_NOT_FOUND_ERROR_PAGE_RANGE_START_ = 0x00600000;
        static constexpr uint32_t SECTOR_NOT_FOUND_ERROR_PAGE_RANGE_LENGTH_ = 0x00000064; // 50kB
        static constexpr uint32_t SECTOR_NOT_FOUND_ERROR_PAGE_SIZE_ = sizeof(uint32_t);
        static constexpr uint32_t SECTOR_NOT_FOUND_ERROR_PAGE_MAX_NUM_ = board::SD_PAGE_SIZE / SECTOR_NOT_FOUND_ERROR_PAGE_SIZE_;

        static constexpr std::array<TDataStorageSectorPars, SECTORS_COUNT_> SECTORS_PARS_ = {
            {
                {
                    SECTOR_CASE1_FILLED_PAGE_RANGE_START_,
                    SECTOR_CASE1_FILLED_PAGE_RANGE_LENGTH_,
                    SECTOR_CASE1_FILLED_PAGE_SIZE_,
                    SECTOR_CASE1_FILLED_PAGE_MAX_NUM_
                },
                {
                    SECTOR_CASE2_NOT_FILLED_PAGE_RANGE_START_,
                    SECTOR_CASE2_NOT_FILLED_PAGE_RANGE_LENGTH_,
                    SECTOR_CASE2_NOT_FILLED_PAGE_SIZE_,
                    SECTOR_CASE2_NOT_FILLED_PAGE_MAX_NUM_
                },
                {
                    SECTOR_CASE3_FILLED_PAGE_RANGE_START_,
                    SECTOR_CASE3_FILLED_PAGE_RANGE_LENGTH_,
                    SECTOR_CASE3_FILLED_PAGE_SIZE_,
                    SECTOR_CASE3_FILLED_PAGE_MAX_NUM_
                },
                {
                    SECTOR_CASE4_NOT_FILLED_PAGE_RANGE_START_,
                    SECTOR_CASE4_NOT_FILLED_PAGE_RANGE_LENGTH_,
                    SECTOR_CASE4_NOT_FILLED_PAGE_SIZE_,
                    SECTOR_CASE4_NOT_FILLED_PAGE_MAX_NUM_
                },
                {
                    SECTOR_NOT_FOUND_ERROR_PAGE_RANGE_START_,
                    SECTOR_NOT_FOUND_ERROR_PAGE_RANGE_LENGTH_,
                    SECTOR_NOT_FOUND_ERROR_PAGE_SIZE_,
                    SECTOR_NOT_FOUND_ERROR_PAGE_MAX_NUM_
                }
            }
        };

    public:
        static constexpr TDataStorageSectorPars GetSectorPars(uint8_t sector_num) {
            assert(sector_num < SECTORS_COUNT_);
            return SECTORS_PARS_[sector_num];
        }

        static constexpr  uint32_t GetSectorsCount() {
            return SECTORS_COUNT_;
        }

        enum class Sector {
            Form1FilledPageMockData = 0x00,
            Form1NotFilledPageMockData = 0x01,
            Form2FilledPageMockData = 0x02,
            Form2NotFilledPageMockData = 0x03,
            NotFoundErrorPageMockData = 0x04
        };
    };
    
    typedef struct __attribute__ ((__packed__)) {
        uint32_t time;
    } TMockData;

    PoolAllocatorPortMock& mock_port_;
    MockBoard& obc_;
    DataStorage<MockBoard, PoolAllocatorPortMock, MockDataStorageConfig>& data_storage_;

    uint32_t number_of_seeded_pages_case1_and_case2_ = 10;
    uint32_t last_seeded_time_stamp_case2_ = 1010;
    uint32_t page_to_write_case3_and_case4_ = 26;
    uint32_t last_seeded_time_num_case4_ = 40;
    uint32_t last_seeded_time_stamp_case4_ = (100 + page_to_write_case3_and_case4_ - 1) * 128 + last_seeded_time_num_case4_;
    uint32_t number_of_seeded_pages_case5_ = 10;
    uint32_t skip_step_case5_ = 4;

    DataStorageSeededDataTest()
        : DataStorageTestHelper(),
        mock_port_(Singleton<PoolAllocatorPortMock>::GetInstance()),
        obc_(Singleton<MockBoard>::GetInstance()),
        data_storage_(Singleton<DataStorage<MockBoard, PoolAllocatorPortMock, MockDataStorageConfig>>::GetInstance())
        {
            uint8_t page[data_storage_.BUFFER_SIZE];

            { // Form1FilledPageMockData
                uint32_t current_time = 0;
                TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(MockDataStorageConfig::Sector::Form1FilledPageMockData));

                for (uint16_t addr_shift = 0; addr_shift < pars.tme_range_length; ++addr_shift) {
                    if (addr_shift < number_of_seeded_pages_case1_and_case2_) {
                        memset(page, 0xff, sizeof(page));
                        for (uint16_t num = 0; num < pars.tme_max_num_in_page; ++num) {
                            current_time = addr_shift * pars.tme_max_num_in_page + num;
                            memcpy(&page[sizeof(uint32_t) * num], &current_time, sizeof(uint32_t));
                        }
                    } else {
                        memset(page, 0x00, sizeof(page));
                    }
                    obc_.SdPageWrite(Sd::kNum1, pars.tme_range_start + addr_shift, page);   
                }
            }

            { // Form1NotFilledPageMockData
                uint32_t current_time = 0;
                TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(MockDataStorageConfig::Sector::Form1NotFilledPageMockData));

                for (uint16_t addr_shift = 0; addr_shift < pars.tme_range_length; ++addr_shift) {
                    if (addr_shift < number_of_seeded_pages_case1_and_case2_ && current_time < last_seeded_time_stamp_case2_) {
                        memset(page, 0xff, sizeof(page));
                        for (uint16_t num = 0; num < pars.tme_max_num_in_page; ++num) {
                            current_time = addr_shift * pars.tme_max_num_in_page + num;
                            memcpy(&page[sizeof(uint32_t) * num], &current_time, sizeof(uint32_t));
                            if (current_time == last_seeded_time_stamp_case2_) {
                                break;
                            }
                        } 
                    } else {
                        memset(page, 0x00, sizeof(page));
                    }
                    obc_.SdPageWrite(Sd::kNum1, pars.tme_range_start + addr_shift, page);
                }
            }

            { // Form2FilledPageMockData
                uint32_t current_time = 0;
                TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(MockDataStorageConfig::Sector::Form2FilledPageMockData));

                for (uint16_t addr_shift = 0; addr_shift < pars.tme_range_length; ++addr_shift) {
                    memset(page, 0xff, sizeof(page));
                    if (addr_shift < page_to_write_case3_and_case4_) {
                        for (uint16_t num = 0; num < pars.tme_max_num_in_page; ++num) {
                            current_time = (pars.tme_range_length + addr_shift) * pars.tme_max_num_in_page + num;
                            memcpy(&page[sizeof(uint32_t) * num], &current_time, sizeof(uint32_t));
                        }
                    } else {
                        for (uint16_t num = 0; num < pars.tme_max_num_in_page; ++num) {
                            current_time = addr_shift * pars.tme_max_num_in_page + num;
                            memcpy(&page[sizeof(uint32_t) * num], &current_time, sizeof(uint32_t));
                        } 
                    }
                    obc_.SdPageWrite(Sd::kNum1, pars.tme_range_start + addr_shift, page);
                }
            }

            { // Form2NotFilledPageMockData
                uint32_t current_time = 0;
                TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(MockDataStorageConfig::Sector::Form2NotFilledPageMockData));

                for (uint16_t addr_shift = 0; addr_shift < pars.tme_range_length; ++addr_shift) {
                    memset(page, 0xff, sizeof(page));
                    if (addr_shift < page_to_write_case3_and_case4_ && current_time < last_seeded_time_stamp_case4_) {
                        for (uint16_t num = 0; num < pars.tme_max_num_in_page; ++num) {
                            current_time = (pars.tme_range_length + addr_shift) * pars.tme_max_num_in_page + num;
                            memcpy(&page[sizeof(uint32_t) * num], &current_time, sizeof(uint32_t));
                            if (current_time == last_seeded_time_stamp_case4_) {
                                break;
                            }
                        } 
                    } else {
                        for (uint16_t num = 0; num < pars.tme_max_num_in_page; ++num) {
                            current_time = addr_shift * pars.tme_max_num_in_page + num;
                            memcpy(&page[sizeof(uint32_t) * num], &current_time, sizeof(uint32_t));
                        }
                    }
                    obc_.SdPageWrite(Sd::kNum1, pars.tme_range_start + addr_shift, page);
                }
            }

            { // NotFoundErrorPageMockData
                uint32_t current_time = 0;
                TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(MockDataStorageConfig::Sector::NotFoundErrorPageMockData));

                for (uint16_t addr_shift = 0; addr_shift < pars.tme_range_length; ++addr_shift) {
                    if (addr_shift < number_of_seeded_pages_case5_) {
                        memset(page, 0xff, sizeof(page));
                        for (uint16_t num = 0; num < pars.tme_max_num_in_page; ++num) {
                            current_time = skip_step_case5_ * (addr_shift * pars.tme_max_num_in_page + num);
                            memcpy(&page[sizeof(uint32_t) * num], &current_time, sizeof(uint32_t));
                        }
                    } else {
                        memset(page, 0x00, sizeof(page));
                    }
                    obc_.SdPageWrite(Sd::kNum1, pars.tme_range_start + addr_shift, page);   
                }
            }
        }

    ~DataStorageSeededDataTest() override = default;
};

class DataStorageMagnTest : public DataStorageTestHelper {
protected:
    PoolAllocatorPortMock& mock_port_;
    MockBoard& obc_;
    DataStorage<MockBoard, PoolAllocatorPortMock, DataStorageConfig>& data_storage_;
    
    static Document ReadJson(string path) {
        ifstream file(path);
        Document doc; 

        if (file.is_open()) {
            string json((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()); 
            doc.Parse(json.c_str()); 
        }
        file.close();

        assert(doc.IsObject());
        assert(doc["data"].IsArray());

        return doc;
    }

    std::vector<TObcMagnTme> ParseObcMagnTmes(const Document& json) {
        const Value& data = json["data"];
        std::vector<TObcMagnTme> tme_vec(data.Size());
        for (SizeType j = 0; j < data.Size(); ++j) {
            tme_vec[j].time = data[j]["time"].GetUint();
            const Value& sensors = data[j]["sensors"];
            for (SizeType i = 0; i < sensors.Size(); ++i) {           
                tme_vec[j].sensors[i].B_X = sensors[i]["B_X"].GetInt();
                tme_vec[j].sensors[i].B_Y = sensors[i]["B_Y"].GetInt();
                tme_vec[j].sensors[i].B_Z = sensors[i]["B_Z"].GetInt();
                tme_vec[j].sensors[i].G_X = sensors[i]["G_X"].GetInt();
                tme_vec[j].sensors[i].G_Y = sensors[i]["G_Y"].GetInt();
                tme_vec[j].sensors[i].G_Z = sensors[i]["G_Z"].GetInt();
                tme_vec[j].sensors[i].A_X = sensors[i]["A_X"].GetInt(); 
                tme_vec[j].sensors[i].A_Y = sensors[i]["A_Y"].GetInt();
                tme_vec[j].sensors[i].A_Z = sensors[i]["A_Z"].GetInt(); 
                tme_vec[j].sensors[i].T = sensors[i]["T"].GetInt();
            }
        }
        return tme_vec;
    }

    int32_t SdPageClear(Sd sd_num, uint32_t page_to_clear) {
        uint8_t zero_page[data_storage_.BUFFER_SIZE];
        memset(zero_page, 0x00, data_storage_.BUFFER_SIZE);
        return obc_.SdPageWrite(sd_num, page_to_clear, zero_page);
    }

    DataStorageMagnTest()
        : DataStorageTestHelper(),
        mock_port_(Singleton<PoolAllocatorPortMock>::GetInstance()),
        obc_(Singleton<MockBoard>::GetInstance()),
        data_storage_(Singleton<DataStorage<MockBoard, PoolAllocatorPortMock, DataStorageConfig>>::GetInstance())
        {
            using Sector = DataStorageConfig::Sector;
            const Document json = ReadJson("data\\data_storage_magn_seeded_data.json"s);
            const std::vector<TObcMagnTme> tme_vec = ParseObcMagnTmes(json);
            uint8_t page_to_seed[data_storage_.BUFFER_SIZE];
            memset(page_to_seed, 0xff, data_storage_.BUFFER_SIZE);
            const uint32_t addr_to_seed = 0;
            uint32_t res;
            for (uint8_t j = 0; j < tme_vec.size(); ++j) {
                memcpy(&page_to_seed[j*sizeof(tme_vec[j])], &tme_vec[j], sizeof(tme_vec[j]));
            }
            if ( obc_.GetSdStatus(Sd::kNum1) == Status::kWorked ) {
                obc_.SdPageWrite(static_cast<Sd>(Sd::kNum1), addr_to_seed, page_to_seed);
                data_storage_.template InitSector<Sd::kNum1, Sector::ObcSensors>();
            }
            if ( obc_.GetSdStatus(Sd::kNum2) == Status::kWorked ) {
                obc_.SdPageWrite(static_cast<Sd>(Sd::kNum2), addr_to_seed, page_to_seed);
                data_storage_.template InitSector<Sd::kNum2, Sector::ObcSensors>();
            }
        }

    ~DataStorageMagnTest() override = default;
};


TEST_F(DataStorageMagnTest, ReadMagnTmeByTime) {
    using Sector = DataStorageConfig::Sector;
    int32_t res;
    const uint32_t time_stamp_to_read_tme = 16;
    const uint32_t page_to_clear = 1;
    TObcMagnTme data = {0};

    if ( obc_.GetSdStatus(Sd::kNum1) == Status::kWorked ) {
        SdPageClear(Sd::kNum1, page_to_clear);

        res = data_storage_.template InitSector<Sd::kNum1,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);

        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::ObcSensors>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(time_stamp_to_read_tme, data.time);
    }
    if ( obc_.GetSdStatus(Sd::kNum2) == Status::kWorked ) {
        SdPageClear(Sd::kNum2, page_to_clear);

        res = data_storage_.template InitSector<Sd::kNum2,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);

        res = data_storage_.template ReadTmeByTime<Sd::kNum2,Sector::ObcSensors>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(time_stamp_to_read_tme, data.time);
    } 
}

TEST_F(DataStorageMagnTest, ReadMagnTmeByTimeGreateThenNewestTimeError) {
    using Sector = DataStorageConfig::Sector;
    int32_t res;
    const uint32_t time_stamp_to_read_tme = 19;
    const uint32_t page_to_clear = 1;
    TObcMagnTme data = {0};

    if ( obc_.GetSdStatus(Sd::kNum1) == Status::kWorked ) {
        SdPageClear(Sd::kNum1, page_to_clear);

        res = data_storage_.template InitSector<Sd::kNum1,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);

        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::ObcSensors>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_GREATER_THEN_NEWEST_TIME);
    }
    if ( obc_.GetSdStatus(Sd::kNum2) == Status::kWorked ) {
        SdPageClear(Sd::kNum2, page_to_clear);   

        res = data_storage_.template InitSector<Sd::kNum2,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);

        res = data_storage_.template ReadTmeByTime<Sd::kNum2,Sector::ObcSensors>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_GREATER_THEN_NEWEST_TIME);
    } 
}

TEST_F(DataStorageMagnTest, ReadMagnTmeByTimeLessThenOldestTimeError) {
    using Sector = DataStorageConfig::Sector;
    int32_t res;
    const uint32_t time_stamp_to_read_tme = 3;
    const uint32_t page_to_clear = 1;
    TObcMagnTme data = {0};

    if ( obc_.GetSdStatus(Sd::kNum1) == Status::kWorked ) {
        SdPageClear(Sd::kNum1, page_to_clear);

        res = data_storage_.template InitSector<Sd::kNum1,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);

        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::ObcSensors>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_LESS_THEN_OLDEST_TIME);
    }
    if ( obc_.GetSdStatus(Sd::kNum2) == Status::kWorked ) {
        SdPageClear(Sd::kNum2, page_to_clear);
        
        res = data_storage_.template InitSector<Sd::kNum2,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);

        res = data_storage_.template ReadTmeByTime<Sd::kNum2,Sector::ObcSensors>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_LESS_THEN_OLDEST_TIME);
    } 
}

TEST_F(DataStorageMagnTest, ReadMagnTmeByTimeTmeNotFoundError) {
    using Sector = DataStorageConfig::Sector;
    int32_t res;
    const uint32_t time_stamp_to_read_tme = 8;
    const uint32_t page_to_clear = 1;
    TObcMagnTme data = {0};

    if ( obc_.GetSdStatus(Sd::kNum1) == Status::kWorked ) {
        SdPageClear(Sd::kNum1, page_to_clear);

        res = data_storage_.template InitSector<Sd::kNum1,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);

        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::ObcSensors>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_TME_NOT_FOUND);
    }
    if ( obc_.GetSdStatus(Sd::kNum2) == Status::kWorked ) {
        SdPageClear(Sd::kNum2, page_to_clear);

        res = data_storage_.template InitSector<Sd::kNum2,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);

        res = data_storage_.template ReadTmeByTime<Sd::kNum2,Sector::ObcSensors>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_TME_NOT_FOUND);
    }
}

TEST_F(DataStorageMagnTest, ReadMagnTmeBunchMissingTmes) {
    using Sector = DataStorageConfig::Sector;
    int32_t res;
    const uint32_t tme_time = 1;
    const uint32_t tme_step = 1;
    const uint32_t tme_qty = 10;
    std::array<TObcMagnTme, 10> tmes = {0};
    std::array<uint32_t, 10> tmes_time_true = {
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        5,
        6,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        10
    };

    TDataStorageSectorPars pars = DataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::ObcSensors));

    res = data_storage_.template InitSector<Sd::kNum1,Sector::ObcSensors>();
    EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
    res = data_storage_.template ReadTmeBunch<Sd::kNum1,Sector::ObcSensors>(tme_time, tme_step, tme_qty, reinterpret_cast<uint8_t*>(&tmes));
    EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
    for (uint8_t i = 0; i < tmes_time_true.size(); ++i) {
        tmes_time_true[i] = tmes[i].time;
    }
}

TEST_F(DataStorageMagnTest, AddMagnTmeToSdPage) {
    using Sector = DataStorageConfig::Sector;

    const Document json = ReadJson("data\\add_magn_tme_to_sd_page_input.json"s);
    const std::vector<TObcMagnTme> tme_vec = ParseObcMagnTmes(json);

    const uint32_t expected_page = 1;

    int32_t res;

    if ( obc_.GetSdStatus(Sd::kNum1) == Status::kWorked ) {
        SdPageClear(Sd::kNum1, expected_page);
        res = data_storage_.template InitSector<Sd::kNum1,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
    }
    if ( obc_.GetSdStatus(Sd::kNum2) == Status::kWorked ) {
        SdPageClear(Sd::kNum2, expected_page);
        res = data_storage_.template InitSector<Sd::kNum2,Sector::ObcSensors>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
    }

    uint8_t ref_page[data_storage_.BUFFER_SIZE];
    memset(ref_page, 0xff, data_storage_.BUFFER_SIZE);

    for (uint8_t j = 0; j < tme_vec.size(); ++j) {
        TObcMagnTme tme = tme_vec[j];
        if ( obc_.GetSdStatus(Sd::kNum1) == Status::kWorked ) {
            res = data_storage_.template AddTmeToSd<Sd::kNum1,Sector::ObcSensors>(reinterpret_cast<uint8_t*>(&tme));
            EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        }
        if ( obc_.GetSdStatus(Sd::kNum2) == Status::kWorked ) {
            res = data_storage_.template AddTmeToSd<Sd::kNum2,Sector::ObcSensors>(reinterpret_cast<uint8_t*>(&tme));
            EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        }

        memcpy(&ref_page[j*sizeof(tme)], &tme, sizeof(tme));
	}

    uint32_t addr = expected_page;
    uint8_t page[data_storage_.BUFFER_SIZE];
    for (uint8_t sd_num = 0; sd_num < board::SD_COUNT; ++sd_num) {
        memset(page, 0, data_storage_.BUFFER_SIZE);
        obc_.SdPageRead(static_cast<Sd>(sd_num), addr, page);
        for (uint16_t i = 0; i < data_storage_.BUFFER_SIZE; ++i) {
            EXPECT_EQ(ref_page[i], page[i]);
        }
    }  
}

TEST_F(DataStorageSeededDataTest, FindPageToWrite) {
    using Sector = MockDataStorageConfig::Sector;
    uint32_t page_to_write;
    int32_t res;

    {
        const TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1FilledPageMockData));
        res = data_storage_.template FindPageToWrite<Sd::kNum1,Sector::Form1FilledPageMockData>(&page_to_write);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_to_write, pars.tme_range_start + number_of_seeded_pages_case1_and_case2_);
    }

    {   
        const TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1NotFilledPageMockData));
        res = data_storage_.template FindPageToWrite<Sd::kNum1,Sector::Form1NotFilledPageMockData>(&page_to_write);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const uint32_t number_of_seeded_pages = last_seeded_time_stamp_case2_ / pars.tme_max_num_in_page;
        EXPECT_EQ(page_to_write, pars.tme_range_start + number_of_seeded_pages);
    }

    {   
        const TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
        res = data_storage_.template FindPageToWrite<Sd::kNum1,Sector::Form2FilledPageMockData>(&page_to_write);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_to_write, pars.tme_range_start + page_to_write_case3_and_case4_);
    }

    {   
        const TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2NotFilledPageMockData));
        res = data_storage_.template FindPageToWrite<Sd::kNum1,Sector::Form2NotFilledPageMockData>(&page_to_write);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_to_write, pars.tme_range_start + page_to_write_case3_and_case4_ - 1);
    } 
}

TEST_F(DataStorageSeededDataTest, InitSector) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;

    {
        const TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const TDataStorageSectorVars vars = data_storage_.template GetSectorVars<Sd::kNum1,Sector::Form1FilledPageMockData>();
        EXPECT_EQ(vars.page_to_write, pars.tme_range_start + number_of_seeded_pages_case1_and_case2_);
        EXPECT_EQ(vars.tme_num_in_page, 0);
    }

    {
        const TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1NotFilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const TDataStorageSectorVars vars = data_storage_.template GetSectorVars<Sd::kNum1,Sector::Form1NotFilledPageMockData>();
        const uint32_t number_of_seeded_pages = last_seeded_time_stamp_case2_ / pars.tme_max_num_in_page;
        EXPECT_EQ(vars.page_to_write, pars.tme_range_start + number_of_seeded_pages);
        const uint32_t first_not_seeded_num_in_page = last_seeded_time_stamp_case2_ % pars.tme_max_num_in_page + 1;
        EXPECT_EQ(vars.tme_num_in_page, first_not_seeded_num_in_page);
    }

    {
        const TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const TDataStorageSectorVars vars = data_storage_.template GetSectorVars<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(vars.page_to_write, pars.tme_range_start + page_to_write_case3_and_case4_);
        EXPECT_EQ(vars.tme_num_in_page, 0);
    }

    {
        const TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2NotFilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const TDataStorageSectorVars vars = data_storage_.template GetSectorVars<Sd::kNum1,Sector::Form2NotFilledPageMockData>();
        const uint32_t number_of_seeded_pages = last_seeded_time_stamp_case4_ / pars.tme_max_num_in_page;
        EXPECT_EQ(vars.page_to_write, pars.tme_range_start + number_of_seeded_pages - pars.tme_range_length);
        const uint32_t first_not_seeded_num_in_page = last_seeded_time_stamp_case4_ % pars.tme_max_num_in_page + 1;
        EXPECT_EQ(vars.tme_num_in_page, first_not_seeded_num_in_page);
    }
}

TEST_F(DataStorageSeededDataTest, FindPageToRead) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;
    const uint32_t time_stamp_to_find_page = 1000;
    uint32_t page_with_time_stamp; 

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const uint32_t page_with_time_stamp_ref = pars.tme_range_start + (time_stamp_to_find_page / pars.tme_max_num_in_page);
        res = data_storage_.template FindPageToRead<Sd::kNum1,Sector::Form1FilledPageMockData>(time_stamp_to_find_page, &page_with_time_stamp);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_with_time_stamp, page_with_time_stamp_ref);
    }

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1NotFilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const uint32_t page_with_time_stamp_ref = pars.tme_range_start + (time_stamp_to_find_page / pars.tme_max_num_in_page);
        res = data_storage_.template FindPageToRead<Sd::kNum1,Sector::Form1NotFilledPageMockData>(time_stamp_to_find_page, &page_with_time_stamp);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_with_time_stamp, page_with_time_stamp_ref);
    }

    {
        const uint32_t time_stamp_to_find_page_1 = 5000;
        const uint32_t time_stamp_to_find_page_2 = 15000;
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const uint32_t page_with_time_stamp_ref_1 = pars.tme_range_start + (time_stamp_to_find_page_1 / pars.tme_max_num_in_page);
        res = data_storage_.template FindPageToRead<Sd::kNum1,Sector::Form2FilledPageMockData>(time_stamp_to_find_page_1, &page_with_time_stamp);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_with_time_stamp, page_with_time_stamp_ref_1);
        const uint32_t page_with_time_stamp_ref_2 = pars.tme_range_start + (time_stamp_to_find_page_2 / pars.tme_max_num_in_page) - pars.tme_range_length;
        res = data_storage_.template FindPageToRead<Sd::kNum1,Sector::Form2FilledPageMockData>(time_stamp_to_find_page_2, &page_with_time_stamp);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_with_time_stamp, page_with_time_stamp_ref_2);
    }

    {
        const uint32_t time_stamp_to_find_page_1 = 5000;
        const uint32_t time_stamp_to_find_page_2 = 15000;
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2NotFilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        const uint32_t page_with_time_stamp_ref_1 = pars.tme_range_start + (time_stamp_to_find_page_1 / pars.tme_max_num_in_page);
        res = data_storage_.template FindPageToRead<Sd::kNum1,Sector::Form2NotFilledPageMockData>(time_stamp_to_find_page_1, &page_with_time_stamp);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_with_time_stamp, page_with_time_stamp_ref_1);
        const uint32_t page_with_time_stamp_ref_2 = pars.tme_range_start + (time_stamp_to_find_page_2 / pars.tme_max_num_in_page) - pars.tme_range_length;
        res = data_storage_.template FindPageToRead<Sd::kNum1,Sector::Form2NotFilledPageMockData>(time_stamp_to_find_page_2, &page_with_time_stamp);
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(page_with_time_stamp, page_with_time_stamp_ref_2);
    }
}

TEST_F(DataStorageSeededDataTest, ReadTmeByTime) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;
    const uint32_t time_stamp_to_read_tme = 1000;
    TMockData data = {0};

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form1FilledPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(time_stamp_to_read_tme, data.time);
    }

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1NotFilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form1NotFilledPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(time_stamp_to_read_tme, data.time);
    }

    {
        const uint32_t time_stamp_to_read_tme_1 = 5000;
        const uint32_t time_stamp_to_read_tme_2 = 15000;
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form2FilledPageMockData>(time_stamp_to_read_tme_1, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(time_stamp_to_read_tme_1, data.time);
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form2FilledPageMockData>(time_stamp_to_read_tme_2, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(time_stamp_to_read_tme_2, data.time);
    }

    {
        const uint32_t time_stamp_to_read_tme_1 = 5000;
        const uint32_t time_stamp_to_read_tme_2 = 15000;
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form2NotFilledPageMockData>(time_stamp_to_read_tme_1, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(time_stamp_to_read_tme_1, data.time);
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form2NotFilledPageMockData>(time_stamp_to_read_tme_2, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        EXPECT_EQ(time_stamp_to_read_tme_2, data.time);
    }
}

TEST_F(DataStorageSeededDataTest, ReadTmeByTimeGreateThenNewestTimeError) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;
    const uint32_t time_stamp_to_read_tme = 18000;
    TMockData data = {0};

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form1FilledPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_GREATER_THEN_NEWEST_TIME);
    }

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1NotFilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form1NotFilledPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_GREATER_THEN_NEWEST_TIME);
    }

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form2FilledPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_GREATER_THEN_NEWEST_TIME);
    }

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form2NotFilledPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_GREATER_THEN_NEWEST_TIME);
    }
}

TEST_F(DataStorageSeededDataTest, ReadTmeByTimeLessThenOldestTimeError) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;
    const uint32_t time_stamp_to_read_tme = 3000;
    TMockData data = {0};

    { }

    { }

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form2FilledPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_LESS_THEN_OLDEST_TIME);
    }

    {
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::Form2NotFilledPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_LESS_THEN_OLDEST_TIME);
    }
}

TEST_F(DataStorageSeededDataTest, ReadTmeByTimeTmeNotFoundError) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;

    TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::NotFoundErrorPageMockData));

    const uint32_t time_stamp_to_read_tme_max = pars.tme_max_num_in_page * skip_step_case5_ * number_of_seeded_pages_case1_and_case2_ - skip_step_case5_;
    TMockData data = {0};

    res = data_storage_.template InitSector<Sd::kNum1,Sector::NotFoundErrorPageMockData>();
    EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
    for (uint32_t time_stamp_to_read_tme = skip_step_case5_; time_stamp_to_read_tme <= time_stamp_to_read_tme_max; ++time_stamp_to_read_tme) {
        if (time_stamp_to_read_tme % skip_step_case5_ == 0) {
            continue;
        }
        res = data_storage_.template ReadTmeByTime<Sd::kNum1,Sector::NotFoundErrorPageMockData>(time_stamp_to_read_tme, reinterpret_cast<uint8_t*>(&data));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_TME_NOT_FOUND);
    }

}

TEST_F(DataStorageSeededDataTest, ReadTmeBunch) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;
    const uint32_t tme_time = 900;
    const uint32_t tme_step = 5;
    const uint32_t tme_qty = 10;

    std::array<TMockData, 10> tmes_true = {0};
    for (uint8_t i = 0; i < tmes_true.size(); ++i) {
        tmes_true[i].time = tme_time + tme_step * i;
    }

    {
        std::array<TMockData, 10> tmes = {0};
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeBunch<Sd::kNum1,Sector::Form1FilledPageMockData>(tme_time, tme_step, tme_qty, reinterpret_cast<uint8_t*>(&tmes));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        for (uint8_t i = 0; i < tmes_true.size(); ++i) {
            EXPECT_EQ(tmes_true[i].time, tmes[i].time);
        }
    }

    {
        std::array<TMockData, 10> tmes = {0};
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form1NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form1NotFilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeBunch<Sd::kNum1,Sector::Form1NotFilledPageMockData>(tme_time, tme_step, tme_qty, reinterpret_cast<uint8_t*>(&tmes));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        for (uint8_t i = 0; i < tmes_true.size(); ++i) {
            EXPECT_EQ(tmes_true[i].time, tmes[i].time);
        }
    }
}

TEST_F(DataStorageSeededDataTest, ReadTmeBunchCrossBorder) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;
    const uint32_t tme_time = 12780;
    const uint32_t tme_step = 5;
    const uint32_t tme_qty = 10;

    std::array<TMockData, 10> tmes_true = {0};
    for (uint8_t i = 0; i < tmes_true.size(); ++i) {
        tmes_true[i].time = tme_time + tme_step * i;
    }

    {
        std::array<TMockData, 10> tmes = {0};
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeBunch<Sd::kNum1,Sector::Form2FilledPageMockData>(tme_time, tme_step, tme_qty, reinterpret_cast<uint8_t*>(&tmes));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        for (uint8_t i = 0; i < tmes_true.size(); ++i) {
            EXPECT_EQ(tmes_true[i].time, tmes[i].time);
        }
    }

    {
        std::array<TMockData, 10> tmes = {0};
        TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2NotFilledPageMockData));
        res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2NotFilledPageMockData>();
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
        res = data_storage_.template ReadTmeBunch<Sd::kNum1,Sector::Form2NotFilledPageMockData>(tme_time, tme_step, tme_qty, reinterpret_cast<uint8_t*>(&tmes));
        EXPECT_EQ(res, data_storage_.ERROR_CODE_OK);
        for (uint8_t i = 0; i < tmes_true.size(); ++i) {
            EXPECT_EQ(tmes_true[i].time, tmes[i].time);
        }
    }
}

TEST_F(DataStorageSeededDataTest, ReadTmeBunchGreaterThenNewestTimeError) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;
    const uint32_t tme_time = 16200;
    const uint32_t tme_step = 5;
    const uint32_t tme_qty = 10;
    std::array<TMockData, 10> tmes = {0};
    std::array<TMockData, 10> tmes_true = {0};
    for (uint8_t i = 0; i < tmes_true.size(); ++i) {
        tmes_true[i].time = tme_time + tme_step * i;
    }

    TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
    res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
    EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
    res = data_storage_.template ReadTmeBunch<Sd::kNum1,Sector::Form2FilledPageMockData>(tme_time, tme_step, tme_qty, reinterpret_cast<uint8_t*>(&tmes));
    EXPECT_EQ(res, data_storage_.ERROR_CODE_GREATER_THEN_NEWEST_TIME);
}

TEST_F(DataStorageSeededDataTest, ReadTmeBunchLessThenOldestTimeError) {
    using Sector = MockDataStorageConfig::Sector;
    int32_t res;
    const uint32_t tme_time = 3300;
    const uint32_t tme_step = 5;
    const uint32_t tme_qty = 10;
    std::array<TMockData, 10> tmes = {0};
    std::array<TMockData, 10> tmes_true = {0};
    for (uint8_t i = 0; i < tmes_true.size(); ++i) {
        tmes_true[i].time = tme_time + tme_step * i;
    }

    TDataStorageSectorPars pars = MockDataStorageConfig::GetSectorPars(static_cast<uint8_t>(Sector::Form2FilledPageMockData));
    res = data_storage_.template InitSector<Sd::kNum1,Sector::Form2FilledPageMockData>();
    EXPECT_EQ(res, data_storage_.ERROR_CODE_OK); 
    res = data_storage_.template ReadTmeBunch<Sd::kNum1,Sector::Form2FilledPageMockData>(tme_time, tme_step, tme_qty, reinterpret_cast<uint8_t*>(&tmes));
    EXPECT_EQ(res, data_storage_.ERROR_CODE_LESS_THEN_OLDEST_TIME);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}