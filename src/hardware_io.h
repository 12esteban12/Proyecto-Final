#ifndef HARDWARE_IO_H
#define HARDWARE_IO_H

#include <Arduino.h> // Para uint8_t, int16_t, etc.

// --- Funciones de Inicialización ---

/**
 * @brief Inicializa todos los componentes de hardware (pines, I2C, Serial, LCD, ADC, Servos).
 * Debe llamarse una vez en setup().
 * @return true si la inicialización fue exitosa (ej. ADC encontrado), false si hubo un error crítico.
 */
bool hardware_setup();

// --- Funciones de Servos ---

/**
 * @brief Mueve el Servo A a la posición angular especificada.
 * @param position Ángulo deseado (normalmente 0-180).
 */
void servo_A_move(int position);

/**
 * @brief Mueve el Servo B a la posición angular especificada.
 * @param position Ángulo deseado (normalmente 0-180).
 */
void servo_B_move(int position);

/**
 * @brief Mueve el Servo C a la posición angular especificada.
 * @param position Ángulo deseado (normalmente 0-180).
 */
void servo_C_move(int position);

// --- Funciones de Sensores/Entradas ---

/**
 * @brief Lee el valor del ADC en el canal especificado.
 * @param channel Canal del ADS1115 (0-3).
 * @return La lectura raw del ADC (int16_t), o un valor indicativo de error (ej. 0 o -1 si se prefiere, revisar implementación).
 */
int16_t read_adc(uint8_t channel);

/**
 * @brief Lee el estado de un pin de switch de forma instantánea (sin anti-rebote).
 * Útil para finales de carrera durante el movimiento.
 * @param pin El número de pin Arduino a leer (debe estar configurado como INPUT_PULLUP).
 * @return HIGH o LOW.
 */
int read_switch_instant(uint8_t pin);

/**
 * @brief Lee el estado de un pin de switch con lógica anti-rebote (debouncing).
 * Mantiene el estado internamente para cada pin soportado.
 * @param pin El número de pin Arduino a leer (CRUDO_PIN, DVH21_PIN, etc.).
 * @return El estado estable del switch (HIGH o LOW) después del debounce.
 */
int read_switch_debounced(uint8_t pin);


// --- Funciones de Báscula (HX711) ---

/**
 * @brief Configura la báscula HX711 con valores de calibración iniciales.
 * No realiza la inicialización del hardware HX711 (eso ocurre en hardware_setup).
 * @param offset Valor de offset (tara) leído de EEPROM o default.
 * @param factor Factor de escala leído de EEPROM o default.
 */
void scale_set_calibration(long offset, float factor);

/**
 * @brief Realiza la tara de la báscula, calculando el offset actual.
 * @param times Número de lecturas a promediar para la tara (más es más estable).
 * @return El valor raw del offset calculado.
 */
long scale_tare(int times = 10);

/**
 * @brief Obtiene el peso actual de la báscula en gramos.
 * Utiliza el factor de escala y offset configurados.
 * Es una función BLOQUEANTE mientras lee.
 * @param times Número de lecturas a promediar para obtener el peso.
 * @return El peso calculado en gramos. Puede devolver 0 si la báscula no está lista.
 */
float scale_get_weight_grams(int times = 1);

/**
 * @brief Obtiene la lectura raw promedio del HX711.
 * Útil para la calibración.
 * @param times Número de lecturas a promediar.
 * @return El valor raw promedio.
 */
long scale_get_raw_reading(int times = 10);

/**
 * @brief Verifica si el chip HX711 está listo para enviar datos.
 * @return true si está listo, false si no.
 */
bool scale_is_ready();

// --- Funciones de Salida (LCD) ---

/**
 * @brief Muestra mensajes en las 4 líneas del LCD.
 * Limpia la pantalla antes de escribir. Pasa nullptr para no escribir en una línea.
 * @param line1 Texto para la línea 1 (o nullptr).
 * @param line2 Texto para la línea 2 (o nullptr).
 * @param line3 Texto para la línea 3 (o nullptr).
 * @param line4 Texto para la línea 4 (o nullptr).
 */
void lcd_display(const char* line1 = nullptr, const char* line2 = nullptr, const char* line3 = nullptr, const char* line4 = nullptr);

/**
 * @brief Limpia la pantalla LCD.
 */
void lcd_clear();


// --- Comunicación Serial ---

/**
 * @brief Envía un mensaje a través del puerto Serie.
 * @param message Mensaje a enviar.
 * @param newline true para añadir un salto de línea al final, false para no añadirlo.
 */
void serial_print(const char* message, bool newline = false);
void serial_print(int value, bool newline = false);
void serial_print(float value, int decimals = 2, bool newline = false);


#endif // HARDWARE_IO_H