//
// Created by archie on 11/8/18.
//

# include <math.h>
#include <HID.h>

inline float valueToTemp(int value, float T0, float R0, float B) {
    float t, r;
    r = static_cast<float>(((1023.0 * 10e3) / static_cast<float>(value)) - 10e3);
    T0 += 273.15;
    t = 1.0 / (1.0 / T0 + log(r / R0) / B);
    return t - 273.15;
}

/*−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−*/
const float THERMISTOR_T0 = 25.0;
const float THERMISTOR_R0 = 10000.0;
const float THERMISTOR_B = 3977.0;
/*−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−*/
const unsigned long MONITOR_MS_PERIOD = 1000;
const unsigned long PERIODO_TMP_MS = 1000;
const unsigned long RELAY_MS_PERIOD = 5000;
const byte RELAY_PIN = 2;
const byte RLED_PIN = 3;
const byte GLED_PIN = 5;
const byte TEMP_PIN = 0;
const float TEMP_REF = 38.0;
const float TEMP_MIN = 36.0;
const float TEMP_MAX = 40.0;
//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
struct Temp {
    unsigned long last_ms;
    float value;
};

void temp_setup(unsigned long curr_ms, struct Temp &temp) {
    pinMode(GLED_PIN, OUTPUT);
    temp.last_ms = curr_ms - PERIODO_TMP_MS;
    temp.value = 0;
}

void tmp_task(unsigned long curr_ms, struct Temp &temp) {
    if (curr_ms - temp.last_ms >= PERIODO_TMP_MS) {
        temp.last_ms += PERIODO_TMP_MS;
        int val = analogRead(TEMP_PIN);
        temp.value = valueToTemp(val, THERMISTOR_T0, THERMISTOR_R0, THERMISTOR_B);
        byte est_tmp = static_cast<byte>((TEMP_MIN <= temp.value && temp.value <= TEMP_MAX) ? HIGH : LOW);
        digitalWrite(GLED_PIN, est_tmp);
    }
}

//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
struct Relay {
    unsigned long last_ms;
    byte state;
};

void relay_setup(unsigned long current_ms, struct Relay &relay) {
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(RLED_PIN, OUTPUT);
    relay.last_ms = current_ms - RELAY_MS_PERIOD;
    relay.state = LOW;
}

void relay_task(unsigned long current_ms, float tmp_val, struct Relay &relay) {
    if (current_ms - relay.last_ms >= RELAY_MS_PERIOD) {
        relay.last_ms += RELAY_MS_PERIOD;
        relay.state = static_cast<byte>((tmp_val < TEMP_REF) ? HIGH : LOW);
        digitalWrite(RELAY_PIN, relay.state);
        digitalWrite(RLED_PIN, relay.state);
    }
}

//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
struct Monitor {
    unsigned long last_ms;
};

void monitor_setup(unsigned long current_ms, struct Monitor &monitor) {
    Serial.begin(9600);
    if (Serial) {
        Serial.println("# > Arduino Control de Temperatura Basico (5000 ms)");
        Serial.println("#$ -y20:45 -w4 -l36 -l38 -l40 - tTemperatura - tCalefactor");
    }
    monitor.last_ms = current_ms - MONITOR_MS_PERIOD;
}

void monitor_task(unsigned long curr_ms, struct Monitor &monitor,
                  const struct Temp &temp, const struct Relay &relay) {
    if (curr_ms - monitor.last_ms >= MONITOR_MS_PERIOD) {
        monitor.last_ms += MONITOR_MS_PERIOD;
        if (Serial) {
            Serial.print(temp.value);
            Serial.print(" ");
            Serial.println(relay.state);
        }
    }
}

//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
struct Monitor monitor;
struct Temp temp;
struct Relay relay;

void setup() {
    unsigned long current_ms = millis();
    temp_setup(current_ms, temp);
    relay_setup(current_ms, relay);
    monitor_setup(current_ms, monitor);
}

void loop() {
    unsigned long current_ms = millis();
    tmp_task(current_ms, temp);
    relay_task(current_ms, temp.value, relay);
    monitor_task(current_ms, monitor, temp, relay);
}
