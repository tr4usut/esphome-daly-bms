#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../daly_bms_ble.h"

namespace esphome {
namespace daly_bms_ble {

// ============================================================
// PATCH: Non-optimistic switches cho charging & discharging
//
// Vấn đề gốc (syssi):
//   write_state() gửi lệnh BLE + gọi publish_state(state) ngay lập tức.
//   → Switch hiển thị lệnh đã gửi, KHÔNG phản chiếu trạng thái thực tế.
//   → Nếu BMS từ chối (bảo vệ quá áp/nhiệt/...) switch vẫn hiện ON.
//
// Giải pháp (học từ patagonaa):
//   1. write_state() chỉ gửi lệnh, KHÔNG publish_state().
//   2. DalyBmsBle::parse_charging_status() gọi publish_state(actual_bit)
//      sau mỗi lần nhận dữ liệu từ BMS → switch luôn = thực tế hardware.
// ============================================================

class DalyBmsBleChargingSwitch : public switch_::Switch, public Parented<DalyBmsBle> {
 public:
  // Không optimistic: state chỉ thay đổi khi BMS trả về dữ liệu thực
  bool is_optimistic() override { return false; }

 protected:
  void write_state(bool state) override {
    // Chỉ gửi lệnh, KHÔNG gọi publish_state() ở đây.
    // Trạng thái sẽ được cập nhật bởi vòng polling khi BMS xác nhận.
    this->parent_->set_charging(state);
    // Tùy chọn: log để debug
    ESP_LOGD("daly_bms_ble.switch", "Charging switch command sent: %s (awaiting BMS confirmation)", state ? "ON" : "OFF");
  }
};

class DalyBmsBledischargingSwitch : public switch_::Switch, public Parented<DalyBmsBle> {
 public:
  bool is_optimistic() override { return false; }

 protected:
  void write_state(bool state) override {
    this->parent_->set_discharging(state);
    ESP_LOGD("daly_bms_ble.switch", "Discharging switch command sent: %s (awaiting BMS confirmation)", state ? "ON" : "OFF");
  }
};

}  // namespace daly_bms_ble
}  // namespace esphome
