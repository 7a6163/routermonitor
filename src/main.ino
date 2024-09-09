#include <lvgl.h>
#include <TFT_eSPI.h>
#include <string>
#include <ESP8266WiFi.h>
#include <ElegantOTA.h>
#include <ESP8266WebServer.h>

#include "NetData.h"

using namespace std;

const char *ssid = "HWCTQ";
const char *password = "qazxsw123";

LV_FONT_DECLARE(tencent_w7_22)
LV_FONT_DECLARE(tencent_w7_24)

TFT_eSPI tft = TFT_eSPI();
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

// 定义页面
static lv_obj_t *login_page = NULL;
static lv_obj_t *monitor_page = NULL;

// basic variables
static uint8_t test_data = 0;
static lv_obj_t *upload_label;
static lv_obj_t *down_label;
static lv_obj_t *up_speed_label;
static lv_obj_t *up_speed_unit_label;
static lv_obj_t *down_speed_label;
static lv_obj_t *down_speed_unit_label;
static lv_obj_t *cpu_bar;
static lv_obj_t *cpu_value_label;
static lv_obj_t *mem_bar;
static lv_obj_t *mem_value_label;
static lv_obj_t *temp_value_label;
static lv_obj_t *temperature_arc;
static lv_obj_t *ip_label;
static lv_style_t arc_indic_style;
static lv_obj_t *chart;

static lv_chart_series_t *ser1;
static lv_chart_series_t *ser2;

NetChartData netChartData;

lv_coord_t up_speed_max = 0;
lv_coord_t down_speed_max = 0;
// 监测数值
double up_speed;
double down_speed;
double cpu_usage;
double mem_usage;
double temp_value;
lv_coord_t upload_serise[10] = {0};
lv_coord_t download_serise[10] = {0};

ESP8266WebServer server(80);

// 原有的函数声明和实现保持不变...

// 修改 connectWiFi 函数
void connectWiFi()
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println(" ...");

    int i = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(i++);
        Serial.print(' ');
    }

    lv_obj_set_hidden(login_page, true);
    lv_obj_set_hidden(monitor_page, false);

    Serial.println("");
    Serial.println("Connection established!");
    Serial.print("IP address:    ");
    Serial.println(WiFi.localIP());

    // 初始化 ElegantOTA
    ElegantOTA.begin(&server);
    server.begin();
    Serial.println("HTTP server started");
}

// 修改 task_cb 函数
static void task_cb(lv_task_t *task)
{
    static uint32_t last_run = 0;
    uint32_t now = millis();
    if (now - last_run < 5000) { // 每5秒更新一次
        return;
    }
    last_run = now;

    // 原有的更新代码...
}

void setup()
{
    Serial.begin(921600);
    srand((unsigned)time(NULL));

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    tft.begin();
    tft.setRotation(0);

    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = read_encoder;
    lv_indev_drv_register(&indev_drv);

    setupPages();
    initLoginPage();

    // 原有的 UI 设置代码...

    lv_task_t *t = lv_task_create(task_cb, 100, LV_TASK_PRIO_MID, &test_data);
}

void loop()
{
    lv_task_handler();
    server.handleClient(); // 处理 OTA 更新请求
    ElegantOTA.loop(); // ElegantOTA 背景任务
}