/////////////////////////////////////////////////////////////////////////////
////    CALCULADORA T-WATCH.                                             ////
////                        Alberto Mouriño Pardo. sep-2022.             ////
/////////////////////////////////////////////////////////////////////////////

#define LILYGO_WATCH_2020_V2
#include <LilyGoWatch.h>

TTGOClass *ttgo;
AXP20X_Class *axp;

#define pintar true

#if pintar
#define pinta(cosa) Serial.print(cosa)
#define pintaln(cosa) Serial.println(cosa)
#else
#define pinta(cosa)
#define pintaln(cosa)
#endif


int brillo = 50;
unsigned long tiempo_touch = millis();
unsigned long tmp = 0;
bool estado_touch = false;
int16_t x = 0;
int16_t y = 0;

char botones_calc[4][4] = {
  { '7', '8', '9', '*' },
  { '4', '5', '6', '-' },
  { '1', '2', '3', '+' },
  { '/', '0', '.', '=' }
};

float valores[20];
char operadores[20];
int indice_valores = 0;
int indice_operadores = 0;

char tecla = 0;
char input = 0;
char input_anterior = 0;
String cifra_str = "";
float cifra = 0;
float new_cifra = 0;
float old_cifra = 0;
int digitos = 0;
bool coma = false;
char operador_anterior = 0;
int x_operador = 2;
int y_operador = 54;


void setup() {
  Serial.begin(115200);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->setBrightness(brillo);
  ttgo->tft->fillScreen(TFT_BLACK);
  pinta_calculadora();
  pintaln("\n Fin Setup \n");
  // incluir captura hasta que se introduzca una cifra (con un while).
}


void loop() {
  /*  if (millis() > tmp + 5000) {
    tmp = millis();
    pintaln("Inicio loop");
  }*/
  captura_touch();
}


/////////////////  FUNCIONES  /////////////////


void captura_touch() {
  limpia_touch();
  estado_touch = ttgo->getTouch(x, y);
  if (estado_touch) {
    captura_teclas();
    pinta("\nArray valores: ");
    for (int i = 0; i < 20; i++) {
      pinta(String(valores[i]) + " - ");
    }
    pinta("\nArray operadores: ");
    for (int i = 0; i < 20; i++) {
      pinta(String(operadores[i]) + " - ");
    }
    pintaln("\n");
  }
}

void limpia_touch() {
  estado_touch = false;
  x = 0;
  y = 0;
}
////////////////////


void pinta_calculadora() {
  ttgo->tft->fillScreen(TFT_BLACK);
  ttgo->tft->setTextColor(TFT_WHITE, 0x7D7E);
  ttgo->tft->fillRoundRect(2, 6, 30, 42, 8, 0x7D7E);
  ttgo->tft->drawString("C", 8, 16, 4);
  ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
  int coord_x = 2;
  int coord_y = 54;
  ttgo->tft->setTextColor(TFT_WHITE, 0x7D7E);
  for (int i = 0; i < 4; i++) {
    for (int i2 = 0; i2 < 4; i2++) {
      ttgo->tft->fillRoundRect(coord_x, coord_y, 56, 42, 8, 0x7D7E);
      ttgo->tft->drawString(String(botones_calc[i][i2]), coord_x + 20, coord_y + 10, 4);
      coord_x += 60;
    }
    coord_x = 2;
    coord_y += 46;
  }
  estado_touch = false;
}


void captura_teclas() {
  if (y >= 54) {
    x = int((x - 2) / 60);
    y = int((y - 54) / 46);
    tecla = botones_calc[y][x];
    pinta("X: " + String(x) + "  -  ");
    pinta("Y: " + String(y) + "  -  ");
    pinta("Tecla: " + String(tecla));
    ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);

    if (isDigit(tecla) || tecla == 46) input = 1;
    if (tecla == 42 || tecla == 43 || tecla == 45 || tecla == 47) {
      input = 2;
      ttgo->tft->drawRoundRect(2 + x * 60, 54 + y * 46, 56, 42, 8, TFT_YELLOW);
    }
    if (tecla == 61) input = 3;

    switch (input) {
      case 0:
        break;

      case 1:
        if (tecla == 46 && coma) break;
        if (tecla == 46 && !coma) coma = true;
        cifra_str = cifra_str + String(tecla);
        cifra = cifra_str.toFloat();
        ttgo->tft->fillRect(36, 17, 222, 20, TFT_BLACK);
        ttgo->tft->drawString(cifra_str, 36, 17, 4);
        pinta(" Cifra ");
        pinta(cifra_str);
        break;

      case 2:
        pinta(" Operador ");
        cifra_str = "";
        coma = false;
        ttgo->tft->drawRoundRect(x_operador, y_operador, 56, 42, 8, 0x7D7E);
        ttgo->tft->drawRoundRect(x_operador + 1, y_operador + 1, 54, 40, 8, 0x7D7E);
        x_operador = 2 + x * 60;
        y_operador = 54 + y * 46;
        ttgo->tft->drawRoundRect(x_operador, y_operador, 56, 42, 8, TFT_YELLOW);
        ttgo->tft->drawRoundRect(x_operador + 1, y_operador + 1, 54, 40, 8, TFT_YELLOW);
        if (input_anterior == 1) {
          valores[indice_valores] = cifra;
          indice_valores++;
        }
        if (indice_valores == 20) indice_valores = 0;
        if (input_anterior == 2 && indice_operadores > 0) indice_operadores--;
        operadores[indice_operadores] = tecla;
        indice_operadores++;
        if (indice_operadores == 20) indice_operadores = 0;
        if (input_anterior == 1 && indice_valores > 1) {
          String resultado = proc_operaciones(operadores[indice_operadores - 2], valores[indice_valores - 2], valores[indice_valores - 1]);
          valores[indice_valores] = resultado.toFloat();
          indice_valores++;
          if (indice_valores == 20) indice_valores = 0;
          ttgo->tft->fillRect(36, 17, 222, 20, TFT_BLACK);
          ttgo->tft->drawString(resultado, 36, 17, 4);
          pinta("Resultado: " + resultado);
        }
        break;

      case 3:
        pinta(" igual ");
        cifra_str = "";
        coma = false;
        ttgo->tft->drawRoundRect(x_operador, y_operador, 56, 42, 8, 0x7D7E);
        ttgo->tft->drawRoundRect(x_operador + 1, y_operador + 1, 54, 40, 8, 0x7D7E);
        String resultado;
        if (indice_operadores > 0) {
          if (input_anterior == 1) {
            valores[indice_valores] = cifra;
            indice_valores++;
            if (indice_valores == 20) indice_valores = 0;

            resultado = proc_operaciones(operadores[indice_operadores - 1], valores[indice_valores - 2], valores[indice_valores - 1]);
            valores[indice_valores] = resultado.toFloat();
            indice_valores++;
            if (indice_valores == 20) indice_valores = 0;
            pinta("Resultado: " + resultado);
          } else {
            resultado = proc_operaciones(operadores[indice_operadores - 1], valores[indice_valores - 1], valores[indice_valores - 1]);
            valores[indice_valores] = resultado.toFloat();
            indice_valores++;
            if (indice_valores == 20) indice_valores = 0;
            pinta("Resultado: " + resultado);
          }
          ttgo->tft->fillRect(36, 17, 222, 20, TFT_BLACK);
          ttgo->tft->drawString(resultado, 36, 17, 4);
        }
        break;
    }
    input_anterior = input;
  } else if (x <= 30) {
    cifra_str = "";
    cifra = 0;
    old_cifra = 0;
    new_cifra = 0;
    digitos = 0;
    coma = false;
    for (int i = 0; i < 10; i++) {
      valores[i] = 0;
      operadores[i] = 0;
    }
    input = 0;
    input_anterior = 0;
    indice_valores = 0;
    indice_operadores = 0;
    ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
    ttgo->tft->fillRect(37, 16, 202, 20, TFT_BLACK);
  }
  delay(300);
}

String proc_operaciones(char tecla_, double cifra_1, double cifra_2) {
  float resultado = 0;
  String resultado_str = "";
  switch (tecla_) {
    case 43:
      resultado = cifra_1 + cifra_2;
      break;
    case 45:
      resultado = cifra_1 - cifra_2;
      break;
    case 42:
      resultado = cifra_1 * cifra_2;
      break;
    case 47:
      resultado = cifra_1 / cifra_2;
      break;
  }
  if (int(resultado * 10000) % 10000 == 0) {
    resultado_str = String(int(resultado));
  } else resultado_str = String(resultado);
  return (resultado_str);
}


////////////////////