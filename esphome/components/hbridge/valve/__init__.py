from esphome import pins
import esphome.codegen as cg
from esphome.components import valve
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_PIN_A,
    CONF_PIN_B,
    CONF_SLEEP_PIN,
    CONF_PULSE_LENGTH,
    CONF_WAIT_TIME,
    CONF_OPTIMISTIC,
)
from .. import hbridge_ns

HBridgeValve = hbridge_ns.class_("HBridgeValve", valve.Valve, cg.Component)

CONF_WAKEUP_TIME = "conf_wakeup_time"

CODEOWNERS = ["@jfbuggen"]

CONFIG_SCHEMA = (
    valve.valve_schema(HBridgeValve)
    .extend(
        {
            cv.Required(CONF_PIN_A): pins.gpio_output_pin_schema,
            cv.Required(CONF_PIN_B): pins.gpio_output_pin_schema,
            cv.Optional(CONF_SLEEP_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_PULSE_LENGTH, default="50ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_WAIT_TIME, default="0ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_WAKEUP_TIME, default="1ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_OPTIMISTIC, default=False): cv.boolean,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = await valve.new_valve(config)
    await cg.register_component(var, config)

    pin_a = await cg.gpio_pin_expression(config[CONF_PIN_A])
    cg.add(var.set_pin_a(pin_a))
    pin_b = await cg.gpio_pin_expression(config[CONF_PIN_B])
    cg.add(var.set_pin_b(pin_b))
    if pin_sleep_config := config.get(CONF_SLEEP_PIN):
        pin_sleep = await cg.gpio_pin_expression(pin_sleep_config)
        cg.add(var.set_pin_sleep(sleep_pin))
    cg.add(var.set_pulse_length(config[CONF_PULSE_LENGTH]))
    cg.add(var.set_wait_time(config[CONF_WAIT_TIME]))
    cg.add(var.set_wakeup_time(config[CONF_WAKEUP_TIME]))
    cg.add(var.set_optimistic(config[CONF_OPTIMISTIC]))
