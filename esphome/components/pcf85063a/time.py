import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import automation
from esphome.components import i2c, time
from esphome.const import CONF_ID


CODEOWNERS = ["@ADeadPixel"]
DEPENDENCIES = ["i2c"]
pcf85063a_ns = cg.esphome_ns.namespace("pcf85063a")
PCF85063AComponent = pcf85063a_ns.class_("PCF85063AComponent", time.RealTimeClock, i2c.I2CDevice)
WriteAction = pcf85063a_ns.class_("WriteAction", automation.Action)
ReadAction = pcf85063a_ns.class_("ReadAction", automation.Action)


CONFIG_SCHEMA = time.TIME_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PCF85063AComponent),
    }
).extend(i2c.i2c_device_schema(0x68))


@automation.register_action(
    "pcf85063a.write_time",
    WriteAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(PCF85063AComponent),
        }
    ),
)
async def ds1307_write_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "pcf85063a.read_time",
    ReadAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(PCF85063AComponent),
        }
    ),
)
async def pcf85063a_read_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    await time.register_time(var, config)
