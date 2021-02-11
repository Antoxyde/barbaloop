#define LIGHTRED "#cc241d"
#define RED "#cc241d"
#define ORANGE "#fe8019"
#define DARKORANGE "#af3a03"
#define BLUE "#83a598"
#define WHITE "#ebdbb2"
#define YELLOW "#fabd2f"
#define GREEN "#b8bb26"
#define BLACK "#1c1c1c"
#define ACQUA "#427b58"

#define MAX_STATUS_SIZE 256

#define CONF_TIME_FORMAT "%a %x - %X"
#define CONF_CATCHALL_SIGNUM 40
#define CONF_COLOR_TIME "^b" BLACK "^" "^c" WHITE "^" " "

#define CONF_BATT_PATH "/sys/class/power_supply/BAT0/"
#define CONF_LIMIT_BATT_LOW 10
#define CONF_LOW_BATTERY_MESSAGE "'Low battery !!'"
#define CONF_COLOR_BATT_DISCHARGING_LOW "^b" BLACK "^" "^c" RED "^" " "
#define CONF_COLOR_BATT_CHARGING "^b" BLACK "^" "^c" ACQUA "^" " " 
#define CONF_COLOR_BATT_DISCHARGING "^b" BLACK "^" "^c" ORANGE "^" " "
#define CONF_COLOR_BATT_UNKNOWN "^b" BLACK "^" "^c" BLUE "^" " "

#define CONF_COLOR_RAM "^b" BLACK "^" "^c" WHITE "^" " "
#define CONF_COLOR_RAM_HIGH "^b" BLACK "^" "^c" RED "^" " "
#define CONF_LIMIT_RAM_HIGH 14

#define CONF_COLOR_LOAD "^b" BLACK "^" "^c" WHITE "^" " "
#define CONF_COLOR_LOAD_HIGH "^b" BLACK "^" "^c" RED "^" " "
#define CONF_LIMIT_LOAD_HIGH 10.0

#define CONF_WIFI_IFACE  "wlp1s0"
#define CONF_WIFI_COLOR  "^b" BLACK "^" "^c" WHITE "^" " "
#define CONF_WIFI_COLOR_DOWN  "^b" BLACK "^" "^c" ORANGE "^" " " 

#define CONF_COLOR_VOLUME "^b" BLACK "^"  "^c" ACQUA "^" " " 
#define CONF_COLOR_VOLUME_MUTED "^b" BLACK "^" "^c" ORANGE "^" " " 

#define CONF_COLOR_MPD "^b" BLACK "^" "^c" ACQUA "^" " " 
#define CONF_COLOR_MPD_PAUSED "^b" BLACK "^" "^c" ORANGE "^" " " 
#define CONF_COLOR_MPD_UNKOWN "^b" BLACK "^" "^c" RED "^" " " 


static const actualizer_t actualizers[] = {
    /* func, signal, sleep time , format*/
    {actualize_wifi, 0, 60},
    {actualize_load, 0, 10},
    {actualize_ram, 0, 10},
    {actualize_battery, 0, 60}, 
    {actualize_time, 0, 1},
};
