from esphome import pins
import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_PIN_A,
    CONF_PIN_B,
    CONF_PULSE_LENGTH,
    CONF_WAIT_TIME,
    CONF_OPTIMISTIC,
)
from .. import hbridge_ns

HBridgeValve = hbridge_ns.class_("HBridgeValve", valve.Valve, cg.Component)

CODEOWNERS = ["@jfbuggen"]

CONFIG_SCHEMA = (
    valve.valve_schema(HBridgeValve)
    .extend(
        {
            cv.Required(CONF_PIN_A): pins.gpio_output_pin_schema,
            cv.Required(CONF_PIN_B): pins.gpio_output_pin_schema,
            cv.Optional(
                CONF_PULSE_LENGTH, default="50ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_WAIT_TIME): cv.positive_time_period_milliseconds,
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
    cg.add(var.set_pulse_length(config[CONF_PULSE_LENGTH]))
    cg.add(var.set_optimistic(config[CONF_OPTIMISTIC]))
    if wait_time := config.get(CONF_WAIT_TIME):
        cg.add(var.set_wait_time(wait_time))
