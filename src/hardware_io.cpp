#include "hardware_io.h"
#include "config.h" // Necesario para pines y configuraciones
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_ADS1X15.h>
#include <HX711.h> // Añadir la librería para el módulo HX711

// --- Objetos globales de Hardware (privados a este módulo) ---
namespace { // Usar namespace anónimo para encapsular
    Servo ServoA;
    Servo ServoB;
    Servo ServoC;
    LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);
    Adafruit_ADS1115 ads;
    HX711 scale; // Objeto para la báscula

    // Estructura para manejar el estado anti-rebote por pin
    struct DebounceState {
        int currentState = HIGH;
        int lastReading = HIGH;
        unsigned long lastDebounceTime = 0;
    };

    // Crear instancias de estado para cada pin que necesita debounce
    DebounceState debounceCrudo;
    DebounceState debounceDvh21;
    DebounceState debounceDvh28;
    DebounceState debouncePosicion;
    // Añadir más si otros switches necesitan debounce (ej. botones de calibración)

} // fin namespace anónimo

// --- Implementación de Funciones ---

bool hardware_setup() {
    Serial.begin(115200);
    while (!Serial); // Espera a que el puerto serie esté listo (importante para algunos Arduinos)
    serial_print("Iniciando Hardware IO...\n", true);

    // Inicializa I2C
    Wire.begin();

    // Inicializa LCD
    lcd.init();
    lcd.backlight();
    lcd_display("Inicializando...", "Hardware", "Por favor espere");
    serial_print("LCD inicializado.\n", true);

    // Inicializa ADS1115
    if (!ads.begin(ADS1115_I2C_ADDRESS)) {
        serial_print("ERROR: No se pudo conectar al ADS1115.\n", true);
        lcd_display("!! ERROR !!", "ADC no encontrado", "(ADS1115)");
        return false; // Error crítico
    }
    ads.setGain(GAIN_ONE); // Ajustar ganancia si es necesario
    serial_print("ADS1115 conectado.\n", true);

    // Attach Servos
    ServoA.attach(SERVO_A_PIN);
    ServoB.attach(SERVO_B_PIN);
    ServoC.attach(SERVO_C_PIN);
    serial_print("Servos attached.\n", true);

    // Configura Pines de Switches
    pinMode(CRUDO_PIN, INPUT_PULLUP);
    pinMode(DVH21_PIN, INPUT_PULLUP);
    pinMode(DVH28_PIN, INPUT_PULLUP);
    pinMode(POSICION_PIN, INPUT_PULLUP);
    pinMode(SSA_PIN, INPUT_PULLUP);
    pinMode(SSB_PIN, INPUT_PULLUP);
    pinMode(SSCP_PIN, INPUT_PULLUP);
    pinMode(SSCN_PIN, INPUT_PULLUP);
    serial_print("Pines de entrada configurados.\n", true);

    // Inicializa HX711
    serial_print("Iniciando HX711...\n", true);
    scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
    if (!scale.is_ready()) {
         serial_print("WARN: HX711 no encontrado o no listo inicialmente.\n", true);
         // Podría no ser un error crítico si se recupera, pero informar.
         // lcd_display("!! ADVERTENCIA !!", "Bascula HX711", "no detectada");
         // delay(2000); // Mostrar advertencia
    } else {
        serial_print("HX711 listo.\n", true);
    }

    serial_print("Hardware IO Listo.\n", true);
    lcd_clear(); // Limpiar LCD después de la inicialización
    return true;
}

void servo_A_move(int position) {
    ServoA.write(position);
}

void servo_B_move(int position) {
    ServoB.write(position);
}

void servo_C_move(int position) {
    ServoC.write(position);
}

int16_t read_adc(uint8_t channel) {
    if (channel > 3) {
        serial_print("Error: Canal ADS invalido: ", false);
        serial_print((int)channel, true);
        return 0; // O un valor específico de error si se prefiere
    }
    return ads.readADC_SingleEnded(channel);
}

int read_switch_instant(uint8_t pin) {
    return digitalRead(pin);
}

// Implementación interna del debounce
namespace {
    int debounce_pin(uint8_t pin, DebounceState& state) {
        int reading = digitalRead(pin);

        if (reading != state.lastReading) {
            state.lastDebounceTime = millis(); // Reset timer
        }

        if ((millis() - state.lastDebounceTime) > DEBOUNCE_DELAY) {
            // El estado ha sido estable por suficiente tiempo
            if (reading != state.currentState) {
                state.currentState = reading;
                // Opcional: añadir un flag o callback si se necesita saber *cuándo* cambia
            }
        }

        state.lastReading = reading;
        return state.currentState; // Devuelve el estado estable actual
    }
} // fin namespace anónimo


int read_switch_debounced(uint8_t pin) {
    // Seleccionar la estructura de estado correcta según el pin
    switch (pin) {
        case CRUDO_PIN:     return debounce_pin(pin, debounceCrudo);
        case DVH21_PIN:     return debounce_pin(pin, debounceDvh21);
        case DVH28_PIN:     return debounce_pin(pin, debounceDvh28);
        case POSICION_PIN:  return debounce_pin(pin, debouncePosicion);
        // Añadir casos para otros pines con debounce si es necesario
        default:
            serial_print("WARN: Debounce no implementado para pin: ", false);
            serial_print((int)pin, true);
            return digitalRead(pin); // Devolver lectura instantánea como fallback
    }
}

// --- Funciones Báscula ---

void scale_set_calibration(long offset, float factor) {
    scale.set_offset(offset);
    scale.set_scale(factor);
    serial_print("Calibracion bascula aplicada: Offset=", false);
    serial_print(offset, false);
    serial_print(" Factor=", false);
    serial_print(factor, 3, true);
}

long scale_tare(int times) {
    serial_print("Realizando Tara... ", false);
    scale.tare(times); // La librería calcula y establece el offset internamente
    long current_offset = scale.get_offset(); // Obtenemos el offset calculado por la librería
    serial_print("Tara completa. Offset calculado: ", false);
    serial_print(current_offset, true);
    return current_offset; // Devolvemos el offset para guardarlo si es necesario
}

float scale_get_weight_grams(int times) {
    if (scale.is_ready()) {
        // La función get_units() aplica el factor y el offset
        float weight = scale.get_units(times);
        // Serial.print("Peso medido (g): "); Serial.println(weight); // Debug frecuente
        return weight;
    } else {
        serial_print("WARN: HX711 no listo para leer peso.\n", true);
        return 0.0; // O un valor NaN si se prefiere indicar error
    }
}

long scale_get_raw_reading(int times) {
     if (scale.is_ready()) {
        return scale.read_average(times);
    } else {
        serial_print("WARN: HX711 no listo para lectura raw.\n", true);
        return 0L; // O un valor indicativo de error
    }
}

bool scale_is_ready() {
    return scale.is_ready();
}

void lcd_display(const char* line1, const char* line2, const char* line3, const char* line4) {
    lcd.clear();
    if (line1) { lcd.setCursor(0, 0); lcd.print(line1); }
    if (line2) { lcd.setCursor(0, 1); lcd.print(line2); }
    if (line3) { lcd.setCursor(0, 2); lcd.print(line3); }
    if (line4) { lcd.setCursor(0, 3); lcd.print(line4); }
}

void lcd_clear() {
    lcd.clear();
}

void serial_print(const char* message, bool newline) {
    Serial.print(message);
    if (newline) Serial.println();
}
void serial_print(int value, bool newline) {
    Serial.print(value);
    if (newline) Serial.println();
}
void serial_print(float value, int decimals, bool newline) {
    Serial.print(value, decimals);
    if (newline) Serial.println();
}