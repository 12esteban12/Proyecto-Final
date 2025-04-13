#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Arduino.h> // para int16_t, float

// --- Variables Globales de Calibración (Cargadas desde EEPROM/Defaults) ---
// Se declaran aquí como 'extern' para que otros módulos puedan verlas si es necesario,
// pero se definen en measurement.cpp
extern int16_t cal_AdcAHome;
extern int16_t cal_AdcBHome;
extern float cal_MmPerAdcA;
extern float cal_MmPerAdcB;
extern float cal_TotalGapMmHome;

// --- Variables Globales de Calibración Báscula ---
extern float cal_ScaleFactor; // Factor de escala (uds raw / gramo)
extern long cal_ScaleOffset;  // Offset (valor raw de tara)

// --- Variables de Resultado ---
extern float measuredThicknessMm; // Espesor medido (si aplica)
extern float measuredWeightGrams; // Peso medido en gramos

// --- Funciones de Cálculo ---

/**
 * @brief Calcula el espesor del vidrio en milímetros usando las lecturas ADC
 * finales y los parámetros de calibración cargados.
 * @param finalAdcValueA Lectura ADC del potenciómetro A en el momento del contacto.
 * @param finalAdcValueB Lectura ADC del potenciómetro B en el momento del contacto.
 * @return El espesor calculado en mm. Puede devolver valores negativos o irreales si la calibración es incorrecta.
 */
float calculate_thickness_adc(int16_t finalAdcValueA, int16_t finalAdcValueB);

// --- Funciones de Calibración ---

/**
 * @brief Carga los parámetros de calibración desde la EEPROM.
 * Si no hay datos válidos guardados (según CAL_VALID_FLAG) o es la primera vez,
 * carga los valores por defecto definidos en config.h en las variables globales cal_*.
 * Imprime en Serial los valores cargados/usados.
 */
void calibration_load();

/**
 * @brief Guarda los valores *actuales* de las variables globales cal_* en la EEPROM.
 * Marca los datos como válidos escribiendo CAL_VALID_FLAG.
 * Se debe llamar después de una rutina de calibración exitosa.
 */
void calibration_save();

/**
 * @brief Placeholder para iniciar el modo de calibración asistida.
 * (Requiere implementación detallada de la interacción con el usuario).
 */
void calibration_enter_mode();

/**
 * @brief Guía al usuario para calibrar el factor de escala de la báscula.
 * Requiere un peso conocido. Guarda el nuevo factor en EEPROM.
 * (Requiere implementación detallada).
 */
void scale_perform_calibration();

#endif // MEASUREMENT_H