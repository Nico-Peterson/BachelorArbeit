

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

#define ROLE_SENDER 0x01

#define MAX_DATA_LEN 249

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

// Operations
#define RANGING_INIT 0x00
#define RANGING_DONE 0x01
#define RANGING_ACK 0x02
#define RANGIGN_MODE_MASTER 0x03
#define RECEIVE 0x05

#define ALL_DONE_PKT 0x06
#define START_RANGING 0x07
#define ANCHOR_PKT 0x10
#define RE_RANGING_PKT 0x11
#define CORNER_PKT 0x12
#define NONE 0xFF
//
#define SW0_NODE DT_ALIAS(sw0)

LOG_MODULE_REGISTER(Indoor_Localization_Mobile);

const uint16_t TxtimeoutmS = 5000;
const uint16_t RxtimeoutmS = 0xFFFF;
const uint32_t device_address = 01;

struct __attribute__((__packed__)) Coordinates
{
    bool flag; //  Validated Coordinates if TRUE else not validated
    float x;
    float y;
};

/*
************ Payload Format ************
DEVICE_ID | OPERATION | DATA_POINTER
*/

struct __attribute__((__packed__)) Payload
{
    uint32_t host_id;
    uint8_t operation;
    struct Coordinates coords;
};


// main function
void main(void)
{
    const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
	gpio_pin_configure_dt(&sw0, GPIO_INPUT);
    // Getting Device Id
    uint8_t hwid[4];
    ssize_t length;
    length = hwinfo_get_device_id(hwid, sizeof(hwid));
    uint32_t host_id = (hwid[0] << 24) | (hwid[1] << 16) | (hwid[2] << 8) | hwid[3];

    struct Coordinates dev_coords = {.flag = false, .x = -1, .y = -1};

    const struct device *lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
    struct lora_modem_config config;

    // Payload declaration
    struct Payload payload;
    uint8_t *payload_ptr;
    payload_ptr = &payload;

    struct Anchor *anchor_ptr;
    // struct Anchor *prev_anchor = NULL;
    // struct Anchor *temp_anchor;
    struct lora_ranging_params ranging_result;

    // General Variables
    int16_t rssi;
    int8_t snr;
    int ret, len;
    bool ranging_done = false;
    uint8_t operation = RECEIVE;

    int sample_count = 100;
    float sum = 0;
    float avg_fact = 0;
    int samples = 0;
    float ratio = 2570 / 1992;
    float avg_dist = 0;
    bool anchor_pkt_possible = false;

    if (!device_is_ready(lora_dev))
    {
        LOG_ERR("%s Device not ready", lora_dev->name);
        return;
    }

    config.frequency = 2445000000;
    config.bandwidth = BW_0800;
    config.datarate = SF_9;
    config.preamble_len = 12;
    config.coding_rate = CR_4_5;
    config.tx_power = 10;
    config.tx = true;

    // Setup LoRa Device
    ret = lora_config(lora_dev, &config);
    if (ret < 0)
    {
        LOG_ERR("LoRa config failed");
        return;
    }

    uint32_t name = 869725206;
    // BEGIN:
    bool notDone = true;
    while (notDone)
    {

        // Setup LoRa Device
        if (ranging_done)
        {
            ret = lora_config(lora_dev, &config);
            if (ret < 0)
            {
                LOG_ERR("LoRa config failed");
                return;
            }
            ranging_done = false;
        }

        switch (operation)
        {
        case RECEIVE:
            printk("RECEIVE MODE.\n");
            len = lora_recv(lora_dev, payload_ptr, MAX_DATA_LEN, K_FOREVER,
                            &rssi, &snr);
            printk("eyyyyo\n");   
            if (len < 0)
            {
                if (len == -(EAGAIN))
                {
                    
                    operation = RANGING_INIT;
                }
                else
                    operation = RECEIVE;
            }
            else
            {
                printk("data received: %d\n",payload.host_id);
                if (payload.operation == RANGING_INIT){
                    operation = START_RANGING;
                    name = payload.host_id;
                }
                else
                    operation = payload.operation;
            }
            break;
        case START_RANGING:
            lora_setup_ranging(lora_dev, &config, host_id, ROLE_SENDER);//
            samples = 0;
            uint32_t fail = 0;
            k_sleep(K_MSEC(2000));
            printk("{");
            size_t time = k_uptime_get_32();
            while (samples < sample_count)
            {
                ranging_result = lora_transmit_ranging(lora_dev, &config, name);
                size_t time1 = k_uptime_get_32();
                if (ranging_result.status != false && ranging_result.distance > 0)
                {
                    printk("%d,%zu,%d,%d",ranging_result.distance,(time1-time),ranging_result.RSSIReg,ranging_result.RSSIVal);
                    printk("%d,",(int)ranging_result.distance);
                    samples++;
                }else{
                    fail++;
                }
                time = time1;
            }
            printk("} %d\n",fail);
            
            operation = RANGING_DONE;
            notDone = false;
            break;

        default:
            operation = RECEIVE;
        }
    }
}