# Транспортный справочник
Строит оптимальный автобусный маршрут в городе, может отрисовать карту маршрутов и остановок в формате SVG на основе имеющихся данных.
Обмен данными осуществляется в формате JSON. Реализованы сериализация и десериализация данных с помощью Protobuf, а также собственные библиотеки для работы с форматами JSON и SVG.
## Инструкция по использованию
### Системные требования
С++17, CMake (версия 3.10 и выше), Protobuf.
### Сборка приложения
Находясь в директории cpp-transport-catalogue, выполнить в командной строке:
``` 
cmake transport-catalogue -DCMAKE_PREFIX_PATH=/path/to/protobuf/package
cmake --build .
``` 
Вместо `/path/to/protobuf/package` нужно указать путь к пакету Protobuf.

В результате должна была создаться папка Debug с файлом transport_catalogue.exe.
### Создание базы остановок и маршрутов
Из папки Debug выполнить следующую команду в командной строке:
```
transport_catalogue.exe < make_base_input.json > make_base_output.json make_base
``` 
`make_base_input.json` - файл, содержащий список остановок и автобусных маршрутов. Примеры таких файлов можно посмотреть в папке "examples".

В результате выполнения команды в папке Debug должен появиться файл `transport_catalogue.db`, содержащий базу остановок и автобусных маршрутов.
### Работа с приложением
После того, как будет сформирована база автобусных маршрутов и остановок, можно начинать работу с приложением. Для этого нужно выполнить следующую команду:
```
transport_catalogue.exe < process_requests_input.json > process_requests_output.json process_requests
```
Файл `process_requests_input.json` должен содержать запрос к приложению, в файл `process_requests_output.json` будет выведен результат работы приложения.
Примеры файлов с входными и выходными данными для приложения можно посмотреть в папке "examples".
### Типы запросов
* Bus - получить маршрут автобуса с указанным номером.
* Stop - получить список автобусов, которые останавливаются на данной остановке.
* Route - построить оптимальный автобусный маршрут от остановки `from` до остановки `to`. 
* Map - отрисовать карту маршрутов и остановок в формате SVG.
## Стек технологий
С++17, CMake, Protobuf, собственные библиотеки для работы с форматами JSON и SVG.
