#include <device.h>
#include <drivers/lora.h>
#include <errno.h>
#include <sys/util.h>
#include <zephyr.h>
#include <logging/log.h>
#include <drivers/hwinfo.h>
#include <drivers/gpio.h>
#include <stdlib.h>
#include <math.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
             "No default LoRa radio specified in DT");


#define MAX_DATA_LEN 249

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

#define SW0_NODE DT_ALIAS(sw0)

LOG_MODULE_REGISTER(Indoor_Localization_Mobile);

int sample_size = 100;
int number = 7;
int hwadress[7] = {0x48,0x1c,0x1d,0xe7,0x66,0x50,0xd2};

void main(void)
{
    uint8_t hwid[4];
    ssize_t length;
    length = hwinfo_get_device_id(hwid, sizeof(hwid));

    const struct device *lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
    struct lora_modem_config config;

    struct lora_ranging_params ranging_result;

    if (!device_is_ready(lora_dev))
    {
        LOG_ERR("%s Device not ready", lora_dev->name);
        return;
    }

    config.frequency = 2480000000;
    config.bandwidth = BW_0800;
    config.datarate = SF_9;
    config.preamble_len = 12;
    config.coding_rate = CR_4_5;
    config.tx_power = 10;
    config.tx = true;
    // Setup LoRa Device
    while(1){
        k_sleep(K_MSEC(10000));
        lora_setup_ranging(lora_dev, &config, hwid[0], 0x01);
        printk("[");
        for (int i = 0; i < number; i++){
            if(hwadress[i] == hwid[0]){
                continue;
            }else{
                uint16_t RSSIsum = 0;
                uint32_t Distsum = 0;
                for(int j = 0; j < sample_size;j++){
                    ranging_result = lora_transmit_ranging(lora_dev, &config, hwadress[i]);
                    if (ranging_result.status != false && ranging_result.distance > 0)
                    {
                        RSSIsum = RSSIsum + ranging_result.RSSIReg;
                        Distsum = Distsum + (uint32_t)ranging_result.distance;
                        printk("[%d,%d],",(uint32_t)ranging_result.distance,ranging_result.RSSIReg);
                    }else{
                        j--;
                    }
                }
            }
        }
        printk("]\n");
    }
}