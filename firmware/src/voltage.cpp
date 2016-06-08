#include "voltage.h"
#include <terminal.h>
#include "buzzer.h"
#include "mux.h"

bool voltage_is_error;
int voltage_now;
int voltage_limit_l;
int voltage_limit_h;
int voltage_limit_s;

bool voltage_error()
{
    return voltage_is_error;
}

float voltage_spv()
{
    float stepPerVolt = 4095/3.3;
    stepPerVolt *= VOLTAGE_R2/(float)(VOLTAGE_R1+VOLTAGE_R2);
    stepPerVolt /= 2;

    return stepPerVolt;
}

void voltage_init()
{
    float stepPerVolt = voltage_spv();

    voltage_limit_l = VOLTAGE_LIMIT*stepPerVolt;
    voltage_limit_h = VOLTAGE_LIMIT*stepPerVolt + stepPerVolt*0.4;
    voltage_limit_s = VOLTAGE_SHUT*stepPerVolt;
    mux_set_addr(4);
    voltage_now = mux_sample(1);
    voltage_is_error = false;
}

void voltage_tick()
{
    static int divider = 0;
    divider++;

    if (divider > 5) {
        divider = 0;
        mux_set_addr(4);
        int newSample = mux_sample(1);
        if (newSample < voltage_now) voltage_now--;
        if (newSample > voltage_now) voltage_now++;

        if (voltage_is_error) {
            if (voltage_now > voltage_limit_h || voltage_now < voltage_limit_s) {
                voltage_is_error = false;
                buzzer_stop();
            }
        } else {
            if (voltage_now < voltage_limit_l && voltage_now > voltage_limit_s) {
                voltage_is_error = true;
                buzzer_play(MELODY_ALERT, true);
            }
        }
    }
}

float voltage_current()
{
    return voltage_now / voltage_spv();
}

TERMINAL_COMMAND(voltage, "Get the voltage")
{
    terminal_io()->print("voltage=");
    terminal_io()->println((int)(10*voltage_current()));
}
