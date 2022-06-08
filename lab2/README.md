# Лабораторная работа 2

**Название:** "Разработка драйверов блочных устройств"

**Цель работы:** получить знания и навыки разработки драйверов блочных устройств для операционной системы Linux.

---

## Описание функциональности драйвера

* Драйвер создает виртуальный жесткий диск на 50 Мб в RAM со следующей таблицей разделов:

| Файл.система  | Размер        | Вид раздела |
| ------------- | ------------- | ----------- |
| /dev/mydisk1  | 10M           | первичный   |
| /dev/mydisk2  | 20M           | первичный   |
| /dev/mydisk3  | 20M           | расширенный |

```
NAME        MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
vramdisk    252:0    0    50M  0 disk 
├─vramdisk1 252:1    0    10M  0 part /mnt/vramdisk1
├─vramdisk2 252:2    0    20M  0 part 
├─vramdisk3 252:3    0     1K  0 part 
├─vramdisk5 252:5    0    10M  0 part 
└─vramdisk6 252:6    0    10M  0 part 
```

## Инструкция по сборке

### Сборка

```
make # собрать модуль
make load # слинковать модуль с ядром
make vfat DPART=<путь до партиции> # форматирует раздел под ФС FAT 16 
```

### Разборка

```
# прекратить все операции с диском umount там и т.д.
make unload # выгрузить модуль
make clean # (опционально) чтобы не держать собранные пакеты
```

## Инструкция пользователя

1. Собрать драйвер и загрузить в ядро
2. Создать файловую систему для нужных разделов ( `sudo mkfs.vfat <нужный раздел>` )
3. Смонтировать раздел в папку ( `sudo mount -t vfat <раздел> <в нужный каталог>` )
4. Всё. Можно свободно пользоваться диском.

## Примеры использования

```
make vfat DPART=/dev/vramdisk1
sudo mkdir /mnt/vramdisk1
sudo mount -t vfat /dev/vramdisk1 /mnt/vramdisk1
echo "Abacaba!" > test
sudo cp test /mnt/vramdisk1/hello
rm -f test
ls -l /mnt/vramdisk1
cat /mnt/vramdisk1/test
sudo umount /mnt/vramdisk1

### Измерение скорости передачи данных

`sudo dd if=<source part.> of=<destination part.> count=<number of input blocks>`

#### Передача между разделами виртуального жесткого диска

Между первичными
```
sudo dd if=/dev/vramdisk1 of=/dev/vramdisk2 count=10k
10240+0 records in
10240+0 records out
5242880 bytes (5,2 MB, 5,0 MiB) copied, 0,0365509 s, 143 MB/s
```

Между первичным и расширенным
```
sudo dd if=/dev/vramdisk1 of=/dev/vramdisk5 count=10k
10240+0 records in
10240+0 records out
5242880 bytes (5,2 MB, 5,0 MiB) copied, 0,0300502 s, 174 MB/s
```

Между логическими разделами расширенного раздела
```
sudo dd if=/dev/vramdisk5 of=/dev/vramdisk6 count=10k
10240+0 records in
10240+0 records out
5242880 bytes (5,2 MB, 5,0 MiB) copied, 0,0205062 s, 256 MB/s
```

#### Передача между виртуальным и реальным жестким диском

```
sudo dd if=/dev/sda1 of=/dev/vramdisk1 count=10k
10240+0 records in
10240+0 records out
5242880 bytes (5,2 MB, 5,0 MiB) copied, 0,0608588 s, 86,1 MB/s
```
