#if defined(ESP32)
   #include <WiFiMulti.h>
   WiFiMulti wifiMulti;
   #define DEVICE "ESP32_THIRD"
 #elif defined(ESP8266)
   #include <ESP8266WiFiMulti.h>
   ESP8266WiFiMulti wifiMulti;
   #define DEVICE "ESP8266_THIRD"
 #endif
 
 #include <InfluxDbClient.h>
 #include <Adafruit_MPU6050.h>
 #include <Adafruit_Sensor.h>
 #include <Wire.h>
 #include <secrets.h>
 
 // InfluxDB client
 InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
 Point sensor("mpu6050_data");
 
 // MPU6050
 Adafruit_MPU6050 mpu;
 
 void setup() {
     Serial.begin(115200);
     Wire.begin();
     Wire.setClock(400000);
 
     WiFi.mode(WIFI_STA);
     wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
     Serial.print("Connecting to WiFi");
     while (wifiMulti.run() != WL_CONNECTED) {
         Serial.print(".");
         delay(100);
     }
     Serial.println("\nConnected to WiFi");
 
     timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
 
     if (client.validateConnection()) {
         Serial.print("Connected to InfluxDB: ");
         Serial.println(client.getServerUrl());
     } else {
         Serial.print("InfluxDB connection failed: ");
         Serial.println(client.getLastErrorMessage());
     }
 
     // Tags
     sensor.addTag("device", DEVICE);
     sensor.addTag("SSID", WiFi.SSID());
     sensor.addTag("sensor_id", "3");
 
     if (!mpu.begin(0x68)) {
         Serial.println("Failed to find MPU6050!");
         while (1) delay(10);
     }
 
     Serial.println("MPU6050 initialized!");
 }
 
 void loop() {
     sensor.clearFields();

     if (WiFi.status() != WL_CONNECTED) {
  Serial.println("WiFi disconnected! Reconnecting...");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("WiFi reconnected!");
}
 
     sensors_event_t a, g, temp;
     mpu.getEvent(&a, &g, &temp);
 
     sensor.addField("acc_x", a.acceleration.x);
     sensor.addField("acc_y", a.acceleration.y);
     sensor.addField("acc_z", a.acceleration.z);
     sensor.addField("gyro_x", g.gyro.x);
     sensor.addField("gyro_y", g.gyro.y);
     sensor.addField("gyro_z", g.gyro.z);
     sensor.addField("temperature", temp.temperature);
     sensor.addField("rssi", WiFi.RSSI());
 
     Serial.print("Writing to InfluxDB: ");
     Serial.println(sensor.toLineProtocol());
 
     if (!client.writePoint(sensor)) {
         Serial.print("InfluxDB write failed: ");
         Serial.println(client.getLastErrorMessage());
     }
 
     delay(100);
 }