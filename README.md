# scdtr
Distributed Real Time Control Systems

This repository contains the course project and report.
It consists of an automated lighting control system, with a distributed approach to the global optimisation problem.
It was tested using 2 Arduino boards for for system nodes, each with an LED and LDR as actuator and sensor respectively.
The system was enclosed in a shoe box, coated with reflective paper on the inside so as to reflect and accentuate inter-node coupling.

![System image][system]

A TCP/IP server was also developed in order to remotely control and monitor the system, as well as calculate performance heuristics.
The server itself was tested on a Raspberry Pi 3B.
In order for it to extract inforamtion from the syste, it needs an I2C connection to the Arduinos.
One can be simulated by writing to the monitored FIFO.
Specifics are further discussed in the report.

[system]: images/system.jpg "What amazing cable management!"
