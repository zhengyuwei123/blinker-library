#ifndef BlinkerSimplerESP32_H
#define BlinkerSimplerESP32_H

#include <Adapters/BlinkerArduinoWS.h>
#include <ESPmDNS.h>
#include <WiFi.h>

class BlinkerSimpleESP32_WS 
    : public BlinkerProtocol<BlinkerArduinoWS>
{
    typedef BlinkerProtocol<BlinkerArduinoWS> Base;

    public :
        BlinkerSimpleESP32_WS(BlinkerArduinoWS &transp)
            : Base(transp)
        {}

#if defined(BLINKER_ESP_SMARTCONFIG)
        void begin() {
            Base::begin();
            smartconfig();
            BLINKER_LOG1("ESP32_WiFi Initialled...");
        }
#endif
    
        void begin(const char* _ssid,
                    const char* _pswd)
        {
            Base::begin();
            connectWiFi(_ssid, _pswd);
            BLINKER_LOG1("ESP32_WiFi Initialled...");
        }
    
    private :
#if defined(BLINKER_ESP_SMARTCONFIG)
        void smartconfig() {
            WiFi.mode(WIFI_AP_STA);
            WiFi.beginSmartConfig();
            
            BLINKER_LOG1("Waiting for SmartConfig.");
            while (!WiFi.smartConfigDone()) {
                delay(500);
            }

            BLINKER_LOG1("SmartConfig received.");
            
            BLINKER_LOG1("Waiting for WiFi");
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
            }

            BLINKER_LOG1("WiFi Connected.");

            BLINKER_LOG1("IP Address: ");
            BLINKER_LOG1(WiFi.localIP());

            mDNSInit();
        }
#endif

        void mDNSInit()
        {
            if (!MDNS.begin(macDeviceName().c_str())) {
                while(1) {
                    ::delay(100);
                }
            }

            BLINKER_LOG1(BLINKER_F("mDNS responder started"));
            
            MDNS.addService("DiyArduino", "tcp", WS_SERVERPORT);
            MDNS.addServiceTxt("DiyArduino", "tcp", "deviceName", macDeviceName());

            this->conn.begin(macDeviceName());
        }

        void connectWiFi(const char* _ssid, const char* _pswd)
        {
            uint32_t connectTime = millis();

            BLINKER_LOG2(BLINKER_F("Connecting to "), _ssid);
            WiFi.mode(WIFI_STA);
            if (_pswd && strlen(_pswd)) {
                WiFi.begin(_ssid, _pswd);
            }
            else {
                WiFi.begin(_ssid);
            }

            while (WiFi.status() != WL_CONNECTED) {
                ::delay(50);

                if (millis() - connectTime > BLINKER_CONNECT_TIMEOUT_MS && WiFi.status() != WL_CONNECTED) {
                    connectTime = millis();
                    BLINKER_LOG1(("WiFi connect timeout, please check ssid and pswd!"));
                    BLINKER_LOG1(("Retring WiFi connect again!"));
                }
            }
            BLINKER_LOG1(BLINKER_F("Connected"));

            IPAddress myip = WiFi.localIP();
            BLINKER_LOG2(BLINKER_F("Your IP is: "), myip);

            mDNSInit();
        }
};

static BlinkerArduinoWS  _blinkerTransport;
BlinkerSimpleESP32_WS Blinker(_blinkerTransport);

#endif
