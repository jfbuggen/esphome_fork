#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include "esphome/components/valve/valve.h"

namespace esphome {
namespace hbridge {

enum HbridgeState : unit8_t {
  HBRIDGE_IDLE = 0,
  HBRIDGE_PULSE = 1,
  HBRIDGE_WAIT = 2,
  HBRIDGE_SLEEP = 3,
  HBRIDGE_WAKEUP = 4,
};

enum ValveState : uint8_t {
  VALVE_STATE_CLOSED = 0,
  VALVE_STATE_OPEN = 1,
  VALVE_STATE_UNKNOWN = 2,
};

enum ValveCmd: uint8_t {
  VALVE_CMD_CLOSE = 0,
  VALVE_CMD_OPEN = 1,
  VALVE_CMD_TOGGLE = 2,
  VALVE_CMD_NONE = 3,
};

class HBridgeValve : public valve::Valve, public Component {
 public:
  HBridgeValve();

  void set_pin_a(GPIOPin *pin) { this->pin_a_ = pin; }
  void set_pin_b(GPIOPin *pin) { this->pin_b_ = pin; }
  void set_pin_sleep(GPIOPin *pin) { this->pin_sleep_ = pin; }
  void set_pulse_length(uint32_t pulse_length) { this->pulse_length_ = pulse_length; }
  void set_wait_time(uint32_t wait_time) { this->wait_time_ = wait_time; }

  void set_optimistic(bool optimistic) { this->optimistic_ = optimistic; }

  valve::ValveTraits get_traits() override { return this->traits_; }

  float get_setup_priority() const override;
  void setup() override;
  void dump_config() override;

 protected:
  GPIOPin *pin_a_{nullptr};
  GPIOPin *pin_b_{nullptr};
  GPIOPin *pin_sleep_{nullptr};
  uint32_t pulse_length_{50};
  uint32_t wait_time_{0};

  bool timer_running_{false};
  bool pulse_busy_{false};
  bool wait_busy_{false};
  ValveCmd valve_cmd_{VALVE_CMD_NONE};
  HbridgeState hbridge_state_{HBRIDGE_IDLE};
  ValveState valve_state_{VALVE_STATE_UNKNOWN};
  bool optimistic_{false};

  void control(const valve::ValveCall &call) override;
  valve::ValveTraits get_traits() override;
  void write_state(bool state) override;
  void timer_fn_();
};

}  // namespace hbridge
}  // namespace esphome
