#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> // Necesario para tipos como uint8_t, etc. si se usan aquí

// --- Definiciones de Pines ---
// Servos
#define SERVO_A_PIN 9
#define SERVO_B_PIN 10
#define SERVO_C_PIN 8

// Pines para el HX711
#define HX711_DT_PIN 4   // Pin de Datos (DT) conectado a A1 en la imagen (pero usamos D4 en Mega)
#define HX711_SCK_PIN 5  // Pin de Clock (SCK) conectado a A0 en la imagen (pero usamos D5 en Mega)

// Switches de Entrada (Usando INPUT_PULLUP)
#define CRUDO_PIN 37
#define DVH21_PIN 33
#define DVH28_PIN 35
#define POSICION_PIN 25
#define SSA_PIN 29        // Final de carrera Servo A
#define SSB_PIN 27        // Final de carrera Servo B
#define SSCP_PIN 31       // Final de carrera Servo C (positivo)
#define SSCN_PIN 23       // Final de carrera Servo C (negativo)

// --- Constantes de Tiempo ---
#define DEBOUNCE_DELAY 50       // Tiempo (ms) de anti-rebote
#define GLASS_SETTLE_TIME 1500  // Tiempo (ms) para asentamiento del vidrio
#define SERVO_MOVE_INTERVAL 20  // Intervalo (ms) entre pasos del servo

// --- Configuración Servos ---
#define SERVO_A_HOME_POS 80
#define SERVO_B_HOME_POS 180
#define SERVO_C_HOME_POS 90

// --- Constantes de Calibración (ADC - ¡¡VALORES POR DEFECTO, CALIBRAR!!) ---
#define ADC_A_HOME_DEFAULT 5000
#define ADC_B_HOME_DEFAULT 25000
#define MM_PER_ADC_A_DEFAULT 0.0015
#define MM_PER_ADC_B_DEFAULT 0.0015
#define TOTAL_GAP_MM_HOME_DEFAULT 66.5

// El signo negativo es común.
#define SCALE_FACTOR_DEFAULT -430.0 // Ejemplo: -430 unidades raw por gramo. ¡¡CALIBRAR!!
#define SCALE_OFFSET_DEFAULT 0L     // Offset (Tara), se ajusta automáticamente al tarar.

// --- Direcciones EEPROM (para guardar calibración) ---
#define EEPROM_ADDR_CAL_VALID 0     // 1 byte
#define EEPROM_ADDR_ADC_A_HOME (EEPROM_ADDR_CAL_VALID + 1) // 2 bytes (int16_t)
#define EEPROM_ADDR_ADC_B_HOME (EEPROM_ADDR_ADC_A_HOME + sizeof(int16_t)) // 2 bytes
#define EEPROM_ADDR_MM_PER_ADC_A (EEPROM_ADDR_ADC_B_HOME + sizeof(int16_t)) // 4 bytes (float)
#define EEPROM_ADDR_MM_PER_ADC_B (EEPROM_ADDR_MM_PER_ADC_A + sizeof(float)) // 4 bytes
#define EEPROM_ADDR_TOTAL_GAP (EEPROM_ADDR_MM_PER_ADC_B + sizeof(float)) // 4 bytes
#define CAL_VALID_FLAG 0x5A // Valor para marcar calibración como válida
#define EEPROM_ADDR_SCALE_FACTOR (EEPROM_ADDR_TOTAL_GAP + sizeof(float)) // 4 bytes (float)
#define EEPROM_ADDR_SCALE_OFFSET (EEPROM_ADDR_SCALE_FACTOR + sizeof(float)) // 4 bytes (long)

#define CAL_VALID_FLAG 0x5A // Valor para marcar calibración como válida (cubre ambas)


// --- Configuración ADC ADS1115 ---
#define ADS1115_I2C_ADDRESS 0x48 // Dirección I2C por defecto
#define POT_A_CHANNEL 0
#define POT_B_CHANNEL 1
#define POT_C_CHANNEL 2

// --- Configuración LCD ---
#define LCD_I2C_ADDRESS 0x3F
#define LCD_COLS 20
#define LCD_ROWS 4

// --- Dimensiones Placeholder (Opcional, si se usan) ---
// #define RAW_GLASS_HEIGHT_MM 500.0
// ...

#endif // CONFIG_H