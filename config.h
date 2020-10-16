#define CONF_BATTERY_PATH "/sys/class/power_supply/BAT1/"
#define CONF_BATTERY_FORMAT "%.2lf%%"
#define CONF_TIME_FORMAT "%a %x - %X"
#define CONF_DELIMITER " "
#define CONF_DELIMITER_BEFORE "["
#define CONF_DELIMITER_AFTER "]"

#define CONF_CATCHALL_SIGNUM 40

static const actualizer_t actualizers[] = {
    /* func , sleeptime, signal, iscmd, cmd */
    //{ actualize_battery, 4, 12 , 0, ""},
    { NULL, 0, 35, 1, "zik"},
    { NULL, 0, 34, 1, "volume"},
    { NULL, 60, 0, 1, "mynet"},
    { NULL, 10, 0, 1, "ram"},
    { NULL, 60, 0, 1, "battery"},
    { actualize_time, 1, 11, 0, ""},
};


