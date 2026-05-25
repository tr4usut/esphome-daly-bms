"""
PATCH: components/daly_bms_ble/switch/__init__.py

Thay đổi chính:
- Thêm restore_mode mặc định DISABLED (không restore từ flash,
  vì state phải đến từ BMS, không phải từ bộ nhớ ESP)
- Đảm bảo switch được đăng ký với parent component để parent
  có thể gọi publish_state() khi có dữ liệu từ BMS.

Dựa trên pattern của patagonaa/esphome-daly-hkms-bms.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
    CONF_ID,
    ENTITY_CATEGORY_CONFIG,
    ICON_POWER,
)
from .. import daly_bms_ble_ns, DalyBmsBle, CONF_DALY_BMS_BLE_ID

DEPENDENCIES = ["daly_bms_ble"]

DalyBmsBleChargingSwitch = daly_bms_ble_ns.class_(
    "DalyBmsBleChargingSwitch", switch.Switch
)
DalyBmsBledischargingSwitch = daly_bms_ble_ns.class_(
    "DalyBmsBledischargingSwitch", switch.Switch
)

CONF_CHARGING = "charging"
CONF_DISCHARGING = "discharging"

# ============================================================
# QUAN TRỌNG: restore_mode = DISABLED
#
# Không dùng RESTORE_DEFAULT_OFF hay RESTORE_DEFAULT_ON.
# Switch state phải đến từ BMS (qua polling), không phải
# từ flash storage của ESP. Nếu restore từ flash, switch sẽ
# hiển thị sai ngay lúc khởi động trước khi nhận data từ BMS.
# ============================================================
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_DALY_BMS_BLE_ID): cv.use_id(DalyBmsBle),
        cv.Optional(CONF_CHARGING): switch.switch_schema(
            DalyBmsBleChargingSwitch,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_POWER,
            # Không restore: state đến từ BMS polling
            default_restore_mode=switch.SwitchRestoreMode.DISABLED,
        ),
        cv.Optional(CONF_DISCHARGING): switch.switch_schema(
            DalyBmsBleischargingSwitch,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_POWER,
            default_restore_mode=switch.SwitchRestoreMode.DISABLED,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_DALY_BMS_BLE_ID])

    if charging_config := config.get(CONF_CHARGING):
        sw = await switch.new_switch(charging_config)
        await cg.register_parented(sw, parent)
        # Đăng ký pointer trong parent để parent có thể publish_state()
        cg.add(parent.set_charging_switch(sw))

    if discharging_config := config.get(CONF_DISCHARGING):
        sw = await switch.new_switch(discharging_config)
        await cg.register_parented(sw, parent)
        cg.add(parent.set_discharging_switch(sw))
