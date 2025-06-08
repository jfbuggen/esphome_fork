#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include "esphome/components/valve/valve.h"

namespace esphome {
namespace hbridge {

enum ValveState : uint8_t {
  VALVE_STATE_CLOSED = 0,
  VALVE_STATE_OPEN = 1,
  VALVE_STATE_OPENING = 2,
  VALVE_STATE_CLOSING = 3,
  VALVE_STATE_UNKNOWN = 4,
};

class HBridgeValve : public valve::Valve, public Component {
 public:
  HBridgeValve();

  void set_pin_a(GPIOPin *pin) { this->pin_a_ = pin; }
  void set_pin_b(GPIOPin *pin) { this->pin_b_ = pin; }
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
  uint32_t pulse_length_{50};
  uint32_t wait_time_{0};

  bool timer_running_{false};
  bool desired_state_{false};
  ValveState valve_state_{VALVE_STATE_UNKNOWN};
  bool optimistic_{false};

  void control(const valve::ValveCall &call) override;
  valve::ValveTraits get_traits() override;
  void write_state(bool state) override;
  void timer_fn_();
};

}  // namespace hbridge
}  // namespace esphome
