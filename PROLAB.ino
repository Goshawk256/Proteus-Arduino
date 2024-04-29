#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define POTENTIOMETER_PIN A0

#define NUM_ROWS_TOP 4
#define NUM_COLS_TOP 21
#define NUM_ROWS_BOTTOM 1
#define NUM_COLS_BOTTOM 4
#define SQUARE_SIZE 5 // Kare boyutu
#define SPACING 1     // Kareler arası boşluk

#define LED_PIN_1 A1
#define LED_PIN_2 A2
#define LED_PIN_3 A3
#define DISPLAY_PIN_A 2
#define DISPLAY_PIN_B 3
#define DISPLAY_PIN_C 4
#define DISPLAY_PIN_D 5
#define DISPLAY_PIN_E 6
#define DISPLAY_PIN_F 7
#define DISPLAY_PIN_G 8

// Topun konumu ve hızı
float ballX = SCREEN_WIDTH / 2;
float ballY = SCREEN_HEIGHT / 2;
float ballSpeedX = 2;
float ballSpeedY = 2;

bool gameOver = false;
int downCount = 0; // Aşağı düşme sayacı
int destroyedCount = 0; // Yok edilen kare sayısı

// Karelerin durumunu tutacak bir matris oluşturun
bool blockStatus[NUM_ROWS_TOP][NUM_COLS_TOP];

void setupGame() {
  Serial.begin(9600);

  // I2C adresi 0x3C ile başlat
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.display(); // başlangıç ekranı
  delay(2000);
  display.clearDisplay(); // ekranı ve buffer'ı temizle

  pinMode(POTENTIOMETER_PIN, INPUT);

  // LED pinlerini çıkış olarak ayarla ve başlangıçta yak
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  digitalWrite(LED_PIN_1, HIGH);
  digitalWrite(LED_PIN_2, HIGH);
  digitalWrite(LED_PIN_3, HIGH);

  // Yedi segment display pinlerini çıkış olarak ayarla ve başlangıçta sıfır olarak ayarla
  pinMode(DISPLAY_PIN_A, OUTPUT);
  pinMode(DISPLAY_PIN_B, OUTPUT);
  pinMode(DISPLAY_PIN_C, OUTPUT);
  pinMode(DISPLAY_PIN_D, OUTPUT);
  pinMode(DISPLAY_PIN_E, OUTPUT);
  pinMode(DISPLAY_PIN_F, OUTPUT);
  pinMode(DISPLAY_PIN_G, OUTPUT);
  setDisplayValue(0);

  // Karelerin durumunu başlangıçta tümü aktif olarak ayarlayın
  for (int row = 0; row < NUM_ROWS_TOP; row++) {
    for (int col = 0; col < NUM_COLS_TOP; col++) {
      blockStatus[row][col] = true;
    }
  }
}

void setup() {
  setupGame();
}

void loop() {
  if (!gameOver) {
    int potValue = analogRead(POTENTIOMETER_PIN);                                                                                                    // Potansiyometre değerini oku
    int xPos = map(potValue, 0, 1023, 0, SCREEN_WIDTH - (NUM_COLS_BOTTOM * (SQUARE_SIZE + SPACING))); // Potansiyometre değerini ekrana map'le

    display.clearDisplay(); // Önceki ekrandaki şekilleri temizle

    // Üstteki kareleri ekrana yerleştir
    for (int row = 0; row < NUM_ROWS_TOP; row++) {
      for (int col = 0; col < NUM_COLS_TOP; col++) {
        int x = col * (SQUARE_SIZE + SPACING);
        int y = row * (SQUARE_SIZE + SPACING);
        if (blockStatus[row][col]) { // Sadece aktif kareleri çiz
          display.drawRect(x, y, SQUARE_SIZE, SQUARE_SIZE, SSD1306_WHITE);
        }
      }
    }

    // Alt kısmı kapsayan dikdörtgeni çiz
    display.drawRect(xPos, 54, NUM_COLS_BOTTOM * (SQUARE_SIZE + SPACING), NUM_ROWS_BOTTOM * (SQUARE_SIZE + SPACING), SSD1306_WHITE);

    // Topun konumunu güncelle
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    // Topun ekrandan çıkmasını önleme ve sekmeleri kontrol etme
    if (ballX < 0 || ballX > SCREEN_WIDTH) {
      ballSpeedX *= -1;
    }
    if (ballY < 0) {
      ballSpeedY *= -1;
    } else if (ballY > SCREEN_HEIGHT) {
      downCount++; // Top aşağı düştü
      if(downCount==1){
      digitalWrite(LED_PIN_1, LOW);
      }
      if(downCount==2){
      digitalWrite(LED_PIN_2, LOW);
      }
      if(downCount==3){
      digitalWrite(LED_PIN_3, LOW);
      }
      if (downCount >= 3) { // 3 kere aşağı düşme limiti
        gameOver = true; // Oyun bitti
        ballX = SCREEN_WIDTH / 2;
        ballY = SCREEN_HEIGHT / 2;
        ballSpeedX = random(1, 3) * (random(0, 2) == 0 ? 1 : -1);
        ballSpeedY = random(1, 3);
        downCount = 0; // Sayacı sıfırla
        digitalWrite(LED_PIN_1, HIGH);
        digitalWrite(LED_PIN_2, HIGH);
        digitalWrite(LED_PIN_3, HIGH);
        digitalWrite(DISPLAY_PIN_A, HIGH);
        digitalWrite(DISPLAY_PIN_B, HIGH);
        digitalWrite(DISPLAY_PIN_C, HIGH);
        digitalWrite(DISPLAY_PIN_D, HIGH);
        digitalWrite(DISPLAY_PIN_E, HIGH);
        digitalWrite(DISPLAY_PIN_F, HIGH);
        digitalWrite(DISPLAY_PIN_G, HIGH);
        delay(1000); // Oyun bittiğinde bir saniye bekleyin
        setupGame(); // Oyunu yeniden başlat
      } else {
        // Oyun devam ediyor, topun konumunu başlangıç pozisyonuna al
        ballX = SCREEN_WIDTH / 2;
        ballY = SCREEN_HEIGHT / 2;
      }
    }

    // Topun alt dikdörtgene çarpması
    if (ballX >= xPos && ballX <= xPos + NUM_COLS_BOTTOM * (SQUARE_SIZE + SPACING) && ballY >= 54) {
      ballSpeedY *= -1; // Yönü tersine çevir
    }

    // Üst karelere çarpma kontrolü ve kareleri yok etme
    for (int row = 0; row < NUM_ROWS_TOP; row++) {
      for (int col = 0; col < NUM_COLS_TOP; col++) {
        int x = col * (SQUARE_SIZE + SPACING);
        int y = row * (SQUARE_SIZE + SPACING);
        if (blockStatus[row][col] && ballX >= x && ballX <= x + SQUARE_SIZE && ballY >= y && ballY <= y + SQUARE_SIZE) {
          ballSpeedY *= -1; // Yönü tersine çevir
          blockStatus[row][col] = false; // Kareyi yok et
          destroyedCount++; // Yok edilen kare sayısını arttır
          setDisplayValue(destroyedCount); // Yedi segment display'ini güncelle
        }
      }
    }

    // Topu çiz
    display.fillCircle(ballX + 2, ballY + 2, 2, SSD1306_WHITE);

    // Ekranı güncelle
    display.display();
  } else {
     // Oyun bitti, skoru ekrana yazdır
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, SCREEN_HEIGHT / 2 - 15);
    display.println("Oyun bitti!");
    display.setCursor(10, SCREEN_HEIGHT / 2);
    display.print("Skorunuz: ");
    display.println(destroyedCount); // Kırılan duvar sayısını skor olarak yazdır
    display.display();
    delay(1000);
    gameOver = false;
  }

  delay(10); // Stabilite için küçük bir gecikme
}

void setDisplayValue(int value) {
  switch (value) {
    case 0:
      digitalWrite(DISPLAY_PIN_A, HIGH);
      digitalWrite(DISPLAY_PIN_B, HIGH);
      digitalWrite(DISPLAY_PIN_C, HIGH);
      digitalWrite(DISPLAY_PIN_D, HIGH);
      digitalWrite(DISPLAY_PIN_E, HIGH);
      digitalWrite(DISPLAY_PIN_F, HIGH);
      digitalWrite(DISPLAY_PIN_G, LOW);
      break;
    case 1:
      digitalWrite(DISPLAY_PIN_A, LOW);
      digitalWrite(DISPLAY_PIN_B, HIGH);
      digitalWrite(DISPLAY_PIN_C, HIGH);
      digitalWrite(DISPLAY_PIN_D, LOW);
      digitalWrite(DISPLAY_PIN_E, LOW);
      digitalWrite(DISPLAY_PIN_F, LOW);
      digitalWrite(DISPLAY_PIN_G, LOW);
      break;
    case 2:
      digitalWrite(DISPLAY_PIN_A, HIGH);
      digitalWrite(DISPLAY_PIN_B, HIGH);
      digitalWrite(DISPLAY_PIN_C, LOW);
      digitalWrite(DISPLAY_PIN_D, HIGH);
      digitalWrite(DISPLAY_PIN_E, HIGH);
      digitalWrite(DISPLAY_PIN_F, LOW);
      digitalWrite(DISPLAY_PIN_G, HIGH);
      break;
    case 3:
     digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,LOW);
  digitalWrite(7,LOW);
  digitalWrite(8,HIGH);
      break;
      case 4:
      digitalWrite(2,LOW);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  digitalWrite(7,HIGH);
  digitalWrite(8,HIGH);
  break;
  case 5:
   digitalWrite(2,HIGH);
  digitalWrite(3,LOW);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,LOW);
  digitalWrite(7,HIGH);
  digitalWrite(8,HIGH);
  break;
  case 6:
  digitalWrite(2,HIGH);
  digitalWrite(3,LOW);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);
  digitalWrite(8,HIGH);
  break;
  case 7:
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  digitalWrite(7,LOW);
  digitalWrite(8,LOW);
  break;
  case 8:
   digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);
  digitalWrite(8,HIGH);
  break;
  case 9:
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,LOW);
  digitalWrite(7,HIGH);
  digitalWrite(8,HIGH);
  break;

    // Add cases for displaying other numbers as needed
  }
}
