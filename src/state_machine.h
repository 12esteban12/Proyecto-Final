#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h> // Para tipos estándar

// --- Definiciones de Estados ---
typedef enum {
    STATE_IDLE,                 // Esperando vidrio
    STATE_DEBOUNCING_START,     // Espera corta tras detección inicial
    STATE_GLASS_PRESENT,        // (Podría fusionarse con DEBOUNCING o el siguiente estado)
    STATE_MEASURING_RAW_START,  // Iniciando secuencia medición crudo
    STATE_MEASURING_RAW_MOVE_B, // Moviendo servo B hacia adentro
    STATE_MEASURING_RAW_MOVE_A, // Moviendo servo A hacia adentro
    STATE_CALCULATING,          // Cálculo de espesor tras contacto
    STATE_MEASURING_DVH,        // Determinando tipo DVH (21/28/Mal puesto)
    STATE_DISPLAYING_RESULT,    // Mostrando resultado en LCD/Serial
    STATE_WAITING_REMOVAL,      // Esperando que se retire el vidrio medido
    STATE_ERROR,                // Estado de error (ej. vidrio mal puesto, fallo hardware)
    STATE_CALIBRATION           // Estado (o conjunto de estados) para la calibración
} SensorState_t;

// --- Definiciones de Tipos de Vidrio ---
typedef enum {
    GLASS_TYPE_UNKNOWN,
    GLASS_TYPE_RAW,
    GLASS_TYPE_DVH21,
    GLASS_TYPE_DVH28,
    GLASS_TYPE_MISPLACED
} GlassType_t;


// --- Variables Globales de Estado (Declaradas extern) ---
extern SensorState_t currentState;
extern GlassType_t detectedGlassType;
extern float measuredThicknessMm;
extern int16_t finalAdcValueA; // Guardamos la lectura ADC de A en contacto
extern int16_t finalAdcValueB; // Guardamos la lectura ADC de B en contacto
extern float measuredWeightGrams; // Peso medido en gramos 

// --- Funciones Principales de la Máquina de Estados ---

/**
 * @brief Inicializa la máquina de estados (si es necesario más allá de la definición global).
 * Se llama una vez desde setup() después de hardware_setup() y calibration_load().
 */
void state_machine_setup();

/**
 * @brief Ejecuta la lógica del estado actual de la máquina.
 * Debe llamarse repetidamente en el loop() principal.
 * Lee sensores, controla actuadores y cambia de estado según la lógica definida.
 */
void state_machine_run();

/**
 * @brief Permite cambiar el estado actual de la máquina desde fuera (usar con cuidado).
 * @param newState El nuevo estado al que se debe transicionar.
 */
void state_machine_set_state(SensorState_t newState);


#endif // STATE_MACHINE_H