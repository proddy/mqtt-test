

#include <WiFi.h>
#include <espMqttClient.h>

// with TLS
// #define MQTT_PORT 8883
// #define MQTT_HOST "192.168.1.105"
// #define USE_TLS 1

#define MQTT_ROOT_CA_CERT                                                \
    "-----BEGIN CERTIFICATE-----\n"                                      \
    "MIIDmzCCAoOgAwIBAgIUKO2zIxO5Ych3Ipc5pwfqwIBBYj0wDQYJKoZIhvcNAQEL\n" \
    "BQAwXTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n" \
    "GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDEWMBQGA1UEAwwNMTkyLjE2OC4xLjEw\n" \
    "NTAeFw0yMzA3MDkxMjEwMjFaFw0zMzA3MDYxMjEwMjFaMF0xCzAJBgNVBAYTAkFV\n" \
    "MRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRz\n" \
    "IFB0eSBMdGQxFjAUBgNVBAMMDTE5Mi4xNjguMS4xMDUwggEiMA0GCSqGSIb3DQEB\n" \
    "AQUAA4IBDwAwggEKAoIBAQCx37YRSShBS2fRmof1SI3SyBUljkFnL5si/z25dZER\n" \
    "874FG4Nr+pr6juMF1jRDEMstTs3LoBowVCjw5EI1Lm9ZZfZlzW68R+nNR1EQ3zYA\n" \
    "spmBW+mSX9ZFm/S0uLBWhUm2ujMZo2cx1AxDSIzWta0TjJGpu6+gD7Vdx8/HFgba\n" \
    "WmSKDCnovCqtYHoj4M8WhuY/V+5xU9Gixx1v30JzjhOeAwTvI+AH1flPsVc0i0+/\n" \
    "Toi/ckfACbF3i5WEm0I0PQGS3SshBd2zPdI/0eKwCeXPocEI6vmocn9DebE//glK\n" \
    "ABdzZRCwTEH+jOoD77V5GYclfdYFV8ETcACMI50fD1lpAgMBAAGjUzBRMB0GA1Ud\n" \
    "DgQWBBSUnD5TsFTn9hNwQuhGS2LS2hLAsjAfBgNVHSMEGDAWgBSUnD5TsFTn9hNw\n" \
    "QuhGS2LS2hLAsjAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQBi\n" \
    "2kX0Y5FPUystwYk9GdRkkczfl2/BHzeUX75w7cr2GaGgqdEG/BlIvztGGioDCr7K\n" \
    "+Fs61w5N7WfkEKztdF+XkRjc2MKndfcUd7TtgJiTt7bP7+vLS3fNGVHk5tnKjfau\n" \
    "R6v9Ct5nN8SG8AF9bv39f/XOoCa/vV7ac3nBsPtkRWygaPZsq7EbVkIAW7c2a56t\n" \
    "1c9HFIimJWm1WItUkMYr1hXTm6kXPs1LHdPeUKx8cSDWmWZha5FoY8D/yY0po3a/\n" \
    "jsaCY7XQv8Dx7tKhIeGJ8GfPURbr4ugkEdzN2503vXR0OuiOzCqVmH12uHW2C3Jk\n" \
    "iLJ5tVFFFOJCmV/z/TJT\n"                                             \
    "-----END CERTIFICATE-----\n";

// without TLS
#define MQTT_PORT 1883
#define MQTT_HOST "192.168.1.212"
#define USE_TLS 0

// change these
#define WIFI_SSID "xxx"
#define WIFI_PASSWORD "xxx"
#define MQTT_USER "xxx"
#define MQTT_PASS "xxx"

MqttClient *mqttClient = nullptr;

static TaskHandle_t taskHandle;
bool reconnectMqtt = false;
uint32_t lastReconnect = 0;

void connectToWiFi()
{
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    if (!mqttClient->connect())
    {
        reconnectMqtt = true;
        lastReconnect = millis();
        Serial.println("Connecting failed.");
    }
    else
    {
        reconnectMqtt = false;
    }
}

void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        break;
    default:
        break;
    }
}

void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);

    uint16_t packetIdSub0 = mqttClient->subscribe("foo/bar/0", 0);
    Serial.print("Subscribing at QoS 0, packetId: ");
    Serial.println(packetIdSub0);

    uint16_t packetIdPub0 = mqttClient->publish("foo/bar/0", 0, false, "test");
    Serial.println("Publishing at QoS 0, packetId: ");
    Serial.println(packetIdPub0);
}

void onMqttDisconnect(espMqttClientTypes::DisconnectReason reason)
{
    Serial.printf("Disconnected from MQTT: %u.\n", static_cast<uint8_t>(reason));

    if (WiFi.isConnected())
    {
        reconnectMqtt = true;
        lastReconnect = millis();
    }
}

void onMqttSubscribe(uint16_t packetId, const espMqttClientTypes::SubscribeReturncode *codes, size_t len)
{
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    for (size_t i = 0; i < len; ++i)
    {
        Serial.print("  qos: ");
        Serial.println(static_cast<uint8_t>(codes[i]));
    }
}

void onMqttUnsubscribe(uint16_t packetId)
{
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void onMqttMessage(const espMqttClientTypes::MessageProperties &properties, const char *topic, const uint8_t *payload, size_t len, size_t index, size_t total)
{
    (void)payload;
    Serial.println("Publish received.");
    Serial.print("  topic: ");
    Serial.println(topic);
    Serial.print("  qos: ");
    Serial.println(properties.qos);
    Serial.print("  dup: ");
    Serial.println(properties.dup);
    Serial.print("  retain: ");
    Serial.println(properties.retain);
    Serial.print("  len: ");
    Serial.println(len);
    Serial.print("  index: ");
    Serial.println(index);
    Serial.print("  total: ");
    Serial.println(total);
}

void onMqttPublish(uint16_t packetId)
{
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void networkingTask()
{
    for (;;)
    {
        mqttClient->loop();
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(true);
    WiFi.onEvent(WiFiEvent);

    if (USE_TLS)
    {
        mqttClient = static_cast<MqttClient *>(new espMqttClientSecure(espMqttClientTypes::UseInternalTask::NO));

        const char rootCA[] = MQTT_ROOT_CA_CERT;
        static_cast<espMqttClientSecure *>(mqttClient)->setCACert(rootCA);

        static_cast<espMqttClientSecure *>(mqttClient)->onConnect(onMqttConnect);
        static_cast<espMqttClientSecure *>(mqttClient)->onDisconnect(onMqttDisconnect);
        static_cast<espMqttClientSecure *>(mqttClient)->onConnect(onMqttConnect);
        static_cast<espMqttClientSecure *>(mqttClient)->onDisconnect(onMqttDisconnect);
        static_cast<espMqttClientSecure *>(mqttClient)->onSubscribe(onMqttSubscribe);
        static_cast<espMqttClientSecure *>(mqttClient)->onUnsubscribe(onMqttUnsubscribe);
        static_cast<espMqttClientSecure *>(mqttClient)->onMessage(onMqttMessage);
        static_cast<espMqttClientSecure *>(mqttClient)->onPublish(onMqttPublish);
        static_cast<espMqttClientSecure *>(mqttClient)->setServer(MQTT_HOST, MQTT_PORT);
        static_cast<espMqttClientSecure *>(mqttClient)->setCleanSession(true);
    }
    else
    {
        mqttClient = static_cast<MqttClient *>(new espMqttClient(espMqttClientTypes::UseInternalTask::NO));

        static_cast<espMqttClient *>(mqttClient)->setCredentials(MQTT_USER, MQTT_PASS);
        static_cast<espMqttClient *>(mqttClient)->onConnect(onMqttConnect);
        static_cast<espMqttClient *>(mqttClient)->onDisconnect(onMqttDisconnect);
        static_cast<espMqttClient *>(mqttClient)->onConnect(onMqttConnect);
        static_cast<espMqttClient *>(mqttClient)->onDisconnect(onMqttDisconnect);
        static_cast<espMqttClient *>(mqttClient)->onSubscribe(onMqttSubscribe);
        static_cast<espMqttClient *>(mqttClient)->onUnsubscribe(onMqttUnsubscribe);
        static_cast<espMqttClient *>(mqttClient)->onMessage(onMqttMessage);
        static_cast<espMqttClient *>(mqttClient)->onPublish(onMqttPublish);
        static_cast<espMqttClient *>(mqttClient)->setServer(MQTT_HOST, MQTT_PORT);
        static_cast<espMqttClient *>(mqttClient)->setCleanSession(true);
    }

    xTaskCreatePinnedToCore((TaskFunction_t)networkingTask, "mqttclienttask", 5120, nullptr, 1, &taskHandle, 0);

    connectToWiFi();
}

void loop()
{
    static uint32_t currentMillis = millis();

    if (reconnectMqtt && currentMillis - lastReconnect > 5000)
    {
        connectToMqtt();
    }

    static uint32_t lastMillis = 0;
    if (currentMillis - lastMillis > 5000)
    {
        lastMillis = currentMillis;
        Serial.printf("heap: %u\n", ESP.getFreeHeap());
    }

    static uint32_t millisDisconnect = 0;
    if (currentMillis - millisDisconnect > 60000)
    {
        millisDisconnect = currentMillis;
        mqttClient->disconnect();
    }
}