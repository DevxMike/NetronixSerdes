# NetronixSerdes

Lightweight frame serializer/deserializer for embedded systems.

## Frame format

[ADDR][LEN][CMD][PAYLOAD...][CRC_H][CRC_L]

## Features

- CRC16-CCITT
- Frame validation
- Zero dynamic allocation

## Use cases 

- MW-R7B / MW-R7G
- MW-R4B / MW-R4G
- MW-R8B / MW-R8G
- Other Netronix devices
- Literally any other embedded systems communication

## Unit tests build
- Execute **build_test.sh** script
- Change working directory to **build**
- Run **frame_ut**