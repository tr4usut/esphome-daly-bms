#include "daly_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome::daly_bms_ble {

static const char *const TAG = "daly_bms_ble.switch";

static const uint8_t DALY_FUNCTION_WRITE = 0x06;

void DalySwitch::dump_config() { LOG_SWITCH("", "DalyBmsBle Switch", this); }
void DalySwitch::write_state(bool state) {
  // [PATCH] Non-optimistic: chỉ gửi lệnh xuống BMS, KHÔNG gọi publish_state() ở đây.
  // Trạng thái thực tế sẽ được cập nhật bởi decode_status_data_() trong vòng polling
  // tiếp theo (mỗi update_interval), đảm bảo switch luôn phản chiếu hardware thực tế.
  // Trước đây: publish_state(state) gọi ngay sau khi gửi lệnh (optimistic) khiến
  // switch hiển thị sai nếu BMS từ chối lệnh do bảo vệ (OVP, OTP, SCP...).
  if (!this->parent_->send_command(DALY_FUNCTION_WRITE, this->holding_register_, (uint16_t) state)) {
    ESP_LOGW(TAG, "Failed to send command for register 0x%04X, state=%s", this->holding_register_,
             state ? "ON" : "OFF");
  }
  // State sẽ tự cập nhật khi BMS trả về status frame tiếp theo.
}

}  // namespace esphome::daly_bms_ble
