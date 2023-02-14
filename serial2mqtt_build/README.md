All credit for the serial2mqtt tool goes to vortex314.
https://github.com/vortex314/serial2mqtt

Only change is in the config file.
mqtt connection set to 'tcp://localhost:1883'
idleTimeout set to '20000' milliseconds.
protocol set to 'jsonObject'.

deploy the pre-build versions from the Debug directory , 2 versions available : Linux 64bits Intel and Raspberry Pi ARM. The armv6l also runs on raspberry pi 3. Watch out for the arch command below.

wget https://github.com/TGit-Tech/Tode-RC-Firmware/tree/main/serial2mqtt_build/serial2mqtt.`arch`.zip
wget https://github.com/TGit-Tech/Tode-RC-Firmware/tree/main/serial2mqtt_build/serial2mqtt.json
unzip serial2mqtt.`arch`.zip
mv Debug/serial2mqtt.`arch` serial2mqtt
