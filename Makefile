
#curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh

#sed -i '/<util\/atomic.h>/ s/<util\/atomic.h>/<RHutil\/atomic.h>/' ~/Arduino/libraries/Grove_LoRa_433MHz_and_915MHz_RF/RadioHead.h

CLIENT_PORT=/dev/ttyUSB0
SERVER_PORT=/dev/ttyUSB1

.PHONY: all
all:
	@echo nothing to do

.PHONY: config
config:
	arduino-cli config init
	arduino-cli config set board_manager.additional_urls "https://raw.githubusercontent.com/sparkfun/Arduino_Boards/main/IDE_Board_Manager/package_sparkfun_index.json"
	arduino-cli config add board_manager.additional_urls "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"
	arduino-cli core update-index
	arduino-cli core install SparkFun:avr
	arduino-cli core install esp32:esp32
	arduino-cli lib install SerialCommands
	arduino-cli lib install OneWire
	arduino-cli lib install --git-url https://github.com/Seeed-Studio/Grove_LoRa_433MHz_and_915MHz_RF.git
	arduino-cli board attach -b SparkFun:avr:promicro:cpu=16MHzatmega32U4 client
	arduino-cli board attach -b esp32:esp32:nodemcu-32s server

.PHONY: client
client:
	arduino-cli compile client
	arduino-cli upload -p $(CLIENT_PORT) -P buspirate client

.PHONY: server
server:
	arduino-cli compile server
	arduino-cli upload -p $(SERVER_PORT) server

#arduino-cli monitor --port=/dev/ttyUSB0
