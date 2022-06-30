#include "pcf85063a.h"
#include "esphome/core/log.h"

// Datasheet:
// - https://www.nxp.com/docs/en/data-sheet/PCF85063A.pdf

namespace esphome {
namespace pcf85063a {

static const char *const TAG = "pcf85063a";

void PCF85063AComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up pcf85063a...");
  if (!this->read_rtc_()) {
    this->mark_failed();
  }
}

void PCF85063AComponent::update() { this->read_time(); }

void PCF85063AComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "PCF85063A:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with PCF85063A failed!");
  }
  ESP_LOGCONFIG(TAG, "  Timezone: '%s'", this->timezone_.c_str());
}

float PCF85063AComponent::get_setup_priority() const { return setup_priority::DATA; }

void PCF85063AComponent::read_time() {
  if (!this->read_rtc_()) {
    return;
  }
  if (pcf85063a_.reg.ch) {
    ESP_LOGW(TAG, "RTC halted, not syncing to system clock.");
    return;
  }
  time::ESPTime rtc_time{.second = uint8_t(pcf85063a_.reg.second + 10 * pcf85063a_.reg.second_10),
                         .minute = uint8_t(pcf85063a_.reg.minute + 10u * pcf85063a_.reg.minute_10),
                         .hour = uint8_t(pcf85063a_.reg.hour + 10u * pcf85063a_.reg.hour_10),
                         .day_of_week = uint8_t(pcf85063a_.reg.weekday),
                         .day_of_month = uint8_t(pcf85063a_.reg.day + 10u * pcf85063a_.reg.day_10),
                         .day_of_year = 1,  // ignored by recalc_timestamp_utc(false)
                         .month = uint8_t(pcf85063a_.reg.month + 10u * pcf85063a_.reg.month_10),
                         .year = uint16_t(pcf85063a_.reg.year + 10u * pcf85063a_.reg.year_10 + 2000)};
  rtc_time.recalc_timestamp_utc(false);
  if (!rtc_time.is_valid()) {
    ESP_LOGE(TAG, "Invalid RTC time, not syncing to system clock.");
    return;
  }
  time::RealTimeClock::synchronize_epoch_(rtc_time.timestamp);
}

void PCF85063AComponent::write_time() {
  auto now = time::RealTimeClock::utcnow();
  if (!now.is_valid()) {
    ESP_LOGE(TAG, "Invalid system time, not syncing to RTC.");
    return;
  }
  pcf85063a_.reg.year = (now.year - 2000) % 10;
  pcf85063a_.reg.year_10 = (now.year - 2000) / 10 % 10;
  pcf85063a_.reg.month = now.month % 10;
  pcf85063a_.reg.month_10 = now.month / 10;
  pcf85063a_.reg.day = now.day_of_month % 10;
  pcf85063a_.reg.day_10 = now.day_of_month / 10;
  pcf85063a_.reg.weekday = now.day_of_week;
  pcf85063a_.reg.hour = now.hour % 10;
  pcf85063a_.reg.hour_10 = now.hour / 10;
  pcf85063a_.reg.minute = now.minute % 10;
  pcf85063a_.reg.minute_10 = now.minute / 10;
  pcf85063a_.reg.second = now.second % 10;
  pcf85063a_.reg.second_10 = now.second / 10;
  pcf85063a_.reg.ch = false;

  this->write_rtc_();
}

bool PCF85063AComponent::read_rtc_() {
  if (!this->read_bytes(0, this->pcf85063a_.raw, sizeof(this->pcf85063a_.raw))) {
    ESP_LOGE(TAG, "Can't read I2C data.");
    return false;
  }
  ESP_LOGD(TAG, "Read  %0u%0u:%0u%0u:%0u%0u 20%0u%0u-%0u%0u-%0u%0u  CH:%s RS:%0u SQWE:%s OUT:%s", pcf85063a_.reg.hour_10,
           pcf85063a_.reg.hour, pcf85063a_.reg.minute_10, pcf85063a_.reg.minute, pcf85063a_.reg.second_10, pcf85063a_.reg.second,
           pcf85063a_.reg.year_10, pcf85063a_.reg.year, pcf85063a_.reg.month_10, pcf85063a_.reg.month, pcf85063a_.reg.day_10,
           pcf85063a_.reg.day, ONOFF(pcf85063a_.reg.ch), pcf85063a_.reg.rs, ONOFF(pcf85063a_.reg.sqwe), ONOFF(pcf85063a_.reg.out));

  return true;
}

bool PCF85063AComponent::write_rtc_() {
  if (!this->write_bytes(0, this->pcf85063a_.raw, sizeof(this->pcf85063a_.raw))) {
    ESP_LOGE(TAG, "Can't write I2C data.");
    return false;
  }
  ESP_LOGD(TAG, "Write %0u%0u:%0u%0u:%0u%0u 20%0u%0u-%0u%0u-%0u%0u  CH:%s RS:%0u SQWE:%s OUT:%s", pcf85063a_.reg.hour_10,
           pcf85063a_.reg.hour, pcf85063a_.reg.minute_10, pcf85063a_.reg.minute, pcf85063a_.reg.second_10, pcf85063a_.reg.second,
           pcf85063a_.reg.year_10, pcf85063a_.reg.year, pcf85063a_.reg.month_10, pcf85063a_.reg.month, pcf85063a_.reg.day_10,
           pcf85063a_.reg.day, ONOFF(pcf85063a_.reg.ch), pcf85063a_.reg.rs, ONOFF(pcf85063a_.reg.sqwe), ONOFF(pcf85063a_.reg.out));
  return true;
}
}  // namespace pcf85063a
}  // namespace esphome
