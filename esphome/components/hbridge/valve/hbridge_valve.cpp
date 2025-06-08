#include "hbridge_valve.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hbridge {

using namespace esphome::valve;

static const char *const TAG = "hbridge.valve";

void HBridgeValve::setup() {
  ESP_LOGCONFIG(TAG, "Running setup for '%s'", this->name_.c_str());
  this->pin_a_->setup();
  this->pin_b_->setup();
  this->loop();
}

void HBridgeValve::loop() {

)

void HBridgeValve::control(const valve::ValveCall &call) {
    if (call.get_position().has_value()) {
      this->position = *call.get_position();
      this->publish_state();
      return;
    } else if (call.get_toggle().has_value()) {
      if (call.get_toggle().value()) {
        if (this->position == valve::VALVE_OPEN) {
          this->position = valve::VALVE_CLOSED;
          this->publish_state();
        } else {
          this->position = valve::VALVE_OPEN;
          this->publish_state();
        }
      }
      return;
    } else if (call.get_stop()) {
      this->current_operation = valve::VALVE_OPERATION_IDLE;
      this->publish_state();  // Keep the current position
      return;
    }
}

void HBridgeValve::write_state(bool state) {
  this->desired_state_ = state;
  if (!this->timer_running_)
    this->timer_fn_();
}

void HBridgeValve::timer_fn_() {
  uint32_t next_timeout = 0;

  while ((uint8_t) this->desired_state_ != this->valve_state_) {
    switch (this->valve_state_) {
      case VALVE_STATE_ON:
      case VALVE_STATE_OFF:
      case VALVE_STATE_UNKNOWN:
        if (this->desired_state_) {
          this->pin_a_->digital_write(true);
          this->pin_b_->digital_write(false);
          this->valve_state_ = VALVE_STATE_SWITCHING_ON;
        } else {
          this->pin_a_->digital_write(false);
          this->pin_b_->digital_write(true);
          this->valve_state_ = VALVE_STATE_SWITCHING_OFF;
        }
        next_timeout = this->pulse_length_;
        if (!this->optimistic_)
          this->publish_state(this->desired_state_);
        break;

      case VALVE_STATE_SWITCHING_ON:
        this->pin_a_->digital_write(false);
        this->pin_b_->digital_write(false);
        this->valve_state_ = VALVE_STATE_ON;
        if (this->optimistic_)
          this->publish_state(true);
        next_timeout = this->wait_time_;
        break;

      case VALVE_STATE_SWITCHING_OFF:
        this->pin_a_->digital_write(false);
        this->pin_b_->digital_write(false);
        this->valve_state_ = RELAY_STATE_OFF;
        if (this->optimistic_)
          this->publish_state(false);
        next_timeout = this->wait_time_;
        break;
    }

    if (next_timeout) {
      this->timer_running_ = true;
      this->set_timeout(next_timeout, [this]() { this->timer_fn_(); });
      return;
    }

    // In the case where OPEN/CLOSE state has been reached but we need to
    // immediately change back again to reach desired_state_, we loop.
  }
  this->timer_running_ = false;
}


void HBridgeValve::dump_config() {
  LOG_VALVE("", "Template Valve", this);
  LOG_PIN("  Pin A: ", this->pin_a_);
  LOG_PIN("  Pin B: ", this->pin_b_);
  ESP_LOGCONFIG(TAG, "  Optimistic: %s", YESNO(this->optimistic_));
  ESP_LOGCONFIG(TAG, "  Pulse length: %" PRId32 " ms", this->pulse_length_);
  if (this->wait_time_)
    ESP_LOGCONFIG(TAG, "  Wait time %" PRId32 " ms", this->wait_time_);
}

ValveTraits TemplateValve::get_traits() {
  auto traits = ValveTraits();
  traits.set_is_assumed_state(false);
  traits.set_supports_stop(false);
  traits.set_supports_toggle(true);
  traits.set_supports_position(false);
  return traits;
}

