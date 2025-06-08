#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include "esphome/components/valve/valve.h"

namespace esphome {
namespace hbridge {

class HBridgeValve : public valve::Valve, public Component {
 public:
  HBridgeValve();

  void set_pin_a(GPIOPin *pin) { this->pin_a_ = pin; }
  void set_pin_b(GPIOPin *pin) { this->pin_b_ = pin; }
  void set_pulse_length(uint32_t pulse_length) { this->pulse_length_ = pulse_length; }
  void set_wait_time(uint32_t wait_time) { this->wait_time_ = wait_time; }
  void set_optimistic(bool optimistic) { this->optimistic_ = optimistic; }

  valve::ValveTraits get_traits() override { return this->traits_; }
  fan::FanCall brake();

  float get_setup_priority() const override;
  void setup() override;
  void dump_config() override;


 protected:
  GPIOPin *pin_a_;
  GPIOPin *pin_b_;
  uint32_t pulse_length_{0};
  uint32_t wait_time_{0};
  bool optimistic_{false};

  void control(const valve::ValveCall &call) override;
  valve::ValveTraits get_traits() override;
};

}  // namespace hbridge
}  // namespace esphome
