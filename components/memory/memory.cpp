#include "memory.hpp"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_vfs_fat.h"
#include "hal/spi_types.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

char BASE_PATH[] = "/sdcard";

// Потолок для линейного поиска свободного индекса, чтобы при повреждённой
// файловой системе не крутиться бесконечно.
#define MAX_RECORDINGS 10000

Memory::Memory(gpio_num_t cs, gpio_num_t clk, gpio_num_t mosi, gpio_num_t miso) {
    this->chip_select_pin = cs;
    this->clock_pin = clk;
    this->mosi_pin = mosi;
    this->miso_pin = miso;
    this->_card = nullptr;
    this->_file = nullptr;
    this->_data_bytes = 0;
    this->_writingFailed = false;
}

bool Memory::init() {
    spi_bus_config_t config = {};
    config.mosi_io_num = mosi_pin;
    config.miso_io_num = miso_pin;
    config.sclk_io_num = clock_pin;
    config.max_transfer_sz = 512;

    esp_err_t err = spi_bus_initialize(SPI2_HOST, &config, SPI_DMA_CH_AUTO);
    printf("spi_bus_initialize: %s\n", esp_err_to_name(err));
    if (err != ESP_OK) {
        return false;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    host.max_freq_khz = 4000;

    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.gpio_cs = this->chip_select_pin;
    slot_cfg.host_id = SPI2_HOST;

    esp_vfs_fat_mount_config_t mount_conf = {
        .format_if_mount_failed = false,
        .max_files = 1,
        .allocation_unit_size = 512,
        .disk_status_check_enable = true,
        .use_one_fat = true
    };

    err = esp_vfs_fat_sdspi_mount(BASE_PATH, &host, &slot_cfg, &mount_conf, &_card);
    printf("esp_vfs_fat_sdspi_mount: %s\n", esp_err_to_name(err));

    return err == ESP_OK;
}

int Memory::findNextIndex(const char* prefix, const char* ext) {
    if (_card == nullptr) {
        return -1;
    }

    char fullPath[64];
    struct stat st;

    for (int i = 0; i < MAX_RECORDINGS; i++) {
        snprintf(fullPath, sizeof(fullPath), "%s/%s%d%s", BASE_PATH, prefix, i, ext);
        if (stat(fullPath, &st) != 0) {
            return i;
        }
    }

    return MAX_RECORDINGS;
}

WavHeader Memory::_buildHeader() {
    WavHeader header;
    memcpy(header.riff_tag, "RIFF", 4);
    memcpy(header.wave_tag, "WAVE", 4);
    memcpy(header.fmt_tag, "fmt ", 4);
    header.fmt_size = 16;
    header.audio_format = 1; // PCM
    header.num_channels = _channels;
    header.sample_rate = _sample_rate;
    header.byte_rate = _sample_rate * _channels * _bits_per_sample / 8;
    header.block_align = _channels * _bits_per_sample / 8;
    header.bits_per_sample = _bits_per_sample;
    memcpy(header.data_tag, "data", 4);
    header.chunk_size = 36 + _data_bytes;
    header.data_size = _data_bytes;
    return header;
}

bool Memory::startFile(const char* filename, uint32_t sample_rate, uint16_t bits_per_sample, uint16_t channels) {
    this->_writingFailed = false;
    
    char fullPath[64];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", BASE_PATH, filename);

    // "x" => O_EXCL: существующий файл не будет затёрт, fopen вернёт nullptr.
    // Это страховка на случай, если счётчик записей разошёлся с содержимым карты.
    _file = fopen(fullPath, "wbx");

    if (_file == nullptr) {
        return false;
    }

    _sample_rate = sample_rate;
    _bits_per_sample = bits_per_sample;
    _channels = channels;
    _data_bytes = 0;

    WavHeader header = _buildHeader();
    fwrite(&header, sizeof(WavHeader), 1, _file);

    return true;
}

void Memory::writeSamples(const int16_t* data, size_t count) {
    size_t wrote = fwrite(data, sizeof(int16_t), count, _file);

    if (wrote < count) {
        size_t wrote_remain_count = count - wrote;
        const int16_t* remain_data = data + wrote;
        size_t repeated_wrote = fwrite(remain_data, sizeof(int16_t), wrote_remain_count, _file);
        
        if (repeated_wrote < wrote_remain_count) {
            this->_writingFailed = true;
            this->_data_bytes += (wrote + repeated_wrote) * sizeof(int16_t);
            return;
        }
    }
    
    this->_data_bytes += wrote * sizeof(int16_t);
}

bool Memory::isWritingFailed() {
    return this->_writingFailed;
}

void Memory::stopFile() {
    if (_file == nullptr) return;

    WavHeader header = _buildHeader();

    fseek(_file, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, _file);
    fclose(_file);

    _file = nullptr;
}
