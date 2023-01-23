#ifndef DMA_HPP
#define DMA_HPP

#include "hardware/dma.h"

class DmaConfig{
    dma_channel_config channel_config;
    public: 
    DmaConfig(dma_channel_config cfg) :channel_config(cfg) {}
    operator dma_channel_config*() { return &channel_config;}
    inline void bswap(bool swap) {channel_config_set_bswap( &channel_config, swap);}
    inline void dreq(uint dreq)  {channel_config_set_dreq(	&channel_config, dreq);}
    inline void enable(bool enable) { channel_config_set_enable( &channel_config, enable);}
    inline void highPriority(bool highPriority) {channel_config_set_high_priority(&channel_config, highPriority);}
    inline void readIncrement(bool incr) {channel_config_set_read_increment(&channel_config, incr);}
    inline void transferDataSize(enum dma_channel_transfer_size size) {channel_config_set_transfer_data_size(&channel_config, size);}
    inline void writeIncrement(bool incr) {channel_config_set_write_increment(&channel_config, incr);}
};


// For simple peripheral / memory or memory/memory transfers.
class Dma {
    unsigned int channel;
    public:
    Dma();
    Dma(unsigned int dma_channel);
    ~Dma();

    DmaConfig getDefaultConfig() {return dma_channel_get_default_config(channel);}
    DmaConfig getConfig() {return dma_get_channel_config(channel);}

    void abort() { dma_channel_abort(channel);}
    void configure(DmaConfig config, volatile void * write_addr, const volatile void *	read_addr, uint transfer_count, bool trigger=false )
        { dma_channel_configure(channel, config, write_addr, read_addr, transfer_count, trigger);}
    bool isBusy() {return dma_channel_is_busy(channel);}
    void setConfig(DmaConfig config, bool trigger = false) {dma_channel_set_config(channel, config, trigger);}
    void setReadAddr(const volatile void * readAddr,bool trigger = false) {dma_channel_set_read_addr(channel, readAddr, trigger);}
    void setTransCount(uint32_t transCount, bool trigger = false) {dma_channel_set_trans_count(channel, transCount, trigger);} 	
    void setWriteAddr(volatile void* writeAddr, bool trigger = false) {dma_channel_set_write_addr(channel, writeAddr, trigger);}
    void start(){dma_channel_start(channel);}
    void fromBufferNow(const volatile void * readAddr, uint32_t transferCount){dma_channel_transfer_from_buffer_now(channel, readAddr, transferCount);}
    void toBufferNow(volatile void * writeAddr, uint32_t transferCount) {dma_channel_transfer_to_buffer_now(channel, writeAddr, transferCount);}
    volatile void waitForFinish(){dma_channel_wait_for_finish_blocking(channel);}
   
};

#endif