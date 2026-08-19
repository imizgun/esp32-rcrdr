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

char BASE_PATH[] = "/sdcard";

Memory::Memory(gpio_num_t cs, gpio_num_t clk, gpio_num_t mosi, gpio_num_t miso) {
    this->chip_select_pin = cs;
    this->clock_pin = clk;
    this->mosi_pin = mosi;
    this->miso_pin = miso;
    this->_card = nullptr;
    this->_file = nullptr;
    this->_data_bytes = 0;
}

bool Memory::init() {
    spi_bus_config_t config = {
        .mosi_io_num = mosi_pin, 
        .miso_io_num = miso_pin,
        .sclk_io_num = clock_pin,
        .max_transfer_sz = 512,
    };

    spi_bus_initialize(SPI2_HOST, &config, this->mosi_pin);

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;

    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.gpio_cs = this->chip_select_pin;
    slot_cfg.host_id = SPI2_HOST;

    esp_vfs_fat_mount_config_t mount_conf = {
        .format_if_mount_failed = true,
        .max_files = 1,
        .allocation_unit_size = 512,
        .disk_status_check_enable = true,
        .use_one_fat = true
    };

    esp_vfs_fat_sdmmc_mount(BASE_PATH, &host, &slot_cfg, &mount_conf, &_card);

    
    return false;
}

bool Memory::startFile(const char* filename, uint32_t sample_rate, uint16_t bits_per_sample, uint16_t channels) {
    char slash[] = "/";
    char* fullPath = strcat(BASE_PATH, strcat(slash, filename));

    _file = fopen(fullPath, "wb");

    if (_file == nullptr) {
        return false;
    }

    _sample_rate = sample_rate;
    _bits_per_sample = bits_per_sample;
    _channels = channels;
    _data_bytes = 0;

    WavHeader header;

    fwrite(&header, sizeof(WavHeader), 1, _file);
    
    return true;
}

void Memory::writeSamples(const int16_t* data, size_t count) {
    // TODO(you):
    //   1. fwrite(data, sizeof(int16_t), count, _file).
    //   2. _data_bytes += count * sizeof(int16_t).
    // Подумай, что делать, если fwrite записал меньше, чем count (SD отвалилась
    // на записи) — молча игнорировать некорректно.

    size_t wrote = fwrite(data, sizeof(int16_t), count, _file);

    if (wrote < sizeof(data)) {
        ...
    }
    
    this->_data_bytes += count * sizeof(int16_t);
}

void Memory::stopFile() {
    // TODO(you):
    //   1. Если _file == nullptr — нечего закрывать, выйти.
    //   2. Собрать финальный WavHeader: chunk_size = 36 + _data_bytes,
    //      data_size = _data_bytes, byte_rate/block_align посчитать из
    //      _sample_rate/_bits_per_sample/_channels.
    //   3. fseek(_file, 0, SEEK_SET), fwrite(&header, sizeof(header), 1, _file) —
    //      перезаписать заглушку из startFile() реальными значениями.
    //   4. fclose(_file); _file = nullptr.
    
    if (_file == nullptr) return;

    // уточнить по поводу byte_rate и block_align 
    WavHeader header;
    header.chunk_size = 36 + _data_bytes;
    header.data_size = _data_bytes;
    header.byte_rate = _sample_rate * _channels * _bits_per_sample / 8;
    header.block_align = _channels * _bits_per_sample / 8;
    
    fseek(_file, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, _file);
    fclose(_file);
    _file = nullptr;
}
