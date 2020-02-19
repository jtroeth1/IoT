# IoT

Description:
Android app is used to connect to Gateway via TCP to issue control commands. The gateway then relays these commands to the correct slave node.
The gateway features logging and SQL database. Slave send temperature LED and GPS data upon request. Gateway and slave uses the same software, separated by #IFDEF's.

Jora_client : C# Android TCP Control Application
Jora_server : Python TCP Server
Jora_Gateway : C++ Rpi Gateway node
Jora_slave : C++ Arduino Slave node

Hardware:
* Raspberry Pi (Ubuntu Mate) Gateway
* Arduino Slave
* Semtech SX1272 Comms board
* uBlox NEO-6M GPS
