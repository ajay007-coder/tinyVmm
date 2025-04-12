#define MY_MODNAME "tinyVmm-AR: "

#define PR_INFO(fmt, ...)    pr_info(MY_MODNAME fmt, ##__VA_ARGS__)
#define PR_ERR(fmt, ...)     pr_err(MY_MODNAME fmt, ##__VA_ARGS__)
#define PR_WARN(fmt, ...)    pr_warn(MY_MODNAME fmt, ##__VA_ARGS__)
#define PR_DEBUG(fmt, ...)   pr_debug(MY_MODNAME fmt, ##__VA_ARGS__)


