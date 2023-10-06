#include <device.h>
#include <devicetree.h>
#include <drivers/lora.h>
#include <errno.h>
#include <sys/util.h>
#include <zephyr.h>
#include <logging/log.h>
#include <drivers/hwinfo.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
	     "No default LoRa radio specified in DT");


#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

LOG_MODULE_REGISTER(Hardware_Id_Collection);

void main(void){
    uint8_t hwid[4];
    ssize_t length;
    length = hwinfo_get_device_id(hwid, sizeof(hwid));

    const struct device *lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
    struct lora_modem_config config;

    if (!device_is_ready(lora_dev))
    {
        LOG_ERR("%s Device not ready.", lora_dev->name);
        return;
    }

    config.frequency = 2445000000;
    config.bandwidth = BW_0800;
    config.datarate = SF_9;
    config.preamble_len = 12;
    config.coding_rate = CR_4_5;
    config.tx_power = 10;
    config.tx = true;

    lora_setup_ranging(lora_dev, &config, hwid[0], 0x00);

    while (1)
    {
        lora_receive_ranging(lora_dev, &config, hwid[0], K_FOREVER);

    }
}