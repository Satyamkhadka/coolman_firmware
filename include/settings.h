

// software settings
#define TEST_MODE true

#define NO_OF_BUTTONS 5
#define DEVICE_PREFIX "D-A-"

// pins settings
#define BUTTON_ONE GPIO_NUM_10
#define BUTTON_TWO GPIO_NUM_4
#define BUTTON_THREE GPIO_NUM_5
#define BUTTON_FOUR GPIO_NUM_6
#define BUTTON_FIVE GPIO_NUM_7

#define BUTTON_RESET GPIO_NUM_3

#define BUTTON_BUZZER GPIO_NUM_18

char service_name[25];

// web
#define WEB_SERVER "192.168.1.107" // domain eg.  "facebook.com"
#define WEB_PORT "8000"            // port eg.   443
#define WEB_URL "/api/data"        // route -    eg.  "/ap/data"