#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include "esphome/components/valve/valve.h"

namespace esphome {
namespace hbridge {

// State machine for the HBridge
enum class HbridgeState {
  HBRIDGE_IDLE,			// Ready to process a new command
  HBRIDGE_PULSE,		// Busy generating the pulse from the last command
  HBRIDGE_PULSE_END,	// End of pulse
  HBRIDGE_WAIT,			// Need to wait after last pulse (if wait_time configured)
  HBRIDGE_SLEEP,		// Sleep mode activated (if sleep pin is configured)
  HBRIDGE_WAKEUP		// Need to wait after wakeup (if sleep pin is configured and wakeup_time as well)
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
  void set_pin_sleep(InternalGPIOPin *pin) { this->pin_sleep_ = pin; }
  void set_pulse_length(uint32_t pulse_length) { this->pulse_duration_ms_ = pulse_length; }
  void set_wait_time(uint32_t wait_time) { this->wait_duration_ms_ = wait_time; }
  void set_wakeup_time(uint32_t wakeup_time) { this->wakeup_duration_ms_= wakeup_time; }
  void set_optimistic(bool optimistic) { this->optimistic_ = optimistic; }

  float get_setup_priority() const override;
  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
   // Valve
  ValveCmd valve_cmd_{VALVE_CMD_NONE};
  ValveState valve_state_{VALVE_STATE_UNKNOWN};
  bool optimistic_{false};
  void control(const valve::ValveCall &call) override;
  void interpret_toggle_(void);
  void publish_position(bool open_);
  valve::ValveTraits get_traits() override;

  // HBridge
  GPIOPin *pin_a_{nullptr};
  GPIOPin *pin_b_{nullptr};
  InternalGPIOPin *pin_sleep_{nullptr};
  uint32_t pulse_duration_ms_{50};
  uint32_t wait_duration_ms_{0};
  uint32_t wakeup_duration_ms_{0};
  HbridgeState hbridge_state_{HbridgeState::HBRIDGE_IDLE};
  void hbridge_setup_();
  void hbridge_pulse_start_(bool open);
  void hbridge_pulse_end_(bool open);
  bool hbridge_sleep_(bool sleep);
  

};

}  // namespace hbridge
}  // namespace esphome
