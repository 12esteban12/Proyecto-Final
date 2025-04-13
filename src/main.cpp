#include <Arduino.h>
#include "config.h"         // Configuraciones globales
#include "hardware_io.h"    // Funciones de hardware
#include "measurement.h"    // Funciones de cálculo y calibración
#include "state_machine.h"  // Lógica de la máquina de estados

void setup() {
  // 1. Inicializar Hardware básico (Serial, I2C, Pines, LCD, ADC, Servos)
  if (!hardware_setup()) {
    // Si falla la inicialización crítica (ej. ADC), detener
    serial_print("Fallo critico en inicializacion de hardware. Deteniendo.\n", true);
    while(1) { delay(1000); } // Bucle infinito
  }

  // 2. Cargar Parámetros de Calibración (desde EEPROM o usar defaults)
  calibration_load();

  // 3. Inicializar la Máquina de Estados (poner servos en home, etc.)
  state_machine_setup(); // Asegura estado inicial IDLE y servos en HOME

  // 4. Mensaje de Sistema Listo
  serial_print("\n*** Sistema de Medicion de Espesor Listo ***\n", true);
  // El LCD ya muestra "Sistema Listo" desde state_machine_set_state(STATE_IDLE)
}

void loop() {
  // Ejecutar la lógica del estado actual de la máquina de estados
  state_machine_run();

  // El loop principal es muy simple, toda la lógica está en state_machine_run()
  // Se podría añadir aquí un pequeño delay si se detectan problemas de WDT
  // o si el loop corre demasiado rápido sin necesidad, pero con millis()
  // generalmente no es necesario.
  // delay(1); // Descomentar si es absolutamente necesario
}