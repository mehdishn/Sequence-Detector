#include "systemc.h"

SC_MODULE(sequence_detector)
{
    sc_in<sc_logic> input, rst, clk;
    sc_out<sc_logic> output;

    enum states {S0, S1, S2, S3, S4};
    sc_signal<states> present_state, next_state;

    SC_CTOR(sequence_detector)
    {
        present_state.write(S0);
        next_state.write(S0);

        SC_METHOD(eval_combinational);
        sensitive << input << present_state;

        SC_METHOD(eval_sequential);
        sensitive << clk << rst;
    }
    void eval_combinational();
    void eval_sequential();
};

void sequence_detector::eval_combinational()
{
    output = SC_LOGIC_0;
    next_state = S0;

    switch (present_state.read())
    {
    case S0:
        if (input == SC_LOGIC_1) next_state = S1;
        else next_state = S0;
        break;
    case S1:
        if (input == SC_LOGIC_1) next_state = S2;
        else next_state = S0;
        break;
    case S2:
        if (input == SC_LOGIC_1) next_state = S2;
        else next_state = S3;
        break;
    case S3:
        if (input == SC_LOGIC_1) next_state = S4;
        else next_state = S0;
        break;
    case S4:
        if (input == SC_LOGIC_1) next_state = S2;
        else next_state = S0;
        break;
    }

    if (present_state == S4 && input == SC_LOGIC_0) // issuing output signal
        output = SC_LOGIC_1;
}

void sequence_detector::eval_sequential()
{
    if (rst == SC_LOGIC_1)
        present_state = S0;
    else if (clk->event() && clk == SC_LOGIC_1)
        present_state = next_state;
}

SC_MODULE(sequence_detector_TB)
{
    sc_signal<sc_logic> serial_input, rst, clk;
    sc_signal<sc_logic> output;

    sequence_detector* UUT;

    SC_CTOR(sequence_detector_TB)
    {
        UUT = new sequence_detector("sequence_detector_instance");
        UUT->input(serial_input);
        UUT->rst(rst);
        UUT->clk(clk);
        UUT->output(output);

        SC_THREAD(clock_generation);
        SC_THREAD(reset_assertion);
        SC_THREAD(input_waveform);
    }

    void clock_generation();
    void reset_assertion();
    void input_waveform();
};

void sequence_detector_TB::clock_generation()
{
    while (true)
    {
        wait(20, SC_NS);
        clk = SC_LOGIC_0;
        wait(20, SC_NS);
        clk = SC_LOGIC_1;
    }
}

void sequence_detector_TB::reset_assertion()
{
    while (true)
    {
        wait(37, SC_NS);
        rst = SC_LOGIC_0;
        wait(59, SC_NS);
        rst = SC_LOGIC_1;
        wait(59, SC_NS);
        rst = SC_LOGIC_0;
        wait(); // wait forever
    }
}

void sequence_detector_TB::input_waveform()
{
    while (true)
    {
        wait(25, SC_NS);
        serial_input = SC_LOGIC_1;
        wait(65, SC_NS);
        serial_input = SC_LOGIC_0;
        wait(30, SC_NS);
        serial_input = SC_LOGIC_1;
        wait(30, SC_NS);
        serial_input = SC_LOGIC_0;
    }
}

int sc_main(int, char*[])
{
    sequence_detector_TB* top_level = new sequence_detector_TB("sequence_detector_TB_inst");

    sc_trace_file* VCDFile;
    VCDFile = sc_create_vcd_trace_file("sequence_detector");
    
    sc_trace(VCDFile, top_level->clk, "clock");
    sc_trace(VCDFile, top_level->rst, "reset");
    sc_trace(VCDFile, top_level->serial_input, "serial_input");
    sc_trace(VCDFile, top_level->output, "Mealy_output");

    sc_start(4000, SC_NS);
    return 0;
}