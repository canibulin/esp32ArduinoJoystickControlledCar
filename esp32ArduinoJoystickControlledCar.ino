/***
 *   Arturo Gonzalez M. 
 *   Arduino remote controlled car with esp32 Ai Thinker
 *   Based on the code from:
 *   https://randomnerdtutorials.com/esp32-cam-car-robot-web-server/
 *   
 * ***/


#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems
#include "esp_http_server.h"

//Motor Variables
#define MOTOR_1_PIN_1    14
#define MOTOR_1_PIN_2    15
#define MOTOR_2_PIN_1    13
#define MOTOR_2_PIN_2    12

// Replace with your network credentials
const char* ssid = "esp32";
const char* password = "123456789";

#define PART_BOUNDARY "123456789000000000000987654321"

//Camera Variables
#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

// transform: scaleX(-1);
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>ESP32-CAM Robot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-left: auto; margin-right: auto; }
      td { padding: 8 px; }
      .button {
        background-color: #2f4468;
        border: none;
        color: white;
        padding: 10px 20px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }
      img {  width: auto ;
        max-width: 100% ;
        height: auto ;
        transform:rotate(90deg); 
       
      }
      #container {
            width: 100%;
            height: 49vh;
            background-color: #333;
            display: flex;
            align-items: center;
            justify-content: center;
            overflow: hidden;
            border-radius: 7px;
            touch-action: none;
        }

        #item {
            width: 100px;
            height: 100px;
            background-color: rgb(245, 230, 99);
            border: 10px solid rgba(136, 136, 136, .5);
            border-radius: 50%;
            touch-action: none;
            user-select: none;
        }

        #item:active {
            background-color: rgba(168, 218, 220, 1.00);
        }

        #item:hover {
            cursor: pointer;
            border-width: 20px;
        }

        #area {
            position: fixed;
            right: 0;
            top: 0;
        }
    </style>

<script type="text/javascript">
        var JoyStick = function (t, e) { var i = void 0 === (e = e || {}).title ? "joystick" : e.title, n = void 0 === e.width ? 0 : e.width, o = void 0 === e.height ? 0 : e.height, h = void 0 === e.internalFillColor ? "#00AA00" : e.internalFillColor, r = void 0 === e.internalLineWidth ? 2 : e.internalLineWidth, d = void 0 === e.internalStrokeColor ? "#003300" : e.internalStrokeColor, a = void 0 === e.externalLineWidth ? 2 : e.externalLineWidth, l = void 0 === e.externalStrokeColor ? "#008000" : e.externalStrokeColor, c = document.getElementById(t), u = document.createElement("canvas"); u.id = i, 0 == n && (n = c.clientWidth), 0 == o && (o = c.clientHeight), u.width = n, u.height = o, c.appendChild(u); var s = u.getContext("2d"), f = 0, v = 2 * Math.PI, g = (u.width - 110) / 2, w = g + 5, C = g + 30, m = u.width / 2, p = u.height / 2, L = u.width / 10, E = -1 * L, S = u.height / 10, k = -1 * S, W = m, G = p; function x() { s.beginPath(), s.arc(m, p, C, 0, v, !1), s.lineWidth = a, s.strokeStyle = l, s.stroke() } function y() { s.beginPath(), W < g && (W = w), W + g > u.width && (W = u.width - w), G < g && (G = w), G + g > u.height && (G = u.height - w), s.arc(W, G, g, 0, v, !1); var t = s.createRadialGradient(m, p, 5, m, p, 200); t.addColorStop(0, h), t.addColorStop(1, d), s.fillStyle = t, s.fill(), s.lineWidth = r, s.strokeStyle = d, s.stroke() } "ontouchstart" in document.documentElement ? (u.addEventListener("touchstart", function (t) { f = 1 }, !1), u.addEventListener("touchmove", function (t) { t.preventDefault(), 1 == f && (W = t.touches[0].pageX, G = t.touches[0].pageY, W -= u.offsetLeft, G -= u.offsetTop, s.clearRect(0, 0, u.width, u.height), x(), y()) }, !1), u.addEventListener("touchend", function (t) { f = 0, W = m, G = p, s.clearRect(0, 0, u.width, u.height), x(), y() }, !1)) : (u.addEventListener("mousedown", function (t) { f = 1 }, !1), u.addEventListener("mousemove", function (t) { 1 == f && (W = t.pageX, G = t.pageY, W -= u.offsetLeft, G -= u.offsetTop, s.clearRect(0, 0, u.width, u.height), x(), y()) }, !1), u.addEventListener("mouseup", function (t) { f = 0, W = m, G = p, s.clearRect(0, 0, u.width, u.height), x(), y() }, !1)), x(), y(), this.GetWidth = function () { return u.width }, this.GetHeight = function () { return u.height }, this.GetPosX = function () { return W }, this.GetPosY = function () { return G }, this.GetX = function () { return ((W - m) / w * 100).toFixed() }, this.GetY = function () { return ((G - p) / w * 100 * -1).toFixed() }, this.GetDir = function () { var t = "", e = W - m, i = G - p; return i >= k && i <= S && (t = "C"), i < k && (t = "N"), i > S && (t = "S"), e < E && ("C" == t ? t = "W" : t += "W"), e > L && ("C" == t ? t = "E" : t += "E"), t } };
    </script>
    
  </head>
  <body>
    <h1>Robot WL-01</h1>
    <div>
    <div style="height: 10vh;"></div>
     <img src="" id="photo" >
    </div>
   
     <div id='outerContainer'>
        <div style="height: 10vh;"></div>
        <div id="joyDiv" style="width:200px;height:200px;margin:auto;"></div>
    </div>
   
   <script>
  
   window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";

   // Create JoyStick object into the DIV 'joyDiv'
    var joy = new JoyStick('joyDiv');
    var inputPosX = document.getElementById("posizioneX");
    var inputPosY = document.getElementById("posizioneY");
    var direzione = document.getElementById("direzione");
    var fuerzaI = document.getElementById("fuerzaI");
    var fuerzaD = document.getElementById("fuerzaD");
    var x = document.getElementById("X");
    var y = document.getElementById("Y");




    //Steering control program, based on :
    // Differential Steering Joystick Algorithm
    //   by Calvin Hass
    //   https://www.impulseadventure.com/elec/

    function getfuerza(nJoyX, nJoyY) {
        var nMotMixL;
        var nMotMixR;
        var fPivYLimit = 32.0; //The threshold at which the pivot action starts
        //                This threshold is measured in units on the Y-axis
        //                away from the X-axis (Y=0). A greater value will assign
        //                more of the joystick's range to pivot actions.
        //                Allowable range: (0..+127)

        // TEMP VARIABLES
        var nMotPremixL;    // Motor (left)  premixed output        (-128..+127)
        var nMotPremixR;    // Motor (right) premixed output        (-128..+127)
        var nPivSpeed;      // Pivot Speed                          (-128..+127)
        var fPivScale;       // Balance scale b/w drive and pivot    (   0..1   )

        // Calculate Drive Turn output due to Joystick X input
        if (nJoyY >= 0) {
            // Forward
            nMotPremixL = (nJoyX >= 0 ? 100.0 : 100.0 + parseFloat(nJoyX));
            nMotPremixR = (nJoyX >= 0 ? 100.0 - nJoyX : 100.0);
        } else {
            // Reverse
            nMotPremixL = (nJoyX >= 0 ? 100.0 - nJoyX : 100.0);
            nMotPremixR = (nJoyX >= 0 ? 100.0 : 100.0 + parseFloat(nJoyX));
        }

        // Scale Drive output due to Joystick Y input (throttle)
        nMotPremixL = nMotPremixL * nJoyY / 100.0;
        nMotPremixR = nMotPremixR * nJoyY / 100.0;

        // Now calculate pivot amount
        // - Strength of pivot (nPivSpeed) based on Joystick X input
        // - Blending of pivot vs drive (fPivScale) based on Joystick Y input
        nPivSpeed = nJoyX;
        fPivScale = (Math.abs(nJoyY) > fPivYLimit) ? 0.0 : (1.0 - Math.abs(nJoyY) / fPivYLimit);

        // Calculate final mix of Drive and Pivot
        nMotMixL = (1.0 - fPivScale) * nMotPremixL + fPivScale * (nPivSpeed);
        nMotMixR = (1.0 - fPivScale) * nMotPremixR + fPivScale * (-nPivSpeed);




        return Math.round(nMotMixL * 2.55) + "," + Math.round(nMotMixR * 2.55);   // The function returns the product of p1 and p2
    }

    setInterval(function () { send(getfuerza(joy.GetX(), joy.GetY())); }, 200);

         function send(txt) {
            
              var xhr = new XMLHttpRequest();
  
  xhr.open("GET", "/action?VAL="+txt, true);
  xhr.send();
        }
    
  </script>
  </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[64] = {0,};
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "VAL", variable, sizeof(variable)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  sensor_t * s = esp_camera_sensor_get();
  int res = 0;

  Serial.println(variable);
  int commaIndex = String(variable).indexOf(',');
  int LValue = String(variable).substring(0, commaIndex).toInt();
  int RValue = String(variable).substring(commaIndex + 1).toInt();
  //Serial.println(LValue);
  //Serial.println(RValue);
  if(LValue>100&&RValue>100){
    Serial.println("Foward");
    foward();
  }else if(LValue>100&&RValue<0){
    Serial.println("Right");
    right();
  }else if(LValue<-100&&RValue<-100){
    Serial.println("Back");
    reverse();
  }else if(LValue<0&&RValue>100){
    Serial.println("Left");
    left();
  }else{
    Serial.println("Stop");
    stopp();
  }

  //We send correct response to avoid errors
    //res = -1;
  

  if(res){
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  pinMode(MOTOR_1_PIN_1, OUTPUT);
  pinMode(MOTOR_1_PIN_2, OUTPUT);
  pinMode(MOTOR_2_PIN_1, OUTPUT);
  pinMode(MOTOR_2_PIN_2, OUTPUT);
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
 WiFi.softAP(ssid, password);
IPAddress IP = WiFi.softAPIP();
Serial.print("AP IP address: ");
Serial.println(IP);
  
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());
  
  // Start streaming web server
  startCameraServer();
}

void loop() {
  
}

void foward(){
  digitalWrite(MOTOR_1_PIN_1, 1);
  digitalWrite(MOTOR_1_PIN_2, 0);
  digitalWrite(MOTOR_2_PIN_1, 1);
  digitalWrite(MOTOR_2_PIN_2, 0);
}

void reverse(){
  digitalWrite(MOTOR_1_PIN_1, 0);
  digitalWrite(MOTOR_1_PIN_2, 1);
  digitalWrite(MOTOR_2_PIN_1, 0);
  digitalWrite(MOTOR_2_PIN_2, 1);
}

void right(){
  digitalWrite(MOTOR_1_PIN_1, 1);
  digitalWrite(MOTOR_1_PIN_2, 0);
  digitalWrite(MOTOR_2_PIN_1, 0);
  digitalWrite(MOTOR_2_PIN_2, 1);
}

void left(){
  digitalWrite(MOTOR_1_PIN_1, 0);
  digitalWrite(MOTOR_1_PIN_2, 1);
  digitalWrite(MOTOR_2_PIN_1, 1);
  digitalWrite(MOTOR_2_PIN_2, 0);
}

void stopp(){
  digitalWrite(MOTOR_1_PIN_1, 0);
  digitalWrite(MOTOR_1_PIN_2, 0);
  digitalWrite(MOTOR_2_PIN_1, 0);
  digitalWrite(MOTOR_2_PIN_2, 0);
}
