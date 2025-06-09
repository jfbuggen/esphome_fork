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
    switch(this->hbridge_state_) {
      case HBRIDGE_SLEEP:
        if (valve_cmd_ != VALVE_CMD_NONE) {
          if (this->pin_sleep_ != nullptr) {
            this->pin_sleep_->digital_write(!this->pin_sleep_->is_inverted());
          }
          this->hbridge_state_ = HBRIDGE_WAKEUP;
        }
        break
      case HBRIDGE_IDLE:
        if (valve_cmd_ != VALVE_CMD_NONE) {
          // Something to do!
        } else {
          // Set to sleep if pin is configured
          if (this->pin_sleep_ != nullptr) {
            this->pin_sleep_->digital_write(this->pin_sleep_->is_inverted());
            this->hbridge_state_ = HBRIDGE_SLEEP;
          }
        }
        break;
      case HBRIDGE_PULSE:
        // if end of time, go to WAIT
        break;
      case HBRIDGE_WAIT:
        // if end of time, go to IDLE
        break;
      
)

void HBridgeValve::control(const valve::ValveCall &call) {
    if (this->valve_cmd_ != VALVE_CMD_NONE)
    {
        ESP_LOGV(TAG, "New command reveived while previous one was not yet processed");
    }
  
    // Check if a specific position is requested
    if (call.get_position().has_value()) {
      float position = *call.get_position();
      if (position == valve::VALVE_OPEN) {
          this->valve_cmd_ = VALVE_CMD_OPEN;
      } else if (position == valve::VALVE_CLOSED) {
          this->valve_cmd_ = VALVE_CMD_CLOSE;
      } else {
          ESP_LOGW(TAG, "Ignoring request to set valve to position %f (not supported)", position);
          this->valve_cmd_ = VALVE_CMD_NONE;
      }
    // Check if toggle is requested
    } else if (call.get_toggle().has_value()) {
      if (call.get_toggle().value()) {
          this->valve_cmd_ = VALVE_CMD_TOGGLE;
      }
    // Check if stop is requested
    } else if (call.get_stop()) {
      ESP_LOGW(TAG, "Ignoring request to stop valve (not supported)");
      this->valve_cmd_ = VALVE_CMD_NONE;
    }
  if (this->valve_cmd_ != VALVE_CMD_NONE) {
    this->execute_();
  }
}

void HBridgeValve::execute_() {
  uint32_t next_timeout = 0;

  // If timer is running, we are either during a pulse, or waiting after last pulse
  if (timer_running_) {
    return;
  }
  
  while (this->valve_cmd_ != VALVE_CMD_NONE) {
    command_ = this->valve_cmd_;
    // Convert toggle command into either open or close
    if (command_ == VALVE_CMD_TOGGLE)
    {
      switch (this->valve_state_) {
        case VALVE_STATE_OPEN:
        case VALVE_STATE_UNKNOWN:    // Toggling from unknown will request a close
          command_ = VALVE_CMD_CLOSE;
          break;
        case VALVE_STATE_CLOSE:
          command_ = VALVE_CMD_OPEN;
          break
        default:
          command_ = VALVE_CMD_NONE;
      }
    }
    if ((this->valve_state_ == VALVE_STATE_OPEN) && (command_ == VALVE_CMD_CLOSE)) {
          timer_running = true;
          this->pin_a_->digital_write(true);
          this->pin_b_->digital_write(false);
          next_timeout = this->pulse_length_;
    } else if ((this->valve_state_ == VALVE_STATE_CLOSED) && (command_ == VALVE_CMD_OPEN)) {
          timer_running = true;
          this->pin_a_->digital_write(false);
          this->pin_b_->digital_write(true);
          next_timeout = this->pulse_length_;
    }
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

