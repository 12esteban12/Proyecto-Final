#include "state_machine.h"
#include "config.h"
#include "hardware_io.h"  // Para interactuar con hardware
#include "measurement.h"  // Para calcular espesor y manejar calibración

// --- Definición de Variables Globales de Estado ---
SensorState_t currentState = STATE_IDLE;
GlassType_t detectedGlassType = GLASS_TYPE_UNKNOWN;
float measuredThicknessMm = 0.0;
int16_t finalAdcValueA = 0;
int16_t finalAdcValueB = 0;

// Variables internas de la máquina de estados
namespace {
    unsigned long stateEntryTime = 0; // Momento en que se entró al estado actual
    unsigned long lastServoMoveTime = 0; // Para movimiento no bloqueante

    // Posiciones actuales (objetivo) de los servos
    int servoAPos = SERVO_A_HOME_POS;
    int servoBPos = SERVO_B_HOME_POS;

    // Banderas para el proceso de medición
    bool servoAMoving = false;
    bool servoBMoving = false;
}

// --- Implementación de Funciones ---

void state_machine_setup() {
    // Mover servos a HOME al iniciar (ya se hace en hardware_setup, pero podemos asegurarlo)
    servo_A_move(SERVO_A_HOME_POS);
    servo_B_move(SERVO_B_HOME_POS);
    servo_C_move(SERVO_C_HOME_POS);
    servoAPos = SERVO_A_HOME_POS;
    servoBPos = SERVO_B_HOME_POS;
    stateEntryTime = millis(); // Inicializar timer de estado
    serial_print("Maquina de estados inicializada.\n", true);
}

// Función auxiliar para cambiar de estado y resetear el timer
void state_machine_set_state(SensorState_t newState) {
    serial_print("Transicion de estado: ", false);
    // Podríamos imprimir los nombres de los estados aquí si mapeamos el enum a strings
    serial_print((int)currentState, false);
    serial_print(" -> ", false);
    serial_print((int)newState, true);

    currentState = newState;
    stateEntryTime = millis(); // Registrar tiempo de entrada al nuevo estado

    // Acciones de ENTRADA al nuevo estado (si son necesarias)
    switch (newState) {
        case STATE_IDLE:
            lcd_display("Sistema Listo", "Coloque el vidrio", "para medir...");
            detectedGlassType = GLASS_TYPE_UNKNOWN;
            measuredThicknessMm = 0.0;
            break;
        case STATE_MEASURING_RAW_START:
             // Resetear flags y posiciones antes de empezar a mover
            servoAMoving = false;
            servoBMoving = true; // Empezamos con B
            servoAPos = SERVO_A_HOME_POS; // Asegurar que partimos de Home lógico
            servoBPos = SERVO_B_HOME_POS;
            servo_A_move(servoAPos); // Mover físicamente a Home por si acaso
            servo_B_move(servoBPos);
            serial_print("Iniciando medicion Crudo: Moviendo Servo B\n", true);
            lcd_display("", "Detectado:", "Vidrio Crudo", "Midiendo...");
             // Pasar directamente a mover B
             state_machine_set_state(STATE_MEASURING_RAW_MOVE_B);
            break;
         case STATE_MEASURING_RAW_MOVE_A:
             serial_print("Iniciando movimiento Servo A\n", true);
             lastServoMoveTime = millis(); // Reset timer para A
             break;
         case STATE_MEASURING_RAW_MOVE_B:
             serial_print("Iniciando movimiento Servo B\n", true);
             lastServoMoveTime = millis(); // Reset timer para B
             break;
         case STATE_CALCULATING:
             serial_print("Calculando espesor...\n", true);
             lcd_display("Calculando...", "", "", "Espere...");
             break;
         case STATE_DISPLAYING_RESULT:
             // La lógica de qué mostrar está en el loop de este estado
             break;
         case STATE_WAITING_REMOVAL:
             // El mensaje se pone en DISPLAYING_RESULT o aquí si se prefiere
             break;
         case STATE_ERROR:
             // El mensaje se pone en el estado que transiciona a ERROR
             break;
        case STATE_CALIBRATION:
            calibration_enter_mode(); // Ejecutar la función de calibración (aún placeholder)
            // La función de calibración debería manejar sus propios sub-estados o lógica
            // y al final, llamar a state_machine_set_state(STATE_IDLE)
            state_machine_set_state(STATE_IDLE); // Volver a idle por ahora
            break;
        default:
            break; // Otros estados no necesitan acción de entrada específica ahora
    }
}


void state_machine_run() {
    unsigned long currentTime = millis();

    // --- Lectura de Sensores (los necesarios para el estado actual) ---
    // Los switches que inician el proceso se leen siempre en IDLE
    int crudo_debounced = read_switch_debounced(CRUDO_PIN);
    int dvh21_debounced = read_switch_debounced(DVH21_PIN);

    // --- Lógica del Estado Actual ---
    switch (currentState) {

        case STATE_IDLE:
            // Esperando que se coloque vidrio
            if (crudo_debounced == LOW || dvh21_debounced == LOW) {
                serial_print("Vidrio detectado!\n", true);
                state_machine_set_state(STATE_DEBOUNCING_START);
            }
            // Aquí se podría añadir lógica para entrar en calibración, ej:
            // if (read_switch_instant(PIN_BOTON_CALIBRAR) == LOW) {
            //     state_machine_set_state(STATE_CALIBRATION);
            // }
            break;

        case STATE_DEBOUNCING_START:
            // Espera para que el vidrio se asiente
            if (currentTime - stateEntryTime > GLASS_SETTLE_TIME) {
                // Re-verificar sensores *después* del tiempo de espera
                int crudo_now = read_switch_debounced(CRUDO_PIN); // Releer estado estable
                int dvh21_now = read_switch_debounced(DVH21_PIN);

                if (crudo_now == LOW && dvh21_now == HIGH) {
                    state_machine_set_state(STATE_MEASURING_RAW_START);
                } else if (dvh21_now == LOW) {
                    state_machine_set_state(STATE_MEASURING_DVH);
                } else {
                    serial_print("Vidrio retirado durante espera.\n", true);
                    state_machine_set_state(STATE_IDLE);
                }
            }
            break;

        // Los estados START ahora solo transicionan inmediatamente
        // case STATE_MEASURING_RAW_START: break; // Ya no se necesita código aquí

        case STATE_MEASURING_RAW_MOVE_B:
            if (servoBMoving) {
                int ssb_state = read_switch_instant(SSB_PIN); // Lectura directa
                if (ssb_state == HIGH) { // No presionado
                    if (currentTime - lastServoMoveTime > SERVO_MOVE_INTERVAL) {
                        servoBPos--;
                        if (servoBPos < 0) servoBPos = 0;
                        servo_B_move(servoBPos);
                        lastServoMoveTime = currentTime;
                        // serial_print("."); // Debug: indicate movement step
                    }
                    // Timeout check (opcional)
                } else { // Contacto SSB (LOW)
                    serial_print("\nContacto Servo B (SSB)\n", true);
                    finalAdcValueB = read_adc(POT_B_CHANNEL); // LEER ADC EN CONTACTO
                    serial_print(" ADC B Contacto: ", false); serial_print(finalAdcValueB, true);
                    servoBMoving = false; // Detener lógica de movimiento B
                    servoAMoving = true;  // Iniciar lógica de movimiento A
                    state_machine_set_state(STATE_MEASURING_RAW_MOVE_A); // Cambiar al estado de mover A
                }
            }
            break;

        case STATE_MEASURING_RAW_MOVE_A:
             if (servoAMoving) {
                int ssa_state = read_switch_instant(SSA_PIN); // Lectura directa
                if (ssa_state == HIGH) { // No presionado
                    if (currentTime - lastServoMoveTime > SERVO_MOVE_INTERVAL) {
                        servoAPos++;
                        if (servoAPos > 180) servoAPos = 180;
                        servo_A_move(servoAPos);
                        lastServoMoveTime = currentTime;
                        // serial_print("+"); // Debug: indicate movement step
                    }
                    // Timeout check (opcional)
                } else { // Contacto SSA (LOW)
                    serial_print("\nContacto Servo A (SSA)\n", true);
                    finalAdcValueA = read_adc(POT_A_CHANNEL); // LEER ADC EN CONTACTO
                    serial_print(" ADC A Contacto: ", false); serial_print(finalAdcValueA, true);
                    servoAMoving = false; // Detener lógica de movimiento A
                    state_machine_set_state(STATE_CALCULATING); // Cambiar al estado de cálculo
                }
            }
            break;

        case STATE_CALCULATING:
            // El cálculo se hace aquí, usando las lecturas guardadas
            measuredThicknessMm = calculate_thickness_adc(finalAdcValueA, finalAdcValueB);
            detectedGlassType = GLASS_TYPE_RAW; // Asignar tipo

            // Validación básica del resultado (ejemplo)
            if (measuredThicknessMm < 0.5 || measuredThicknessMm > 15.0) { // Rango irreal para vidrio crudo
                 serial_print("WARN: Espesor calculado fuera de rango: ", false);
                 serial_print(measuredThicknessMm, 2, true);
                 // Considerar ir a estado de ERROR o mostrar advertencia
            }

            // Mover servos de vuelta a HOME (bloqueante simple por ahora)
            serial_print("Retornando servos a HOME...\n", true);
            servo_A_move(SERVO_A_HOME_POS);
            servo_B_move(SERVO_B_HOME_POS);
            servoAPos = SERVO_A_HOME_POS; // Actualizar posición lógica
            servoBPos = SERVO_B_HOME_POS;
            delay(500); // Espera simple para que lleguen

            state_machine_set_state(STATE_DISPLAYING_RESULT);
            break;

        case STATE_MEASURING_DVH:
            { // Alcance local para leer sensores DVH
                int dvh28_state = read_switch_debounced(DVH28_PIN);
                int posicion_state = read_switch_debounced(POSICION_PIN);

                if (posicion_state == LOW) { // Bien posicionado
                    if (dvh28_state == LOW) {
                        detectedGlassType = GLASS_TYPE_DVH28;
                        serial_print("Tipo Detectado: DVH 28mm\n", true);
                    } else {
                        detectedGlassType = GLASS_TYPE_DVH21;
                        serial_print("Tipo Detectado: DVH 21mm\n", true);
                    }
                } else { // Mal posicionado
                    detectedGlassType = GLASS_TYPE_MISPLACED;
                    serial_print("Error: Vidrio DVH mal colocado\n", true);
                }
                state_machine_set_state(STATE_DISPLAYING_RESULT);
            }
            break;

        case STATE_DISPLAYING_RESULT:
            // Mostrar resultado (se ejecuta solo una vez gracias a state_machine_set_state)
            // O se podría poner un flag para mostrar solo una vez si se permanece en el estado
            {
                char line2_buf[LCD_COLS + 1];
                char line3_buf[LCD_COLS + 1];
                switch (detectedGlassType) {
                    case GLASS_TYPE_RAW:
                        snprintf(line2_buf, sizeof(line2_buf), "Tipo: Vidrio Crudo");
                        snprintf(line3_buf, sizeof(line3_buf), "Espesor: %.2f mm", measuredThicknessMm);
                        lcd_display("Medicion Completa", line2_buf, line3_buf, "Retire el vidrio");
                        break;
                    case GLASS_TYPE_DVH21:
                        lcd_display("Medicion Completa", "Tipo: DVH 21mm", "", "Retire el vidrio");
                        break;
                    case GLASS_TYPE_DVH28:
                        lcd_display("Medicion Completa", "Tipo: DVH 28mm", "", "Retire el vidrio");
                        break;
                    case GLASS_TYPE_MISPLACED:
                        lcd_display("!! ERROR !!", "Vidrio mal colocado", "Retire y reintente");
                        state_machine_set_state(STATE_ERROR); // Ir a estado de error
                        return; // Salir para no pasar a WAITING_REMOVAL inmediatamente
                    default: // Incluye GLASS_TYPE_UNKNOWN
                        lcd_display("!! ERROR !!", "Tipo desconocido", "Contacte soporte");
                        state_machine_set_state(STATE_ERROR); // Ir a estado de error
                        return; // Salir
                }
            }
            // Si no fue un error, esperar a que retiren el vidrio
            state_machine_set_state(STATE_WAITING_REMOVAL);
            break;

        case STATE_WAITING_REMOVAL:
            // Esperar a que ambos sensores iniciales estén libres (HIGH)
            if (crudo_debounced == HIGH && dvh21_debounced == HIGH) {
                serial_print("Vidrio retirado.\n", true);
                state_machine_set_state(STATE_IDLE); // Volver al inicio
            }
            break;

        case STATE_ERROR:
            // Permanecer en este estado mostrando el mensaje de error (ya puesto)
            // Esperar a que se retire el vidrio para volver a IDLE
             if (crudo_debounced == HIGH && dvh21_debounced == HIGH) {
                serial_print("Vidrio retirado despues de error.\n", true);
                state_machine_set_state(STATE_IDLE); // Volver al inicio
            }
            break;

        case STATE_CALIBRATION:
             // La lógica de calibración se manejaría aquí o en calibration_enter_mode()
             // Por ahora, ya transicionó de vuelta a IDLE en la función set_state
             break;

        default:
            // Estado desconocido, intentar recuperar volviendo a IDLE
            serial_print("Error: Estado desconocido - ", false);
            serial_print((int)currentState, true);
            state_machine_set_state(STATE_IDLE);
            break;
    }
}