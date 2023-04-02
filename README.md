# imuball
Control a Sphero Mini robot with the Bosch IMU on a Nicla Sense Me device via bluetooth.

## References 

Nicla Sense Me Board:
https://docs.arduino.cc/tutorials/nicla-sense-me/cheat-sheet

Communication to a Sphero Mini:
* https://github.com/liamhays/sphero-ble
* https://github.com/artificial-intelligence-class/spherov2.py
* https://github.com/igbopie/spherov2.js/


## Setup

Info from `scan.ino` example from `ArduinoBLE`
```
Bluetooth® Low Energy Central scan
Discovered a peripheral
-----------------------
Address: f3:a1:19:f0:59:35
Local Name: SM-5935
Service UUIDs: 00010001-574f-4f20-5370-6865726f2121 
RSSI: -59
```

Info from `peripheral_explorer.ino` example from `ArduinoBLE`
```
Service 00020001-574f-4f20-5370-6865726f2121
	Characteristic 00020003-574f-4f20-5370-6865726f2121, properties 0x4
	Characteristic 00020002-574f-4f20-5370-6865726f2121, properties 0x18
	Characteristic 00020004-574f-4f20-5370-6865726f2121, properties 0x2, value 0x00000F000C000202
	Characteristic 00020005-574f-4f20-5370-6865726f2121, properties 0xC
Service 180f
	Characteristic 2a19, properties 0x12, value 0x64
		Descriptor 2902, value 0x0000
Service 00010001-574f-4f20-5370-6865726f2121
	Characteristic 00010002-574f-4f20-5370-6865726f2121, properties 0x1C
	Characteristic 00010003-574f-4f20-5370-6865726f2121, properties 0x1C
		Descriptor 2902, value 0x0000
```

### Caveats
Necessary changes in the `ArduinoBLE` library:
In the file `ATT.cpp` of the ArduinoBLE library the `+1` needs to be deleted from the statement `service->characteristic(j + 1)` in line 1712.
