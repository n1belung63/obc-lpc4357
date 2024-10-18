# obc-lpc4357 
## Модуль регистрации и хранения данных на SD карте

Цель данного учебного проекта заключается в знакомстве с работой операционной системы реального времени на примере FreeRTOS и в получении опыта использования C++ на микроконтроллере.

Программа реализуется на микроконтроллере LPC4357 бортового компьютера платформы СамСат[^1],[^2]. Производится опрос бортовых датчиков (два чипа mpu9250) и запись их измерений в бортовое хранилище на две SD карты. Реализована вычитка данных с бортового хранилище по UART. Планируется добавить вычитку данных с SD карт по меткам времени с заданным шагом.

[^1]: Первый спутник на платформе СамСат - [SamSat-ION](http://spaceresearch.ssau.ru/ru/samsat-ion) был запущен 27 июня 2023 года.
[^2]: Второй спутник на платформе СамСат - [СамСат-Ионосфера](http://spaceresearch.ssau.ru/ru/samsat-ionosphere) был запущен 5 ноября 2024 года.

## План работ

- [x] Написать классы C++ обёрток на I2C, SPI, UART драйвера
- [x] Написать драйвера на SD карту, микрочип mpu9250 и расширитель порта
- [x] Сделать модуль инициализации аппаратных модулей устройства
- [ ] Сделать модуль обработки ошибок
- [x] Написать C++ обёртку на FreeRTOS, должен быть реализован функционал задач, очередей, событий
- [x] Реализовать в планировщике задачу для общения с устройством по UART
- [x] Написать на Python инфраструктуру для работы с устройством
- [x] Написать на Python тесты для проверки функционала отдельных модулей устройства
- [x] Продумать конфиг хранилища данных на SD
- [x] Реализовать в планировщике задачу для опроса датчика mpu9250 и записи его данных на SD
- [x] Реализовать систему логирования и доступа к данным. Данные mpu9250 должны писаться в свой сектор. При перезагрузке данные должны продолжить писаться в память с места последней записи. По достижению конца сектора данные должны начать писаться заново. Доступ должен быть предоставлен по метке времени.
- [x] Написать mock-тесты на C++ для проверки системы логирования и доступа к данным
- [ ] Добавить RAII к RTOS где требуется

## Блок-диаграмма програмного стека
![Alt text here](images/schema.svg)

## Сборка и проверка работы приложения

Для сборки проекта используется CMake (версия не ниже 3.27). Чтобы собрать проект, необходимо выполнить следующие команды в командной строке, находясь в директории с проектом:

```
mkdir build
cd build
cmake ../ -G "Ninja"
cmake --build .
```

Проверка работы осуществляется с помощью фреймворка googletest. Тесты запускаются следующей командой из командной строки, находясь в директории с проектом:

```
build\cpp_tests\Test.exe
```
