#include "measurement.h"
#include "config.h"       // Para constantes default y direcciones EEPROM
#include "hardware_io.h"  // Para imprimir en Serial/LCD durante carga/guardado/calibración
#include <EEPROM.h>

// --- Definición de Variables Globales de Calibración ---
// (Sin 'extern' aquí, esta es la definición real)
int16_t cal_AdcAHome;
int16_t cal_AdcBHome;
float cal_MmPerAdcA;
float cal_MmPerAdcB;
float cal_TotalGapMmHome;

// --- Implementación de Funciones ---

float calculate_thickness_adc(int16_t finalAdcValueA, int16_t finalAdcValueB) {
    // Calcular cuánto se movió cada palpador desde su posición HOME
    // ¡¡VERIFICAR LA DIRECCIÓN DEL CAMBIO DEL ADC CON EL MOVIMIENTO!!
    // Asumimos aquí que un ADC más alto significa "más adentro" para AMBOS.
    // Si para uno es al revés, cambiar el cálculo del delta.
    int16_t deltaAdcA = finalAdcValueA - cal_AdcAHome;
    int16_t deltaAdcB = finalAdcValueB - cal_AdcBHome;

    // Convertir cambio ADC a distancia recorrida en mm
    float travelA_mm = (float)deltaAdcA * cal_MmPerAdcA;
    float travelB_mm = (float)deltaAdcB * cal_MmPerAdcB;

    // El espesor es la separación inicial menos lo que avanzó cada palpador
    float thickness = cal_TotalGapMmHome - travelA_mm - travelB_mm;

    // Imprimir valores intermedios para depuración (opcional)
    serial_print("--- Calculo Espesor ---\n", true);
    serial_print(" ADC A Final: ", false); serial_print(finalAdcValueA, true);
    serial_print(" ADC B Final: ", false); serial_print(finalAdcValueB, true);
    serial_print(" ADC A Home (Cal): ", false); serial_print(cal_AdcAHome, true);
    serial_print(" ADC B Home (Cal): ", false); serial_print(cal_AdcBHome, true);
    serial_print(" Delta ADC A: ", false); serial_print(deltaAdcA, true);
    serial_print(" Delta ADC B: ", false); serial_print(deltaAdcB, true);
    serial_print(" MM/ADC A (Cal): ", false); serial_print(cal_MmPerAdcA, 6, true);
    serial_print(" MM/ADC B (Cal): ", false); serial_print(cal_MmPerAdcB, 6, true);
    serial_print(" Recorrido A (mm): ", false); serial_print(travelA_mm, 3, true);
    serial_print(" Recorrido B (mm): ", false); serial_print(travelB_mm, 3, true);
    serial_print(" Gap Inicial (Cal) (mm): ", false); serial_print(cal_TotalGapMmHome, 3, true);
    serial_print(" Espesor Calculado (mm): ", false); serial_print(thickness, 3, true);
    serial_print("-----------------------\n", true);

    return thickness;
}

void calibration_load() {
    serial_print("Cargando calibracion...\n", true);
    byte flag = EEPROM.read(EEPROM_ADDR_CAL_VALID);

    if (flag == CAL_VALID_FLAG) {
        // Hay datos válidos guardados
        EEPROM.get(EEPROM_ADDR_ADC_A_HOME, cal_AdcAHome);
        EEPROM.get(EEPROM_ADDR_ADC_B_HOME, cal_AdcBHome);
        EEPROM.get(EEPROM_ADDR_MM_PER_ADC_A, cal_MmPerAdcA);
        EEPROM.get(EEPROM_ADDR_MM_PER_ADC_B, cal_MmPerAdcB);
        EEPROM.get(EEPROM_ADDR_TOTAL_GAP, cal_TotalGapMmHome);
        serial_print("Calibracion cargada desde EEPROM.\n", true);
    } else {
        // No hay datos válidos, usar defaults
        cal_AdcAHome = ADC_A_HOME_DEFAULT;
        cal_AdcBHome = ADC_B_HOME_DEFAULT;
        cal_MmPerAdcA = MM_PER_ADC_A_DEFAULT;
        cal_MmPerAdcB = MM_PER_ADC_B_DEFAULT;
        cal_TotalGapMmHome = TOTAL_GAP_MM_HOME_DEFAULT;
        serial_print("WARN: Usando calibracion por defecto. Se recomienda calibrar.\n", true);
        // Opcional: Forzar un estado de error o mostrar mensaje persistente en LCD
        // lcd_display("CALIBRACION", "NO VALIDA", "Usando defaults");
        // delay(3000);
    }
    // Imprimir los valores que se usarán
    serial_print(" Usando AdcAHome: ", false); serial_print(cal_AdcAHome, true);
    serial_print(" Usando AdcBHome: ", false); serial_print(cal_AdcBHome, true);
    serial_print(" Usando MmPerAdcA: ", false); serial_print(cal_MmPerAdcA, 6, true);
    serial_print(" Usando MmPerAdcB: ", false); serial_print(cal_MmPerAdcB, 6, true);
    serial_print(" Usando TotalGapMmHome: ", false); serial_print(cal_TotalGapMmHome, 3, true);
}

void calibration_save() {
    serial_print("Guardando calibracion en EEPROM...\n", true);
    EEPROM.put(EEPROM_ADDR_ADC_A_HOME, cal_AdcAHome);
    EEPROM.put(EEPROM_ADDR_ADC_B_HOME, cal_AdcBHome);
    EEPROM.put(EEPROM_ADDR_MM_PER_ADC_A, cal_MmPerAdcA);
    EEPROM.put(EEPROM_ADDR_MM_PER_ADC_B, cal_MmPerAdcB);
    EEPROM.put(EEPROM_ADDR_TOTAL_GAP, cal_TotalGapMmHome);

    // Marcar como válidos *después* de escribir todo
    EEPROM.write(EEPROM_ADDR_CAL_VALID, CAL_VALID_FLAG);

    // EEPROM.commit(); // Necesario en ESP32/ESP8266, no usualmente en AVR
    serial_print("Calibracion guardada.\n", true);
    lcd_display("Calibracion", "Guardada", "en EEPROM");
    delay(2000); // Mostrar mensaje
}

void calibration_enter_mode() {
     serial_print("Entrando en modo Calibracion (NO IMPLEMENTADO)...\n", true);
     lcd_display("Modo Calibracion", "(No implementado)", "Reiniciando...");
     delay(3000);
     // --- IMPLEMENTACIÓN FUTURA ---
     // - Guiar al usuario a través de los pasos (LCD/Serial)
     // - Leer entradas (botones?) para confirmar
     // - Mover servos (usando hardware_io)
     // - Leer ADC (usando hardware_io)
     // - Calcular nuevas constantes cal_*
     // - Llamar a calibration_save() si es exitoso
     // - Manejar errores/cancelación
     // ---------------------------

     // Por ahora, simplemente recargamos y volvemos a idle (simulado)
     // calibration_load(); // Para asegurar consistencia si algo se tocó
     // state_machine_set_state(STATE_IDLE); // Necesitaríamos una función para forzar estado
     serial_print("Modo calibracion finalizado (simulado).\n", true);
}