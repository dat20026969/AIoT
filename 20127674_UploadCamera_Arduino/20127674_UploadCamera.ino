#include <WiFi.h>
#include <esp32cam.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

const char* WIFI_SSID = "HTD";
const char* WIFI_PASS = "20012002";

AsyncWebServer server(80);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);

void serveJpg(AsyncWebServerRequest *request) {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    request->send(503, "image/jpeg", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", frame->data(), frame->size());
  request->send(response);
}

void handleJpgLo(AsyncWebServerRequest *request){
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg(request);
}

void handleJpgHi(AsyncWebServerRequest *request){
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg(request);
}

void handleJpgMid(AsyncWebServerRequest *request){
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg(request);
}

void setup(){
  Serial.begin(115200);
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam-mid.jpg");

  server.on("/cam-lo.jpg", HTTP_GET, handleJpgLo);
  server.on("/cam-hi.jpg", HTTP_GET, handleJpgHi);
  server.on("/cam-mid.jpg", HTTP_GET, handleJpgMid);

  server.begin();
}

void loop(){
  // Không cần gì ở đây vì server hoạt động không đồng bộ
}
