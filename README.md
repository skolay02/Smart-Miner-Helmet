# Smart-Miner-Helmet

Mining is one of the most essential yet hazardous industries. Miners work in extreme conditions, often exposed to toxic gases, high temperatures, humidity, poor ventilation, and low visibility. Despite technological advancements, underground mining continues to witness accidents from gas leaks, explosions, heat strokes, and structural failures. This highlights the urgent need for intelligent, wearable safety systems that can provide continuous environmental monitoring and real-time alerts.

The Smart Miner Helmet addresses this need by offering a compact, affordable, and intelligent safety device powered by the versatile ESP32 microcontroller. Designed for underground use, the helmet integrates a range of sensors and communication modules to monitor environmental hazards and alert both miners and supervisors instantly.

It includes MQ2, MQ9, and MQ135 gas sensors to detect flammable, toxic, and harmful gases like methane, carbon monoxide, and air pollutants. A DHT11 sensor tracks temperature and humidity levels to mitigate heat-related risks. All readings are displayed in real time on an OLED screen, keeping miners aware of their surroundings. Simultaneously, the data is stored on an SD card with timestamps for future analysis and safety reporting.

To ensure immediate response, buzzers and LEDs are triggered when any sensor reading exceeds safety limits, warning the miner of danger. The helmet’s Wi-Fi connectivity enables continuous data transmission to a web interface, allowing remote monitoring by safety personnel. This real-time access is crucial for prompt action during emergencies.

An emergency SOS button is also incorporated, which activates a buzzer and sends a distress alert to the web portal when pressed. Furthermore, the integration of an ESP32-CAM module allows for live video streaming from the miner’s location. This feature enhances situational awareness and is especially useful during rescue operations.

Overall, the Smart Miner Helmet is a practical and impactful solution that combines real-time monitoring, alert mechanisms, and visual surveillance to significantly enhance miner safety in hazardous underground environments.
