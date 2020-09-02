#include "govee.h"
#include "esphome/core/log.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace govee {

static const char *TAG = "govee";

void Govee::dump_config() {
  ESP_LOGCONFIG(TAG, "Govee");
  LOG_SENSOR("  ", "Temperature", this->temperature_);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
  LOG_SENSOR("  ", "Battery Level", this->battery_level_);
}

bool Govee::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (device.address_uint64() != this->address_) {
    ESP_LOGVV(TAG, "parse_device(): unknown MAC address.");
    return false;
  }
  ESP_LOGVV(TAG, "parse_device(): MAC address %s found.", device.address_str().c_str());

  for (const auto& manufacturer_data : device.get_manufacturer_datas()) {
    auto& data = manufacturer_data.data;

    // This is built based on
    // https://github.com/Thrilleratplay/GoveeWatcher and
    // https://github.com/Home-Is-Where-You-Hang-Your-Hack/sensor.goveetemp_bt_hci
    // but also changed slightly so the temperature is first divided by 1000 in
    // integer, than divided by 10 in float. This makes sure humidity data will
    // not leak into temperature data.
    int32_t encoded_temp_humid = data[1] << 16 | data[2] << 8 | data[3];
    float temperature_in_celsius = static_cast<float>(encoded_temp_humid / 1000) / 10;
    float humidity_in_percent = static_cast<float>(encoded_temp_humid % 1000) / 10;
    float battery_in_percent = data[4];

    if (temperature_ != nullptr) {
      temperature_->publish_state(temperature_in_celsius);
    }
    if (humidity_ != nullptr) {
      humidity_->publish_state(humidity_in_percent);
    }
    if (battery_level_ != nullptr) {
      battery_level_->publish_state(battery_in_percent);
    }

    return true;
  }

  return false;
}

}  // namespace govee
}  // namespace esphome

#endif
