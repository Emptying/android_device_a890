/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#define LOG_NDEBUG 0
#define LOG_TAG "lights"

#include <cutils/log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

/******************************************************************************/
struct rgb_color {int r,g,b;};

static struct rgb_color colors[7]={{255,0,0},{0,255,0},{255,255,0},{0,0,255},{160,32,240},{0,255,255},{255,255,255}};
//red,green,yellow,blue,purple,cyan,white

static int msgMode = 0;
#define NOTIFICATION_MODE     1
#define ATTENTION_MODE         2
#define BATTERY_MODE         3

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct light_state_t g_notification;
static struct light_state_t g_battery;
static struct light_state_t g_attention;
static int msgCount = 0;
#define EF59_LED1_GREEN     0
#define EF59_LED1_BLUE      1
#define EF59_LED2_GREEN     2
#define EF59_LED2_BLUE      3
#define EF59_MENU_KEY       4
#define EF59_BACK_KEY       5
#define EF59_LED1_RED       6
#define EF59_LED2_RED       7
#define LP5523_LEDS	      8

char const*const LCD_FILE = "/sys/class/leds/lcd-backlight/brightness";
char const*const MENU_LED_FILE = "/sys/class/leds/lp5523:channel5/brightness";
char const*const BACK_LED_FILE = "/sys/class/leds/lp5523:channel6/brightness";
char const*const L_GREEN_LED_FILE = "/sys/class/leds/lp5523:channel1/brightness";
char const*const L_BLUE_LED_FILE = "/sys/class/leds/lp5523:channel2/brightness";
char const*const R_GREEN_LED_FILE = "/sys/class/leds/lp5523:channel3/brightness";
char const*const R_BLUE_LED_FILE = "/sys/class/leds/lp5523:channel4/brightness";
char const*const L_RED_LED_FILE = "/sys/class/leds/lp5523:channel7/brightness";
char const*const R_RED_LED_FILE = "/sys/class/leds/lp5523:channel8/brightness";
char const*const LED_WRITEON_FILE = "/dev/led_fops";

/**
 * device methods
 */

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);
}

static int
write_int(char const* path, int value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
   return -errno;
    }
}

static int
write_str(char const* path, char *value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[PAGE_SIZE];
        int bytes = sprintf(buffer, "%s\n", value);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_str failed to open %s\n", path);
            already_warned = 1;
        }
   return -errno;
    }
}

static int
findBestfitColor(struct rgb_color real_color)
{
    int tmp = 255 * 3;
    int color_index = 0;
    int i;
    for (i = 0; i < 7; i++)
    {
        struct rgb_color std_color = colors[i];
        //int val = (real_color.r - std_color.r) & 0x7F + (real_color.g - std_color.g) & 0x7F + (real_color.b - std_color.b) & 0x7F;
		int val = abs(real_color.r - std_color.r) + abs(real_color.g - std_color.g) + abs(real_color.b - std_color.b);
        if (val < tmp)
        {
            tmp = val;
            color_index = i;
        }
    }
	if (real_color.r==255 && real_color.g==255 && real_color.b==255)
	{
		color_index =6;
	}
    return color_index+1;
} 

static int
is_lit(struct light_state_t const* state)
{
    return state->color & 0x00ffffff;
}

static int
rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77*((color>>16)&0x00ff))
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

static int
set_light_backlight(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    int on = is_lit(state);
    pthread_mutex_lock(&g_lock);
    err = write_int(LCD_FILE, brightness);
    if(!on && msgMode==2){
		write_str(LED_WRITEON_FILE, "reset");
	msgMode = 0;
	}
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int
set_light_buttons(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int on = is_lit(state);
    int brightness = rgb_to_brightness(state);
    pthread_mutex_lock(&g_lock);
    if(on)
	{
		write_str(LED_WRITEON_FILE, "writeon5");
		write_str(LED_WRITEON_FILE, "writeon6");
    }
    else
    {
    	write_str(LED_WRITEON_FILE, "writeoff5");
    	write_str(LED_WRITEON_FILE, "writeoff6");
    }
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int
set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
	int i,red, green, blue;
	unsigned int colorRGB;
	int on = is_lit(state);
	int color_index;
	char buffer[PAGE_SIZE];
    pthread_mutex_lock(&g_lock);
#if 0
    ALOGD("BJW LED set_light_attention on = %d, color=0x%08x\n", on, state->color);
#endif
    g_notification = *state;
	colorRGB = state->color;

	red = (colorRGB >> 16) & 0xFF;
	green = (colorRGB >> 8) & 0xFF;
	blue = colorRGB & 0xFF;
#if 0
	ALOGD("BJW LED set_light_notifications on = %d ,R=%d,G=%d,B=%d\n", on, red, green, blue);
#endif	
	struct rgb_color real_color={red,green,blue};
	color_index = findBestfitColor(real_color);
#if 0
    ALOGD("color_index = %d", color_index);
#endif

	if (on){
		if (msgMode)	write_str(LED_WRITEON_FILE, "reset");
		sprintf(buffer, "mode%d\n", color_index);
	    write_str(LED_WRITEON_FILE, buffer);
	msgMode = 1;
	}
	else{
		if (msgMode)	write_str(LED_WRITEON_FILE, "reset");
	msgMode = 0;
	}
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_attention(struct light_device_t* dev,
        struct light_state_t const* state)
{
    msgCount++;
    int on = is_lit(state);
    if (on){
    	if (msgCount <= 2)
    	return 0;
    }
    pthread_mutex_lock(&g_lock);
#if 0
    ALOGD("BJW LED set_light_attention on = %d, color=0x%08x\n", on, state->color);
#endif
    g_attention = *state;
    if (state->flashMode == LIGHT_FLASH_HARDWARE) {
        g_attention.flashOnMS = state->flashOnMS;
    } else if (state->flashMode == LIGHT_FLASH_NONE) {
        g_attention.color = 0;
    }
	if (on){
		if (msgMode)	write_str(LED_WRITEON_FILE, "reset");
	    write_str(LED_WRITEON_FILE, "lower");
	msgMode = 2;
	}
	else{
		if (msgMode)	write_str(LED_WRITEON_FILE, "reset");
	msgMode = 0;
	}
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_battery(struct light_device_t* dev,
        struct light_state_t const* state)
{
	int i,red, green, blue;
	unsigned int colorRGB;
	int on = is_lit(state);
	int color_index;
	char buffer[PAGE_SIZE];
    pthread_mutex_lock(&g_lock);
#if 0
    ALOGD("set_light_battery color=0x%08x", state->color);
#endif
    g_battery = *state;
	colorRGB = state->color;

	red = (colorRGB >> 16) & 0xFF;
	green = (colorRGB >> 8) & 0xFF;
	blue = colorRGB & 0xFF;
#if 0
	ALOGD("set_light_battery R=%d,G=%d,B=%d\n", red, green, blue);
#endif	
	struct rgb_color real_color={red,green,blue};
	color_index = findBestfitColor(real_color);
#if 0
    ALOGD("color_index = %d", color_index);
#endif

	if (on){
		if (color_index==1){
			if (msgMode)	write_str(LED_WRITEON_FILE, "reset");
			write_str(LED_WRITEON_FILE, "red_dim");
		}
		else{
			if (msgMode)	write_str(LED_WRITEON_FILE, "reset");
			sprintf(buffer, "mode%d\n", color_index);
	    	write_str(LED_WRITEON_FILE, buffer);
		}
		msgMode = 3;
	}
	else{
		if (msgMode)	write_str(LED_WRITEON_FILE, "reset");
		msgMode = 0;
	}
    pthread_mutex_unlock(&g_lock);
    return 0;
}

/** Close the lights device */
static int
close_lights(struct light_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}


/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name))
        set_light = set_light_backlight;
    else if (0 == strcmp(LIGHT_ID_BUTTONS, name))
        set_light = set_light_buttons;
    else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name))
        set_light = set_light_notifications;
    else if (0 == strcmp(LIGHT_ID_ATTENTION, name))
        set_light = set_light_attention;
    else if (0 == strcmp(LIGHT_ID_BATTERY, name))
        set_light = set_light_battery;
    else
        return -EINVAL;

    pthread_once(&g_init, init_globals);

    struct light_device_t *dev = malloc(sizeof(struct light_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "Lights Module for Pantech IM-A900",
    .author = "benjaminwan@163.com",
    .methods = &lights_module_methods,
};
