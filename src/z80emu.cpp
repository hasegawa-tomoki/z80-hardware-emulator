#include <cstdio>
#include <ctime>
#include <random>
#include <pigpio.h>
#include "cpu.hpp"
#include "mcycle.hpp"
#include "log.hpp"
#include "bus/pigpio_bus_bulk.hpp"

void wait_nano_sec(int ns){
    struct timespec req{};
    req.tv_sec = 0;
    req.tv_nsec = ns;
    nanosleep(&req, nullptr);
}

int main(){
    printf("Hello z80\n");

    PigpioBusBulk bus = PigpioBusBulk();
    bus.syncControl();
    Cpu cpu(&bus);

    cpu.instructionCycle();
    return 0;

    /*
    //// ROM check
    std::array<uint8_t, 32> rom{
            0xf3, 0x31, 0xed, 0x80, 0xc3, 0x1b, 0x00, 0xff, 0xc3, 0xc9, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xc3, 0x9d, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xc3, 0x00, 0x21, 0x00, 0x80, 0x11, 0x00,
    };
    for (int i = 0; i < 100; i++){
        printf("Trying %d\n", i);
        for (uint16_t address = 0; address < (uint16_t)(sizeof rom); address++){
            uint8_t data = Mcycle::m2(&cpu, address);
            if (data != rom[address]){
                printf("ERROR Address: %04x  Data: %02x  Read: %02x\n", address, rom[address], data);
                return 0;
            }
        }
    }

    ////// RAM flip check
    uint16_t start = 0x8000;
    uint16_t end = 0x80ff;
    for (uint16_t address = start; address <= end; address++) {
        Mcycle::m3(&cpu, address, 0xff);
    }
    for (uint16_t address = start; address <= end; address++) {
        if (address % 16 == 0){
            printf("\n%04x ", address);
        }
        uint8_t before = Mcycle::m2(&cpu, address);
        uint8_t flip = ~before;
        Mcycle::m3(&cpu, address, flip);
        uint8_t after = Mcycle::m2(&cpu, address);
        if (before != after){
            if (flip == after){
                printf("______ ");
            } else {
                printf("%02x%02x%02x ", before, flip, after);
            }
        } else {
            printf("%02x%02x%02x=", before, flip, after);
            return 0;
        }
        if (address == end){
            break;
        }
    }
    return 0;

    //// ROM dump
    for (uint16_t address = 0x0000; address < 0x0040; address++) {
        if (address % 16 == 0){
            printf("\n");
        }
        uint8_t data = Mcycle::m2(&cpu, address);
        printf("%02x ", data);
    }

    //// RAM write check with all bit pattern
    int err = 0;
    for (uint16_t address = 0x8001; address < 0x8002; address++) {
        printf("Testing %04x...\n", address);
        for (uint8_t data = 0x00; data <= 0xff; data++){
            cpu.bus->setAddress(0);
            Mcycle::m3(&cpu, address, data);
            uint8_t rData = Mcycle::m2(&cpu, address);
            if (data != rData){
                printf("ERROR Address: %04x  Wrote: %02x  Read: %02x\n", address, data, rData);
                err++;
            }
            if (data == 0xff){
                break;
            }
        }
        if (address == 0xffff){
            break;
        }
    }
    printf("%d errors\n", err);

    // Benchmark
    std::random_device rnd;
    volatile uint32_t data;
    const int TIMES = 20;
    double total = 0;
    for (int t = 0; t < TIMES; t++){
        clock_t start = clock();
        for (int i = 0; i < 1000 * 1000; i++){
            //data = rnd();
            //gpioWrite_Bits_0_31_Set(data);
            //gpioWrite_Bits_0_31_Clear(~data);
            //data = rnd() & 1;
            //gpioWrite(0, data);
            //data = gpioRead(2);
            data = gpioRead_Bits_0_31();
        }
        double time = static_cast<double>(clock() - start) / CLOCKS_PER_SEC * 1000.0;
        printf("%lf ms, ", time);
        total += time;
    }
    printf("Ave.: %lf ms", total / TIMES);

    return 0;
     */


    // -- 1M times gpioWrite_Bits_0_31_Set|Reset(random)
    // full: 3175.987000 ms, 3172.137000 ms, 3171.864000 ms, 3164.771000 ms, 3169.189000 ms, 3167.222000 ms, 3170.484000 ms, 3172.494000 ms, 3169.249000 ms, 3161.352000 ms, 3160.767000 ms, 3168.049000 ms, 3162.361000 ms, 3162.881000 ms, 3161.294000 ms, 3159.369000 ms, 3163.484000 ms, 3161.339000 ms, 3160.340000 ms, 3161.038000 ms,
    //          Min: 3159.369000 ms, Max: 3175.987000 ms
    //          Ave.: 3165.783550 ms
    // rnd : 3123.971000 ms, 3088.205000 ms, 3084.393000 ms, 3110.663000 ms, 3093.465000 ms, 3087.395000 ms, 3089.723000 ms, 3083.916000 ms, 3081.665000 ms, 3081.310000 ms, 3076.269000 ms, 3079.061000 ms, 3086.906000 ms, 3089.891000 ms, 3089.367000 ms, 3085.477000 ms, 3089.815000 ms, 3086.299000 ms, 3090.289000 ms, 3087.032000 ms,
    //          Min: 3076.269000 ms, Max: 3123.971000 ms
    //          Ave.: 3089.255600 ms
    // Ave: 76ns, Min: 36ns, Max: 99ns
    //
    // -- 1M times gpioRead_Bits_0_31()
    // 72.331000 ms, 72.022000 ms, 71.986000 ms, 72.013000 ms, 72.054000 ms, 71.962000 ms, 72.239000 ms, 72.054000 ms, 71.919000 ms, 71.940000 ms, 72.234000 ms, 71.986000 ms, 72.099000 ms, 72.103000 ms, 72.007000 ms, 71.966000 ms, 71.974000 ms, 71.918000 ms, 71.954000 ms, 72.007000 ms,
    //          Min: 71.918000 ms, Max: 72.331000 ms
    //          Ave.: 72.038400 ms
    // Ave: 72ns, Min: 71ns, Max: 72ns
    //
    // -- 1M times gpioWrite(0, random)
    // full: 3210.215000 ms, 3210.457000 ms, 3210.521000 ms, 3208.114000 ms, 3213.092000 ms, 3211.097000 ms, 3203.761000 ms, 3202.617000 ms, 3207.424000 ms, 3212.918000 ms, 3214.757000 ms, 3210.067000 ms, 3206.401000 ms, 3206.297000 ms, 3205.771000 ms, 3212.159000 ms, 3210.860000 ms, 3211.907000 ms, 3210.078000 ms, 3213.277000 ms,
    //          Min: 3202.617000 ms, Max: 3214.757000 ms
    //          Ave.: 3209.589500 ms
    // rnd : 3097.428000 ms, 3085.857000 ms, 3100.964000 ms, 3096.763000 ms, 3096.874000 ms, 3096.585000 ms, 3091.513000 ms, 3095.726000 ms, 3093.862000 ms, 3093.766000 ms, 3096.147000 ms, 3096.096000 ms, 3090.139000 ms, 3092.752000 ms, 3085.730000 ms, 3083.932000 ms, 3085.437000 ms, 3086.650000 ms, 3086.882000 ms, 3086.632000 ms,
    //          Min: 3083.932000 ms, Max: 3100.964000 ms
    //          Ave.: 3091.986750 ms
    // Ave: 118ns, Min: 102ns, Max: 131ns
    //
    // -- 1M times gpioRead(0)
    // 83.828000 ms, 77.222000 ms, 77.218000 ms, 77.233000 ms, 77.489000 ms, 77.212000 ms, 77.176000 ms, 77.191000 ms, 77.109000 ms, 77.162000 ms, 77.306000 ms, 77.252000 ms, 77.249000 ms, 77.239000 ms, 77.307000 ms, 77.267000 ms, 77.253000 ms, 77.184000 ms, 77.397000 ms, 77.208000 ms,
    //          Min: 77.109000 ms, Max: 83.828000 ms
    //          Ave.: 77.575100 ms
    // Ave: 77ns, Min: 77ns, Max: 83ns

    // -- 4MHz
    // 1clk = 0.00000025 sec = 0.00025 ms = 0.25 us = 250 ns
    // M1 = 4clk = 1us
    // -- 2.5MHz
    // 1clk = 0.0000004 sec = 0.0004 ms = 0.4 us = 400 ns
    // M1 = 4clk = 1.6us
}
