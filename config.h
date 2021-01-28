#define RED "#cc241d"
#define ORANGE "#d65d0e"
#define BLUE "#83a598"
#define WHITE "#ebdbb2"
#define YELLOW "#fabd2f"

#define MAX_STATUS_SIZE 256

#define CONF_TIME_FORMAT "%a %x - %X"
#define CONF_CATCHALL_SIGNUM 40

#define CONF_BATT_PATH "/sys/class/power_supply/BAT0/"
#define CONF_LIMIT_BATT_LOW 10
#define CONF_LOW_BATTERY_MESSAGE "'Low battery !!'"
#define CONF_COLOR_BATT_DISCHARGING_LOW "^c" RED "^"
#define CONF_COLOR_BATT_CHARGING "^c" BLUE "^"
#define CONF_COLOR_BATT_DISCHARGING "^c" ORANGE "^"
#define CONF_COLOR_BATT_UNKNOWN "^c" WHITE "^"

#define CONF_COLOR_TIME "^c" BLUE "^"

#define CONF_COLOR_RAM "^c" YELLOW "^"
#define CONF_COLOR_RAM_HIGH "^c" RED "^"
#define CONF_LIMIT_RAM_HIGH 14

#define CONF_COLOR_LOAD "^c" YELLOW "^"
#define CONF_COLOR_LOAD_HIGH "^c" RED "^"
#define CONF_LIMIT_LOAD_HIGH 10.0

#define CONF_WIFI_IFACE  "wlp1s0"
#define CONF_WIFI_COLOR  "^c" BLUE "^"
#define CONF_WIFI_COLOR_DOWN  "^c" ORANGE "^"

static const actualizer_t actualizers[] = {
    /* func, signal, sleep time */
    {actualize_wifi, 0, 60},
    {actualize_load, 0, 10},
    {actualize_ram, 0, 10},
    {actualize_battery, 0, 60}, 
    {actualize_time, 0, 1},
};
