
#ifndef PICO_DMA_H
#define PICO_DMA_H

#include "hardware/dma.h"

class DmaConfig : public dma_channel_config {
    public:
    DmaConfig(dma_channel_config cfg) : dma_channel_config(cfg){}
    void setBSwap(bool 	bswap) { channel_config_set_bswap(this, bswap); }
    void setDataTransferRequest(uint dreq) { channel_config_set_dreq(this, dreq); }
    void enable(bool enable) { channel_config_set_enable(this, enable);}	
    void setHighPriority(bool high_priority) { channel_config_set_high_priority(this, high_priority);}
    void setReadIncrement(bool incr) { channel_config_set_read_increment(this, incr);}
    void setDataTransferSize(enum dma_channel_transfer_size size) {channel_config_set_transfer_data_size(this,size);} 
    void setWriteIncrement(bool incr) {channel_config_set_write_increment(this, incr);}
};

class Dma {
    private:
    uint channel;

    public:
    Dma();
    Dma(uint dma_channel);
    ~Dma();
    
    DmaConfig getDefaultConfig(){ return (DmaConfig)::dma_channel_get_default_config(channel); }
    void setConfig(const DmaConfig& config, bool trigger = false) {dma_channel_set_config(channel, &config, trigger);}

    void setReadAddr(const volatile void *read_addr, bool trigger=false) {dma_channel_set_read_addr (channel, read_addr, trigger);}
    void setWriteAddr(volatile void *write_addr, bool trigger=false) {dma_channel_set_write_addr ( channel, write_addr, trigger);}
    void setTransferCount(uint32_t trans_count, bool trigger=false) {dma_channel_set_trans_count(channel, trans_count, trigger);}
    void configure(const DmaConfig& config, volatile void *write_addr, const volatile void *read_addr, uint transfer_count, bool trigger = false){
        dma_channel_configure(channel, &config, write_addr, read_addr, transfer_count, trigger);
    }
    void transferFromBufferNow(const volatile void *read_addr, uint32_t transfer_count) {dma_channel_transfer_from_buffer_now( channel, read_addr, transfer_count);}
    void transferToBufferNow(volatile void *write_addr, uint32_t transfer_count) {dma_channel_transfer_to_buffer_now(channel, write_addr, transfer_count);}
    void start() {dma_channel_start(channel);}
    void abort() {dma_channel_abort(channel);}
    bool isBusy() {return dma_channel_is_busy(channel);}
    void blockUntilFinished() {dma_channel_wait_for_finish_blocking(channel);}
};  

#endif