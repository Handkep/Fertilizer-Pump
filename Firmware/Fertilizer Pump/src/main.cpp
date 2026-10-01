#include <Arduino.h>
#include <AccelStepper.h>
#include <TMCStepper.h>
#include <SoftwareSerial.h>

// --------------------------------------------------
// Zentrale Konfiguration
// Alle Startwerte werden ausschliesslich hier eingestellt.
// --------------------------------------------------

// Serielle Konsole
#define SERIAL_BAUD                       115200UL
#define SERIAL_BUFFER_CAPACITY            64U

// Pins
#define STEP_PIN D2
#define DIR_PIN  D1
#define TMC_RX_PIN D6
#define TMC_TX_PIN D5

// Motor und Bewegung
#define MOTOR_STEP_ANGLE_DEG               1.8f
#define DEFAULT_MICROSTEPS                 8U
#define DEFAULT_MAX_SPEED_RPM              100.0f
#define DEFAULT_ACCELERATION_RPM_PER_S      500.0f
#define DEFAULT_SHOWCASE_RPM               500.0f
#define STEP_MIN_PULSE_US                  2U
#define DEFAULT_DIRECTION_INVERTED         false
#define DEFAULT_STEP_INVERTED              false
#define DEFAULT_ENABLE_INVERTED            false

// TMC2209 UART und Hardware
#define TMC_UART_BAUD                      19200UL
#define DRIVER_ADDRESS                     0b00
#define R_SENSE                            0.11f

// TMC2209 Motorstrom
#define DEFAULT_MOTOR_CURRENT_MA           900U
#define DEFAULT_HOLD_CURRENT_MULTIPLIER    0.30f
#define MIN_MOTOR_CURRENT_MA               100L
#define MAX_MOTOR_CURRENT_MA               2000L

// TMC2209 Chopper und Stromabsenkung
#define DEFAULT_TOFF                       4U
#define DEFAULT_IHOLDDELAY                 8U
#define DEFAULT_TPOWERDOWN                 20U
#define DEFAULT_PDN_UART_ENABLED           true
#define DEFAULT_REGISTER_MICROSTEPS        true
#define DEFAULT_INTERNAL_CURRENT_REFERENCE true
#define DEFAULT_INTERPOLATION              true
#define DEFAULT_SPREADCYCLE                true
#define DEFAULT_PWM_AUTOSCALE              true

// StallGuard; SGTHRS 0 deaktiviert praktisch die Empfindlichkeit.
#define DEFAULT_STALL_THRESHOLD            0U
#define DEFAULT_TCOOLTHRS                  0xFFFFFUL

// Kalibrierung ist nach einem Neustart absichtlich ungesetzt.
#define DEFAULT_STEPS_PER_ML               0.0f


static_assert(DRIVER_ADDRESS <= 3, "DRIVER_ADDRESS muss zwischen 0 und 3 liegen");
static_assert(DEFAULT_HOLD_CURRENT_MULTIPLIER >= 0.0f &&
              DEFAULT_HOLD_CURRENT_MULTIPLIER <= 1.0f,
              "DEFAULT_HOLD_CURRENT_MULTIPLIER muss zwischen 0.0 und 1.0 liegen");
static_assert(DEFAULT_MOTOR_CURRENT_MA >= MIN_MOTOR_CURRENT_MA &&
              DEFAULT_MOTOR_CURRENT_MA <= MAX_MOTOR_CURRENT_MA,
              "DEFAULT_MOTOR_CURRENT_MA liegt ausserhalb der erlaubten Grenzen");
static_assert(DEFAULT_MAX_SPEED_RPM > 0.0f,
              "DEFAULT_MAX_SPEED_RPM muss groesser als 0 sein");
static_assert(DEFAULT_ACCELERATION_RPM_PER_S > 0.0f,
              "DEFAULT_ACCELERATION_RPM_PER_S muss groesser als 0 sein");
static_assert(DEFAULT_SHOWCASE_RPM > 0.0f,
              "DEFAULT_SHOWCASE_RPM muss groesser als 0 sein");
static_assert(MOTOR_STEP_ANGLE_DEG > 0.0f && MOTOR_STEP_ANGLE_DEG <= 360.0f,
              "MOTOR_STEP_ANGLE_DEG muss zwischen 0 und 360 Grad liegen");
static_assert(DEFAULT_TOFF <= 15, "DEFAULT_TOFF muss zwischen 0 und 15 liegen");
static_assert(DEFAULT_IHOLDDELAY <= 15,
              "DEFAULT_IHOLDDELAY muss zwischen 0 und 15 liegen");
static_assert(DEFAULT_TPOWERDOWN <= 255,
              "DEFAULT_TPOWERDOWN muss zwischen 0 und 255 liegen");
static_assert(DEFAULT_STALL_THRESHOLD <= 255,
              "DEFAULT_STALL_THRESHOLD muss zwischen 0 und 255 liegen");
static_assert(DEFAULT_TCOOLTHRS <= 0xFFFFF,
              "DEFAULT_TCOOLTHRS muss ein 20-Bit-Wert sein");
static_assert(DEFAULT_MICROSTEPS == 1 || DEFAULT_MICROSTEPS == 2 ||
              DEFAULT_MICROSTEPS == 4 || DEFAULT_MICROSTEPS == 8 ||
              DEFAULT_MICROSTEPS == 16 || DEFAULT_MICROSTEPS == 32 ||
              DEFAULT_MICROSTEPS == 64 || DEFAULT_MICROSTEPS == 128 ||
              DEFAULT_MICROSTEPS == 256,
              "DEFAULT_MICROSTEPS ist ungueltig");


// --------------------------------------------------
// Stepper
// --------------------------------------------------

SoftwareSerial tmcSerial(TMC_RX_PIN, TMC_TX_PIN);


AccelStepper motor(
    AccelStepper::DRIVER,
    STEP_PIN,
    DIR_PIN
);


TMC2209Stepper driver(
    &tmcSerial,
    R_SENSE,
    DRIVER_ADDRESS
);

// --------------------------------------------------
// Einstellungen
// --------------------------------------------------

uint16_t microstepsSetting = DEFAULT_MICROSTEPS;
uint16_t motorCurrentMaSetting = DEFAULT_MOTOR_CURRENT_MA;
uint8_t stallThresholdSetting = DEFAULT_STALL_THRESHOLD;
bool spreadCycleSetting = DEFAULT_SPREADCYCLE;

// Wird durch CAL gesetzt
float stepsPerMl = DEFAULT_STEPS_PER_ML;

// Letzte gefahrene Schrittzahl für Kalibrierung
long lastMoveSteps = 0;

// Serial-Eingabepuffer
String serialBuffer = "";


// --------------------------------------------------
// Showcase
// --------------------------------------------------

bool showcaseMode = false;

// Laufzeitwerte starten mit den zentral definierten Standardwerten und
// können danach über die seriellen Befehle geändert werden.
float maxSpeedRpm = DEFAULT_MAX_SPEED_RPM;
float accelerationRpmPerSecond = DEFAULT_ACCELERATION_RPM_PER_S;
float showcaseRpm = DEFAULT_SHOWCASE_RPM;


long fullStepsPerRevolution()
{
    return lroundf(360.0f / MOTOR_STEP_ANGLE_DEG);
}


long stepsPerRevolution()
{
    return fullStepsPerRevolution() * (long)microstepsSetting;
}


float rpmToStepsPerSecond(float rpm)
{
    return rpm * stepsPerRevolution() / 60.0f;
}


float stepsPerSecondToRpm(float stepsPerSecond)
{
    return stepsPerSecond * 60.0f / stepsPerRevolution();
}


bool isValidMicrostepSetting(uint16_t microsteps)
{
    return microsteps == 1 || microsteps == 2 || microsteps == 4 ||
           microsteps == 8 || microsteps == 16 || microsteps == 32 ||
           microsteps == 64 || microsteps == 128 || microsteps == 256;
}


bool motorIsRunning()
{
    return showcaseMode || motor.distanceToGo() != 0;
}


// Schreibt immer den vollständigen gewünschten Zustand. Dadurch werden
// auch Register wiederhergestellt, die der TMC2209 bei einem Reset verliert.
uint8_t applyAllDriverSettings()
{
    driver.pdn_disable(DEFAULT_PDN_UART_ENABLED);
    driver.mstep_reg_select(DEFAULT_REGISTER_MICROSTEPS);
    driver.I_scale_analog(!DEFAULT_INTERNAL_CURRENT_REFERENCE);

    driver.toff(DEFAULT_TOFF);
    driver.rms_current(motorCurrentMaSetting,
                       DEFAULT_HOLD_CURRENT_MULTIPLIER);
    driver.iholddelay(DEFAULT_IHOLDDELAY);
    driver.TPOWERDOWN(DEFAULT_TPOWERDOWN);

    driver.microsteps(microstepsSetting);
    driver.intpol(DEFAULT_INTERPOLATION);

    driver.en_spreadCycle(spreadCycleSetting);
    driver.pwm_autoscale(DEFAULT_PWM_AUTOSCALE);

    driver.SGTHRS(stallThresholdSetting);
    driver.TCOOLTHRS(DEFAULT_TCOOLTHRS);

    return driver.test_connection();
}


bool restoreDriverSettings()
{
    uint8_t result = applyAllDriverSettings();

    if (result == 0)
    {
        return true;
    }

    Serial.print("TMC2209 nicht erreichbar, Aktion abgebrochen (Fehler ");
    Serial.print(result);
    Serial.println(")");
    return false;
}


// --------------------------------------------------
// Bewegung starten
// --------------------------------------------------

void moveSteps(long steps)
{
    // Vor jeder Bewegung alle flüchtigen TMC2209-Register neu schreiben.
    if (!restoreDriverSettings())
    {
        return;
    }

    // Showcase beenden
    showcaseMode = false;

    lastMoveSteps = abs(steps);

    motor.move(steps);

    Serial.print("Fahre ");
    Serial.print(steps);
    Serial.println(" Steps");
}


// --------------------------------------------------
// Befehl verarbeiten
// --------------------------------------------------

void processCommand(String command)
{
    command.trim();

    // ==================================================
    // REV <Umdrehungen>
    // ==================================================

    if (command.startsWith("REV "))
    {
        float rev = command.substring(4).toFloat();

        long steps = round(rev * stepsPerRevolution());

        moveSteps(steps);
    }


    // ==================================================
    // MOVE <Steps>
    // ==================================================

    else if (command.startsWith("MOVE "))
    {
        long steps = command.substring(5).toInt();

        moveSteps(steps);
    }


    // ==================================================
    // CAL <gemessene ml>
    // ==================================================

    else if (command.startsWith("CAL "))
    {
        if (motorIsRunning())
        {
            Serial.println("Kalibrierung nur bei stillstehendem Motor speichern");
            return;
        }

        float ml = command.substring(4).toFloat();

        if (ml <= 0)
        {
            Serial.println("Ungueltiger Messwert");
            return;
        }

        if (lastMoveSteps == 0)
        {
            Serial.println("Noch keine Kalibrierfahrt gemacht");
            return;
        }

        stepsPerMl = (float)lastMoveSteps / ml;

        if (!restoreDriverSettings())
        {
            return;
        }

        Serial.println();
        Serial.println("Kalibrierung gespeichert:");

        Serial.print("  Steps: ");
        Serial.println(lastMoveSteps);

        Serial.print("  Volumen: ");
        Serial.print(ml, 3);
        Serial.println(" ml");

        Serial.print("  Steps/ml: ");
        Serial.println(stepsPerMl, 3);

        Serial.print("  ml/Umdrehung: ");
        Serial.println(stepsPerRevolution() / stepsPerMl, 3);

        Serial.println();
    }


    // ==================================================
    // DOSE <ml>
    // ==================================================

    else if (command.startsWith("DOSE "))
    {
        if (stepsPerMl <= 0)
        {
            Serial.println("Zuerst kalibrieren!");
            return;
        }

        float ml = command.substring(5).toFloat();

        if (ml <= 0)
        {
            Serial.println("Ungueltige Menge");
            return;
        }

        long steps = round(ml * stepsPerMl);

        Serial.print("Dosiere ");
        Serial.print(ml, 3);
        Serial.print(" ml -> ");
        Serial.print(steps);
        Serial.println(" Steps");

        moveSteps(steps*-1);
    }


    // ==================================================
    // SPEED <rpm>
    // ==================================================

    else if (command.startsWith("SPEED "))
    {
        if (motorIsRunning())
        {
            Serial.println("Drehzahl nur bei stillstehendem Motor aendern");
            return;
        }

        float rpm = command.substring(6).toFloat();

        if (rpm <= 0)
        {
            Serial.println("Drehzahl muss > 0 RPM sein");
            return;
        }

        maxSpeedRpm = rpm;
        motor.setMaxSpeed(rpmToStepsPerSecond(maxSpeedRpm));

        if (!restoreDriverSettings())
        {
            return;
        }

        Serial.print("Maximale Drehzahl: ");
        Serial.print(maxSpeedRpm, 3);
        Serial.println(" RPM");
    }


    // ==================================================
    // ACCEL <rpm/s^2>
    // ==================================================

    else if (command.startsWith("ACCEL "))
    {
        if (motorIsRunning())
        {
            Serial.println("Beschleunigung nur bei stillstehendem Motor aendern");
            return;
        }

        float acceleration = command.substring(6).toFloat();

        if (acceleration <= 0)
        {
            Serial.println("Beschleunigung muss > 0 RPM/s^2 sein");
            return;
        }

        accelerationRpmPerSecond = acceleration;
        motor.setAcceleration(rpmToStepsPerSecond(accelerationRpmPerSecond));

        if (!restoreDriverSettings())
        {
            return;
        }

        Serial.print("Beschleunigung: ");
        Serial.print(accelerationRpmPerSecond, 3);
        Serial.println(" RPM/s^2");
    }


    // ==================================================
    // CURRENT <mA RMS>
    // ==================================================

    else if (command.startsWith("CURRENT "))
    {
        if (motorIsRunning())
        {
            Serial.println("Motorstrom nur bei stillstehendem Motor aendern");
            return;
        }

        long current = command.substring(8).toInt();

        if (current < MIN_MOTOR_CURRENT_MA || current > MAX_MOTOR_CURRENT_MA)
        {
            Serial.print("Motorstrom muss zwischen ");
            Serial.print(MIN_MOTOR_CURRENT_MA);
            Serial.print(" und ");
            Serial.print(MAX_MOTOR_CURRENT_MA);
            Serial.println(" mA RMS liegen");
            return;
        }

        motorCurrentMaSetting = (uint16_t)current;

        if (!restoreDriverSettings())
        {
            return;
        }

        Serial.print("Motorstrom: ");
        Serial.print(driver.rms_current());
        Serial.print(" mA RMS, Haltestrom ");
        Serial.print(DEFAULT_HOLD_CURRENT_MULTIPLIER * 100.0f, 0);
        Serial.println(" %");
    }


    // ==================================================
    // MICROSTEPS <1|2|4|8|16|32|64|128|256>
    // ==================================================

    else if (command.startsWith("MICROSTEPS "))
    {
        if (motorIsRunning())
        {
            Serial.println("Microsteps nur bei stillstehendem Motor aendern");
            return;
        }

        uint16_t microsteps = (uint16_t)command.substring(11).toInt();

        if (!isValidMicrostepSetting(microsteps))
        {
            Serial.println("Erlaubt: 1, 2, 4, 8, 16, 32, 64, 128 oder 256");
            return;
        }

        microstepsSetting = microsteps;

        // Eine Kalibrierung mit anderer Schrittweite ist sofort ungueltig,
        // auch wenn die anschliessende UART-Uebertragung fehlschlaegt.
        stepsPerMl = DEFAULT_STEPS_PER_ML;

        // Mechanische Drehzahl und Beschleunigung trotz geänderter
        // Schrittweite konstant halten.
        motor.setMaxSpeed(rpmToStepsPerSecond(maxSpeedRpm));
        motor.setAcceleration(rpmToStepsPerSecond(accelerationRpmPerSecond));

        if (!restoreDriverSettings())
        {
            return;
        }

        Serial.print("Microsteps: ");
        Serial.println(microstepsSetting);
        Serial.print("Steps/Umdrehung: ");
        Serial.println(stepsPerRevolution());
        Serial.println("Kalibrierung wurde zurueckgesetzt");
    }


    // ==================================================
    // MODE <STEALTH|SPREAD>
    // ==================================================

    else if (command == "MODE STEALTH")
    {
        if (motorIsRunning())
        {
            Serial.println("Treiber-Modus nur bei stillstehendem Motor aendern");
            return;
        }

        spreadCycleSetting = false;

        if (!restoreDriverSettings())
        {
            return;
        }

        Serial.println("Modus: StealthChop (leise)");
    }

    else if (command == "MODE SPREAD")
    {
        if (motorIsRunning())
        {
            Serial.println("Treiber-Modus nur bei stillstehendem Motor aendern");
            return;
        }

        spreadCycleSetting = true;

        if (!restoreDriverSettings())
        {
            return;
        }

        Serial.println("Modus: SpreadCycle (mehr Drehmoment)");
    }


    // ==================================================
    // STALL <0..255>
    // ==================================================

    else if (command.startsWith("STALL "))
    {
        if (motorIsRunning())
        {
            Serial.println("StallGuard nur bei stillstehendem Motor einstellen");
            return;
        }

        long threshold = command.substring(6).toInt();

        if (threshold < 0 || threshold > 255)
        {
            Serial.println("StallGuard-Schwellwert muss zwischen 0 und 255 liegen");
            return;
        }

        stallThresholdSetting = (uint8_t)threshold;

        if (!restoreDriverSettings())
        {
            return;
        }

        Serial.print("StallGuard-Schwellwert: ");
        Serial.println(threshold);
        Serial.println("Fuer StallGuard MODE SPREAD verwenden");
    }


    // ==================================================
    // SHOWCASE [rpm]
    //
    // SHOWCASE
    // SHOWCASE 20
    // SHOWCASE -20
    // ==================================================

    else if (command.startsWith("SHOWCASE"))
    {
        // Optional eine Geschwindigkeit mitgeben
        if (command.length() > 8)
        {
            float requestedRpm = command.substring(9).toFloat();

            if (requestedRpm != 0)
            {
                showcaseRpm = requestedRpm;
            }
        }

        // MaxSpeed muss groß genug für runSpeed() sein
        if (abs(showcaseRpm) > maxSpeedRpm)
        {
            maxSpeedRpm = abs(showcaseRpm);
            motor.setMaxSpeed(rpmToStepsPerSecond(maxSpeedRpm));
        }

        // Direkt vor dem Start alle Treiberregister wiederherstellen.
        if (!restoreDriverSettings())
        {
            return;
        }

        showcaseMode = true;

        motor.setSpeed(rpmToStepsPerSecond(showcaseRpm));

        Serial.print("Showcase gestartet mit ");
        Serial.print(showcaseRpm, 3);
        Serial.println(" RPM");

        Serial.println("Mit STOP beenden");
    }


    // ==================================================
    // STOP
    // ==================================================

    else if (command == "STOP")
    {
        if (showcaseMode)
        {
            // Showcase sofort stoppen
            showcaseMode = false;
            motor.setSpeed(0);

            Serial.println("Showcase gestoppt");
        }
        else
        {
            // Normale Bewegung kontrolliert abbremsen
            motor.stop();

            Serial.println("Stop / Abbremsen");
        }
    }


    // ==================================================
    // STATUS
    // ==================================================

    else if (command == "STATUS")
    {
        Serial.println();
        Serial.println("----- STATUS -----");

        Serial.print("Position: ");
        Serial.println(motor.currentPosition());

        Serial.print("Zielposition: ");
        Serial.println(motor.targetPosition());

        Serial.print("Restweg: ");
        Serial.println(motor.distanceToGo());

        Serial.print("Geschwindigkeit: ");
        Serial.print(stepsPerSecondToRpm(motor.speed()), 3);
        Serial.println(" RPM");

        Serial.print("Maximale Drehzahl: ");
        Serial.print(maxSpeedRpm, 3);
        Serial.println(" RPM");

        Serial.print("Beschleunigung: ");
        Serial.print(accelerationRpmPerSecond, 3);
        Serial.println(" RPM/s^2");

        Serial.print("Steps/ml: ");

        if (stepsPerMl > 0)
        {
            Serial.println(stepsPerMl, 3);
        }
        else
        {
            Serial.println("nicht kalibriert");
        }

        Serial.print("Showcase: ");
        Serial.println(showcaseMode ? "AN" : "AUS");

        Serial.println();
        Serial.println("----- TMC2209 -----");

        Serial.print("Chip-Version: 0x");
        Serial.println(driver.version(), HEX);

        Serial.print("UART IFCNT: ");
        Serial.println(driver.IFCNT());

        Serial.print("Motorstrom: ");
        Serial.print(driver.rms_current());
        Serial.println(" mA RMS");

        Serial.print("Haltestrom: ");
        Serial.print(DEFAULT_HOLD_CURRENT_MULTIPLIER * 100.0f, 0);
        Serial.print(" % (ca. ");
        Serial.print(driver.rms_current() * DEFAULT_HOLD_CURRENT_MULTIPLIER, 0);
        Serial.println(" mA RMS)");

        Serial.print("Microsteps: ");
        Serial.println(driver.microsteps());

        Serial.print("Vollschrittwinkel: ");
        Serial.print(MOTOR_STEP_ANGLE_DEG, 3);
        Serial.println(" Grad");

        Serial.print("Vollschritte/Umdrehung: ");
        Serial.println(fullStepsPerRevolution());

        Serial.print("Steps/Umdrehung: ");
        Serial.println(stepsPerRevolution());

        Serial.print("Interpolation: ");
        Serial.println(driver.intpol() ? "AN" : "AUS");

        Serial.print("Treiber-Modus: ");
        Serial.println(driver.en_spreadCycle() ? "SpreadCycle" : "StealthChop");

        Serial.print("TSTEP: ");
        Serial.println(driver.TSTEP());

        Serial.print("SG_RESULT: ");
        Serial.println(driver.SG_RESULT());

        Serial.print("SGTHRS: ");
        Serial.println(driver.SGTHRS());

        Serial.print("TOFF: ");
        Serial.println(driver.toff());

        Serial.print("TPOWERDOWN: ");
        Serial.println(driver.TPOWERDOWN());

        Serial.print("Temperaturwarnung: ");
        Serial.println(driver.otpw() ? "JA" : "nein");

        Serial.print("Uebertemperatur-Abschaltung: ");
        Serial.println(driver.ot() ? "JA" : "nein");

        Serial.print("Open Load A/B: ");
        Serial.print(driver.ola() ? "JA" : "nein");
        Serial.print(" / ");
        Serial.println(driver.olb() ? "JA" : "nein");

        Serial.print("Kurzschluss A/B: ");
        Serial.print(driver.s2ga() ? "JA" : "nein");
        Serial.print(" / ");
        Serial.println(driver.s2gb() ? "JA" : "nein");

        Serial.print("Stillstand erkannt: ");
        Serial.println(driver.stst() ? "JA" : "nein");

        Serial.println("------------------");
        Serial.println();
    }


    // ==================================================
    // HELP
    // ==================================================

    else if (command == "HELP")
    {
        Serial.println();
        Serial.println("Befehle:");
        Serial.println();
        Serial.println("REV 10");
        Serial.println("  -> 10 Umdrehungen fahren");
        Serial.println();
        Serial.println("MOVE 32000");
        Serial.println("  -> 32000 Steps fahren");
        Serial.println();
        Serial.println("CAL 38.6");
        Serial.println("  -> letzte Fahrt entsprach 38.6 ml");
        Serial.println();
        Serial.println("DOSE 5");
        Serial.println("  -> 5 ml dosieren");
        Serial.println();
        Serial.print("SPEED ");
        Serial.println(DEFAULT_MAX_SPEED_RPM);
        Serial.println("  -> maximale Drehzahl in RPM");
        Serial.println();
        Serial.print("ACCEL ");
        Serial.println(DEFAULT_ACCELERATION_RPM_PER_S);
        Serial.println("  -> Beschleunigung in RPM/s^2");
        Serial.println();
        Serial.print("CURRENT ");
        Serial.println(DEFAULT_MOTOR_CURRENT_MA);
        Serial.print("  -> Motorstrom in mA RMS, Haltestrom ");
        Serial.print(DEFAULT_HOLD_CURRENT_MULTIPLIER * 100.0f, 0);
        Serial.println(" %");
        Serial.println();
        Serial.print("MICROSTEPS ");
        Serial.println(DEFAULT_MICROSTEPS);
        Serial.println("  -> Microsteps einstellen und Kalibrierung loeschen");
        Serial.println();
        Serial.println("MODE STEALTH");
        Serial.println("  -> leiser StealthChop-Betrieb");
        Serial.println();
        Serial.println("MODE SPREAD");
        Serial.println("  -> SpreadCycle fuer mehr Drehmoment/StallGuard");
        Serial.println();
        Serial.print("STALL ");
        Serial.println(DEFAULT_STALL_THRESHOLD);
        Serial.println("  -> StallGuard-Empfindlichkeit 0 bis 255");
        Serial.println();
        Serial.println("SHOWCASE");
        Serial.println("  -> endlos drehen");
        Serial.println();
        Serial.print("SHOWCASE ");
        Serial.println(DEFAULT_SHOWCASE_RPM);
        Serial.println("  -> endlos mit der angegebenen RPM drehen");
        Serial.println();
        Serial.print("SHOWCASE -");
        Serial.println(DEFAULT_SHOWCASE_RPM);
        Serial.println("  -> rueckwaerts drehen");
        Serial.println();
        Serial.println("STOP");
        Serial.println("  -> stoppen");
        Serial.println();
        Serial.println("STATUS");
        Serial.println("  -> aktuellen Zustand anzeigen");
        Serial.println();
    }


    // ==================================================
    // unbekannter Befehl
    // ==================================================

    else
    {
        Serial.print("Unbekannter Befehl: ");
        Serial.println(command);

        Serial.println("HELP fuer Befehlsliste");
    }
}


// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup()
{
    Serial.begin(SERIAL_BAUD);



    // -----------------------------
    // TMC2209 UART
    // -----------------------------
    tmcSerial.begin(TMC_UART_BAUD);
    driver.begin();

    // Alle Einstellungen in einem Block schreiben und danach testen.
    uint8_t result = applyAllDriverSettings();

    Serial.print("TMC2209 UART Test: ");

    if (result == 0)
        Serial.println("OK");
    else {
        Serial.print("FEHLER ");
        Serial.println(result);
    }


    // Standardwerte
    motor.setMaxSpeed(rpmToStepsPerSecond(maxSpeedRpm));
    motor.setAcceleration(rpmToStepsPerSecond(accelerationRpmPerSecond));
    motor.setPinsInverted(DEFAULT_DIRECTION_INVERTED,
                          DEFAULT_STEP_INVERTED,
                          DEFAULT_ENABLE_INVERTED);

    // Mindestlaenge STEP-Puls fuer TMC2209
    motor.setMinPulseWidth(STEP_MIN_PULSE_US);

    // Etwas Speicher reservieren
    serialBuffer.reserve(SERIAL_BUFFER_CAPACITY);

    Serial.println();
    Serial.println("==============================");
    Serial.println("   Pumpensteuerung bereit");
    Serial.println("==============================");
    Serial.println();

    Serial.println("Befehle:");
    Serial.println("REV 10");
    Serial.println("MOVE 32000");
    Serial.println("CAL 38.6");
    Serial.println("DOSE 5");
    Serial.print("SPEED ");
    Serial.println(DEFAULT_MAX_SPEED_RPM);
    Serial.print("ACCEL ");
    Serial.println(DEFAULT_ACCELERATION_RPM_PER_S);
    Serial.print("CURRENT ");
    Serial.println(DEFAULT_MOTOR_CURRENT_MA);
    Serial.print("MICROSTEPS ");
    Serial.println(DEFAULT_MICROSTEPS);
    Serial.println("MODE STEALTH");
    Serial.println("MODE SPREAD");
    Serial.print("STALL ");
    Serial.println(DEFAULT_STALL_THRESHOLD);
    Serial.println("SHOWCASE");
    Serial.print("SHOWCASE ");
    Serial.println(DEFAULT_SHOWCASE_RPM);
    Serial.print("SHOWCASE -");
    Serial.println(DEFAULT_SHOWCASE_RPM);
    Serial.println("STOP");
    Serial.println("STATUS");
    Serial.println("HELP");
    Serial.println();
}


// --------------------------------------------------
// Main Loop
// --------------------------------------------------

void loop()
{
    // --------------------------------------------------
    // Motor bedienen
    // --------------------------------------------------

    if (showcaseMode)
    {
        // Endlos mit konstanter Geschwindigkeit
        motor.runSpeed();
    }
    else
    {
        // Normale Bewegung mit Beschleunigung/Bremsung
        motor.run();
    }


    // --------------------------------------------------
    // Serielle Schnittstelle einlesen
    // --------------------------------------------------

    while (Serial.available())
    {
        char c = Serial.read();

        // Befehl bei Newline ausführen
        if (c == '\n')
        {
            if (serialBuffer.length() > 0)
            {
                processCommand(serialBuffer);
            }

            serialBuffer = "";
        }

        // Carriage Return ignorieren
        else if (c != '\r')
        {
            serialBuffer += c;
        }
    }
}
