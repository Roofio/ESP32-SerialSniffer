/*
   ESP32-SerialSniffer
   A real-time WiFi packet monitor that outputs directly in the Serial Monitor.

   Author: Roofio 
   Year:   2025
   GitHub: https://github.com/Roofio/ESP32-SerialSniffer

   License: MIT
   See LICENSE file in the repository for full details.

   Features:
   - Tabular output: No., Time, Source/Dest MAC, Length, Frame type, RSSI, Channel, SSID
   - Channel hopping (1-11)
*/
#include <Arduino.h>
#include "nvs_flash.h"
#include "esp_wifi.h"

// SETTINGS
#define BAUD_RATE       115200
#define CHANNEL_HOPPING true
#define MAX_CHANNEL     11   // Most countries channels are 1 to 11
#define HOP_INTERVAL    214  // ms

int currentChannel = 1;
unsigned long lastChannelChange = 0;

// Packet tracking
uint32_t packetCount = 0;
unsigned long firstPacketTime = 0;

void printMac(const uint8_t* mac) {
  if (!mac) {
    Serial.print("                ");  // 17 spaces alignment
    return;
  }
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// Bits 2-3: Type (Management, Control, Data, or Reserved)
// Bits 4-7: Subtype
const char* getFrameTypeName(uint8_t type, uint8_t subtype) {
  switch (type) {
    case 0x00: // Management
      switch (subtype) {
        case 0x08: return "Beacon";
        case 0x04: return "Probe Request";
        case 0x05: return "Probe Response";
        case 0x00: return "Association Request";
        case 0x0B: return "Authentication";
        case 0x0A: return "Disassociation";
        case 0x0C: return "Deauthentication";
        default:   return "Management";
      }
    case 0x01: // Control
      switch (subtype) {
        case 0x07: return "Block Ack";
        case 0x0D: return "ACK";
        case 0x09: return "RTS";
        case 0x0B: return "CTS";
        default:   return "Control";
      }
    case 0x02: return "Data";
    default:   return "Reserved";
  }
}

void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;

  if (ctrl.sig_len < 24) return;

  uint8_t* frame = pkt->payload;
  uint8_t fc = frame[0];
  uint8_t frame_type = (fc & 0x0C) >> 2;
  uint8_t frame_subtype = (fc & 0xF0) >> 4;

  packetCount++;
  unsigned long now = micros();

  if (packetCount == 1) {
    firstPacketTime = now;
    Serial.println();
    Serial.printf("%-6s %-10s %-18s %-18s %-9s %-7s %s\n",
                  "No.", "Time", "Source", "Destination", "Protocol", "Length", "Info");
    Serial.println(String(110, '-'));
  }

  float relTime = (now - firstPacketTime) / 1000000.0;

  uint8_t* dest = &frame[4];   // Addr1: Receiver
  uint8_t* src  = &frame[10];  // Addr2: Transmitter

  Serial.printf("%-6lu %-10.6f ", packetCount, relTime);
  printMac(src);
  Serial.print("  ");
  printMac(dest);
  Serial.print("  802.11   ");
  Serial.printf("%-6u ", ctrl.sig_len);

  // Info: Frame type
  Serial.printf("%s frame", getFrameTypeName(frame_type, frame_subtype));

  // RSSI and Channel
  Serial.printf(", RSSI=%d dBm, Ch=%d", ctrl.rssi, currentChannel);

  // SSID for Beacon, Probe Request, Probe Response
  if (frame_type == 0 && (frame_subtype == 0x08 || frame_subtype == 0x04 || frame_subtype == 0x05)) {
    uint8_t ssid_len = frame[37];
    uint8_t* ssid_ptr = &frame[38];

    Serial.print(", SSID=");
    if (ssid_len == 0) {
      Serial.print("<hidden>");
    } else {
      const uint8_t MAX_DISPLAY = 64;  // Safe upper limit for serial output
      uint8_t to_print = ssid_len < MAX_DISPLAY ? ssid_len : MAX_DISPLAY;

      for (uint8_t i = 0; i < to_print; i++) {
        char c = ssid_ptr[i];
        Serial.print(isprint(c) ? c : '.');
      }

      if (ssid_len > MAX_DISPLAY) {
        Serial.print("...");
      }
    }
  }

  Serial.println();
}

void setup() {
  Serial.begin(BAUD_RATE);
  delay(2000);

  esp_log_level_set("wifi", ESP_LOG_WARN);

  Serial.println("\n=== ESP32 WiFi Sniffer ===\n");

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
  ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&sniffer));
  ESP_ERROR_CHECK(esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE));
}

void loop() {
  if (CHANNEL_HOPPING) {
    unsigned long now = millis();
    if (now - lastChannelChange >= HOP_INTERVAL) {
      lastChannelChange = now;
      currentChannel = (currentChannel % MAX_CHANNEL) + 1;
      esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }
  }
}