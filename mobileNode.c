

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
double anchor_pos[7][2] = {{15,0},{20.39,0},{20.39,7.4},{20.39,21},{15.9,7.25},{0,0},{0,17.9}};
double messurements[7];
int hwadress[7] = {0x48,0x1c,0x1d,0xe7,0x66,0x50,0xd2};
double weights[7] = {1,1,1,1,1,1,1};

double x = 10;
double y = 10;
double LR = 0.01;

double derx(){
    double sum = 0;
    for (int i = 0; i < (sizeof(messurements)-1)/sizeof(double); i++){
        if(!(messurements[i] == 42)){
            double res1 = 2*(x-anchor_pos[i][0])*(sqrt(pow(x-anchor_pos[i][0],2) + pow(y-anchor_pos[i][1],2)) - messurements[i]);
            double res2 = res1 / sqrt(pow(x-anchor_pos[i][0],2) + pow(y-anchor_pos[i][1],2));
            sum = sum + res2;
            sum = sum * weights[i];
        }
    }
    return sum;
}

double dery(){
    double sum = 0;
    for (int i = 0; i < (sizeof(messurements)-1)/sizeof(double); i++){
        if(!(messurements[i] == 42)){
            double res1 = 2*(y-anchor_pos[i][1])*(sqrt(pow(x-anchor_pos[i][0],2) + pow(y-anchor_pos[i][1],2)) - messurements[i]);
            double res2 = res1 / sqrt(pow(x-anchor_pos[i][0],2) + pow(y-anchor_pos[i][1],2));
            sum = sum + res2;
            sum = sum * weights[i];
        }
    }
    return sum;
}


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
        lora_setup_ranging(lora_dev, &config, hwid[0], 0x01);

        x = 10;
        y = 10;

        printk("[");
        for (int i = 0; i < number; i++){
            double RSSI = 0;
            if(hwadress[i] == hwid[0]){
                messurements[i] = 42;
            }else{
                int sum = 0;
                for(int j = 0; j < sample_size;j++){
                    ranging_result = lora_transmit_ranging(lora_dev, &config, hwadress[i]);
                    if (ranging_result.status != false && ranging_result.distance > 0)
                    {
                        RSSI = ranging_result.RSSIReg;
                        sum = sum + ranging_result.distance;
                    }else{
                        j--;
                    }
                }
                messurements[i] = ((sum / sample_size)/100);
            }
            printk("[%d,%d]",(int)messurements[i],(int)RSSI);
        }
        printk("]\n");
        int precision = 10;
        int itersations = 0;
        while(1){
            itersations++;
            double step1 = derx() * LR;
            double step2 = dery() * LR;
            if(step1 < LR / precision && step2 < LR / precision){
                break;
            }
            x = x - step1;
            y = y - step2;
        }
        printk("X is %d and Y is %d and iterations %d\n",(int)x,(int)y,itersations);
    }
}