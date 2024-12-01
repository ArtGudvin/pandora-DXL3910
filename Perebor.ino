const int RELAY_PIN = 48; // Пин управления реле
const int LED_PIN = 49; // Пин сигнального красного диода
const int LED_COUNT_WRONG = 2; // Кол-во изменения состояния диода при неверном valet. Загорелся и потух это 2
const int LED_COUNTER_TIME = 6500; // Сколько времени следить за диодом после ввода пина в мс
const int DIGIT_DELAY = 2500; // Пауза между вводом чисел в мс

int v = 0;
int v1 = 1;
int v2 = 1;
int v3 = 1;
int v4 = 1;
int led_sw = 0;
char buf[30];
bool stp = true;
bool hasChanged = false; // Флаг изменения

void setup() {
pinMode(RELAY_PIN, OUTPUT);
pinMode(LED_PIN, INPUT);
Serial.begin(9600);
}

void parseV() {
v1 = (int)(v / 1000);
v2 = (int)(v % 1000 / 100);
v3 = (int)(v % 100 / 10);
v4 = (int)(v % 10);

if (v1 == 0) v1++;
if (v2 == 0) v2++;
if (v3 == 0) v3++;
if (v4 == 0) v4++;

v = v1 * 1000 + v2 * 100 + v3 * 10 + v4;
}

void writePinDigit(int num, bool is_delay) {
int n;

for (n = 0; n < num; n++) {
digitalWrite(RELAY_PIN, HIGH);
delay(100);
digitalWrite(RELAY_PIN, LOW);
delay(100);
}

if (is_delay) delay(DIGIT_DELAY);
}

void ledCounter() {
int max = LED_COUNTER_TIME / 10;
int prev = LOW;
int sw = 0;
int n = 0;
int st;

for (n = 0; n < max; n++) {
  st = digitalRead(LED_PIN);

  //Добавлен фильтр от одиночных коротких импульсов
  if (st != prev) {
    if (!hasChanged) {
      sw++;
      hasChanged = true; // Фиксируем, что было изменение
    } else {
      sw--;
      hasChanged = false; // сбрасываем флаг изменения
    }
  }
  // Если состояние вернулось к предыдущему, сбрасываем флаг
  if (st == prev) {
    hasChanged = false;
  }
  prev = st;

  delay(10);
  }

  if (sw >> LED_COUNT_WRONG) {  //для снятия с охраны  sw == LED_COUNT_WRONG
  sprintf(buf, "stop!");
  Serial.println(buf);
  stp = true;
  }

  led_sw = sw;
}

void iteration() {
writePinDigit(v1, true);
writePinDigit(v2, true);
writePinDigit(v3, true);
writePinDigit(v4, false);

ledCounter();

sprintf(buf, "code: %d, led: %d", v, led_sw);
Serial.println(buf);

if (!stp) {
v++;
parseV();
}
}

void loop() {
if (Serial.available() > 0) {
String rec = Serial.readString();

v = rec.toInt();
parseV();
stp = false;

sprintf(buf, "start code %d", v);
Serial.println(buf);
}

if (v > 0 && v < 10000 && !stp) {
iteration();
}
}