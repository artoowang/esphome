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
  ESP_LOGE(TAG, "parse_device(): MAC address %s found.", device.address_str().c_str()); // TODO

  bool success = false;
  for (const auto& manufacturer_data : device.get_manufacturer_datas()) {
    auto& data = manufacturer_data.data;
    ESP_LOGE(TAG, "Get manufacturer_data: data.size: %d", data.size());  // TODO
    std::string str;
    for (int i = 0; i < data.size(); ++i) {
      char buf[10];
      sprintf(buf, "%02x ", data[i]);
      str += buf;
    }
    ESP_LOGE(TAG, "data: %s", str.c_str());

    int32_t encoded_temp_humid = data[1] << 16 | data[2] << 8 | data[3];
    float temperature_in_celsius = static_cast<float>(encoded_temp_humid / 1000) / 10;
    float humidity_in_percent = static_cast<float>(encoded_temp_humid % 1000) / 10;
    float battery_in_percent = data[4];
    ESP_LOGE(TAG, "%.1f %.1f %.0f", temperature_in_celsius, humidity_in_percent, battery_in_percent);

    if (temperature_ != nullptr) {
      temperature_->publish_state(temperature_in_celsius);
    }
    if (humidity_ != nullptr) {
      humidity_->publish_state(humidity_in_percent);
    }
    if (battery_level_ != nullptr) {
      battery_level_->publish_state(battery_in_percent);
    }

    success = true;
  }
  //for (auto &service_data : device.get_service_datas()) {
  //  ESP_LOGE(TAG, "Get service_data.");  // TODO
  //  auto res = xiaomi_ble::parse_xiaomi_header(service_data);
  //  if (!res.has_value()) {
  //    ESP_LOGE(TAG, "parse_xiaomi_header failed.");
  //    continue;
  //  }
  //  if (res->is_duplicate) {
  //    continue;
  //  }
  //  if (res->has_encryption) {
  //    ESP_LOGVV(TAG, "parse_device(): payload decryption is currently not supported on this device.");
  //    continue;
  //  }
  //  if (!(xiaomi_ble::parse_xiaomi_message(service_data.data, *res))) {
  //    continue;
  //  }
  //  if (!(xiaomi_ble::report_xiaomi_results(res, device.address_str()))) {
  //    continue;
  //  }
  //  if (res->temperature.has_value() && this->temperature_ != nullptr)
  //    this->temperature_->publish_state(*res->temperature);
  //  if (res->humidity.has_value() && this->humidity_ != nullptr)
  //    this->humidity_->publish_state(*res->humidity);
  //  if (res->battery_level.has_value() && this->battery_level_ != nullptr)
  //    this->battery_level_->publish_state(*res->battery_level);
  //  success = true;
  //}

  ESP_LOGE(TAG, "Ended: no more service_data. success = %d", success);  // TODO

  if (!success) {
    return false;
  }

  return true;
}

}  // namespace govee
}  // namespace esphome

#endif
