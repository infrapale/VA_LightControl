# TK Light Terminal Specification
## Functions
* RFM69 434Mhz radio function, here called radio channel
* Sending relay control messages to relay units via radio channel
* Read contact information via radio channel
* Reading special design keyboard and generate one or many light control messages
* Maintain real time
* Broadcast real time
* Show status or menu information on a connected LCD display
* Receive light measurement from a specific sender
* Read @home key input or radio message
* Start or end the away program 
## Units
* Adafruit M0 & RFM69 433Mhz Feather, radio subunit is connected via SPI
* Adafruit RTC & SD feather wing
* Adafruit feather mother board for 2 (?) units
* Nokia 5110 LCD breakout board connected via SPI
## Controller Pin Map
* SCK (SPI) -> LCD CLK
* MOSI      -> LCD DIN
* MISO      <- LCD ---
* 5 -> LCD DC pin
* 6 -> LCD CS pin
* 9 -> LCD RST
* 10 -> LCD LIGHT via BC547 transistor driver
