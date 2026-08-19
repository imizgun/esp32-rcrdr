#pragma once

#include "soc/gpio_num.h"
#include "sdmmc_cmd.h"
#include <cstdint>
#include <cstdio>

// TODO(you): заполнить значения полей в startFile()/stopFile() (chunk_size и
// data_size известны только когда дозаписали все сэмплы, отсюда и stopFile).
struct WavHeader {
    char riff_tag[4];      // "RIFF"
    uint32_t chunk_size;   // 36 + data_size, известен только в stopFile()
    char wave_tag[4];      // "WAVE"
    char fmt_tag[4];       // "fmt "
    uint32_t fmt_size;     // 16 для PCM
    uint16_t audio_format; // 1 = PCM
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;    // sample_rate * num_channels * bits_per_sample / 8
    uint16_t block_align;  // num_channels * bits_per_sample / 8
    uint16_t bits_per_sample;
    char data_tag[4];      // "data"
    uint32_t data_size;    // количество байт PCM-данных, известен только в stopFile()
};
static_assert(sizeof(WavHeader) == 44, "WAV header must be exactly 44 bytes");

class Memory {
    public:
        Memory(gpio_num_t cs, gpio_num_t clk, gpio_num_t mosi, gpio_num_t miso);

        bool init();
        bool startFile(const char* filename, uint32_t sample_rate, uint16_t bits_per_sample, uint16_t channels);
        void writeSamples(const int16_t* data, size_t count);
        void stopFile();

    private:
        gpio_num_t chip_select_pin;
        gpio_num_t clock_pin;
        gpio_num_t miso_pin;
        gpio_num_t mosi_pin;

        sdmmc_card_t* _card;
        FILE* _file;
        uint32_t _data_bytes;      // сколько байт PCM уже записано в текущий файл
        uint32_t _sample_rate;
        uint16_t _bits_per_sample;
        uint16_t _channels;
};
