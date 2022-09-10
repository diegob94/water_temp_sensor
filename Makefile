
#curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh

#arduino-cli core update-index
#arduino-cli core install SparkFun:avr
#arduino-cli core install esp32:esp32

#arduino-cli lib install SerialCommands
#arduino-cli lib install --git-url https://github.com/Seeed-Studio/Grove_LoRa_433MHz_and_915MHz_RF.git
#sed -i '/<util\/atomic.h>/ s/<util\/atomic.h>/<RHutil\/atomic.h>/' ~/Arduino/libraries/Grove_LoRa_433MHz_and_915MHz_RF/RadioHead.h

.PHONY: all
all:
	@echo nothing to do

.PHONY: client
client:
	arduino-cli compile --fqbn SparkFun:avr:promicro client
	arduino-cli upload -p /dev/ttyUSB1 --fqbn SparkFun:avr:promicro client -P buspirate

.PHONY: server
server:
	arduino-cli compile --fqbn esp32:esp32:nodemcu-32s server
	arduino-cli upload --fqbn esp32:esp32:nodemcu-32s --port=/dev/ttyUSB0 server

#arduino-cli monitor --port=/dev/ttyUSB0
