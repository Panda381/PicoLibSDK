This folder contains sample programs for Raspberry Pico from original SDK, using UART console output.

To use UART console, connect UART-USB converter to pins GPIO0 (TX output from Pico)
and GPIO1 (RX input to Pico) and set COM port to 115200 Baud, 8 bits, no parity,
1 stop bit, no flow control.


How to connect USB-serial adaptor PL2303TA to Pico:

                           +---USB---+
white RxD <-- UART0_TX GP0 |.1    40.| VBUS
green TxD --> UART0_RX GP1 |.2    39.| VSYS --- red +5V
black GND ---          GND |.3    38.| GND
                       GP2 |.4    37.| 3V3_EN
                       GP3 |.5    36.| 3V3_OUT
