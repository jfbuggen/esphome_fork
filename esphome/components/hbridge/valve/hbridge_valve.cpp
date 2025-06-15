#include "hbridge_valve.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hbridge {

using namespace esphome::valve;

static const char *const TAG = "hbridge.valve";

void HBridgeValve::setup() {
  ESP_LOGCONFIG(TAG, "Running setup for '%s'", this->name_.c_str());
  this->hbridge_setup();
}

void HBridgeValve::loop() {
	
	HbridgeState cur_state = this->hbridge_state_;
	
	// h-bridge state machine
    switch(this->hbridge_state_) {
      case HbridgeState::HBRIDGE_SLEEP:
		// Wakeup if a command is waiting
        if (this->valve_cmd_ != VALVE_CMD_NONE) {
		  if ((this->hbridge_sleep(false) && (this->wakeup_duration_ms_ > 0)) {
			// Launch wakeup timeout to become IDLE only after wakeup_time (ms)
			this->hbridge_state_ = HbridgeState::HBRIDGE_WAKEUP;
		    this->set_timeout("wakeup", this->wakeup_duration_ms_, [this]() {
				this->hbridge_state_ = HbridgeState::HBRIDGE_IDLE;
			});
		  } else {
			this->hbridge_state_ = HbridgeState::HBRIDGE_IDLE;
		  }
        }
        break;
	  case HbridgeState::HBRIDGE_WAKEUP:
		// Do nothing, timeout callback will move to IDLE
		break;
      case HbridgeState::HBRIDGE_IDLE:
		// Convert toggle to OPEN, CLOSE or NONE
        if (this->valve_cmd_ == VALVE_CMD_TOGGLE) {
		  this->interpret_toggle_();
		}
		if (this->valve_cmd_ != VALVE_CMD_NONE) {
		  // Execute command
		  bool open_ = (this->valve_cmd_ == VALVE_CMD_OPEN);
		  this->hbridge_pulse_start_(open_);
		  this->current_operation = (open_ ? VALVE_OPERATION_OPENING : VALVE_OPERATION_CLOSING);
		  if (this->optimistic_) {
			 this->publish_position(open_);
		  }
		  if (this->pulse_duration_ms_ == 0) {
			this->hbridge_state_ = HbridgeState::HBRIDGE_PULSE_END;	// Release pulse asap (not realistic)
		  } else {
			// Launch pulse timeout. After pulse, either wait or bo back to idle
			this->hbridge_state_ = HbridgeState::HBRIDGE_PULSE;
		    this->set_timeout("pulse", this->pulse_duration_ms_, [this]() {
				this->hbridge_state_ = HbridgeState::HBRIDGE_PULSE_END;
			});
		  }		  
        } else {
          // Set to sleep (if no pin configured, returns false and stay IDLE)
          if (this->hbridge_sleep(true)) {
            this->hbridge_state_ = HbridgeState::HBRIDGE_SLEEP;
          }
        }
        break;
      case HbridgeState::HBRIDGE_PULSE:
        // Do nothing, timeout callback will move to PULSE_END
        break;
	  case HbridgeState::HBRIDGE_PULSE_END:
		this->end_pulse_();
		bool open_ = (this->current_operation == VALVE_OPERATION_OPENING);
		if (!this->optimistic) {
			this->publish_position(open_);
		}
		this->current_operation = VALVE_OPERATION_IDLE;
		if (this->wait_duration_ms_ == 0) {
		  this->hbridge_state_ = HbridgeState::HBRIDGE_IDLE;
		} else {
		  this->hbridge_state = HbridgeState::HBRIDGE_WAIT;
		  // Launch wait timeout
		  this->set_timeout("wait", this->wait_duration_ms_, [this]() {
			this->hbridge_state_ = HbridgeState::HBRIDGE_IDLE;
			});
		}
		break;
      case HbridgeState::HBRIDGE_WAIT:
        // Do nothing, timeout callback will move to IDLE
        break;
    }

	// Note: this will not log state changes via timeouts
	if (this->hbridge_state_ != cur_state) {
        ESP_LOGV(TAG, "State changed from %d to %d", cur_state, this->hbridge_state_);		
	}
}

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
}

void HBridgeValve::interpret_toggle_() {
    // Convert toggle command into either open or close
    if (this->valve_cmd_ == VALVE_CMD_TOGGLE)
    {
      switch (this->valve_state_) {
        case VALVE_STATE_OPEN:
        case VALVE_STATE_UNKNOWN:    // Toggling from unknown will request a close
          this->valve_cmd_ = VALVE_CMD_CLOSE;
          break;
        case VALVE_STATE_CLOSE:
          this->valve_cmd_ = VALVE_CMD_OPEN;
          break
        default:
          this->valve_cmd_ = VALVE_CMD_NONE; // Should never happen unless code above is changed so the TOGGLE UNKNOWN is not interpreted as CLOSE
      }
    }
}

void HBridgeValve::publish_position(bool open_) {
	this->position = (open_ ? VALVE_OPEN : VALVE_CLOSED);
	this->publish_state(true);
}

void HBridgeValve::dump_config() {
  LOG_VALVE("", "Template Valve", this);
  LOG_PIN("  Pin A: ", this->pin_a_);
  LOG_PIN("  Pin B: ", this->pin_b_);
  ESP_LOGCONFIG(TAG, "  Optimistic: %s", YESNO(this->optimistic_));
  ESP_LOGCONFIG(TAG, "  Pulse length: %" PRId32 " ms", this->pulse_duration_ms_);
  if (this->wait_duration_ms_)
    ESP_LOGCONFIG(TAG, "  Wait time %" PRId32 " ms", this->wait_duration_ms_);
  if (this->pin_sleep_)
	  LOG_PIN("  Pin Sleep: ", this->pin_sleep_);
  if (this->wakeup_duration_ms_)
    ESP_LOGCONFIG(TAG, "  Wakeup time %" PRId32 " ms", this->wakeup_duration_ms_);
}

ValveTraits TemplateValve::get_traits() {
  auto traits = ValveTraits();
  traits.set_is_assumed_state(false);
  traits.set_supports_stop(false);
  traits.set_supports_toggle(true);
  traits.set_supports_position(false);	// Only OPEN and CLOSE, no intermediate position possible
  return traits;
}

void HBridgeValve::hbridge_setup(void)
{
  this->pin_a_->setup();
  this->pin_b_->setup();
}

void HBridgeValve::hbridge_pulse_start_(bool open) {
	
    if ((this->valve_state_ == VALVE_STATE_OPEN) && (!open)) {
          this->pin_a_->digital_write(true);
          this->pin_b_->digital_write(false);
    } else if ((this->valve_state_ == VALVE_STATE_CLOSED) && (open)) {
          this->pin_a_->digital_write(false);
          this->pin_b_->digital_write(true);
          if (this->optimistic_)
			this->publish_state(true);
    } else {
		return;
	}
    if (this->optimistic_) {
	  this->publish_state(open);
	}
}

void HBridgeValve::hbridge_pulse_end_(bool open) {
    this->pin_a_->digital_write(false);
    this->pin_b_->digital_write(false);
    if (!this->optimistic_) {
	  this->publish_state(open);
	}
}

bool HBridgeValve::hbridge_sleep(bool sleep) {
  if (this->pin_sleep_ != nullptr) {
	 this->pin_sleep_->digital_write((this->pin_sleep_->is_inverted ? sleep : !sleep)));
	 return true;
  }
  return false;
}

