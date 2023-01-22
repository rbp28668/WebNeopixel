
#include "hardware/dma.h"
#include "dma.hpp"

Dma::Dma() : Dma(dma_claim_unused_channel(true)) {}

Dma::Dma(uint dma_channel)
: channel(dma_channel)
{
    dma_channel_claim(dma_channel);
}

Dma::~Dma(){
    dma_channel_unclaim	(channel);	
}


    // dma_channel_config channel_config = ;
    // channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
    // channel_config_set_chain_to(&channel_config, DMA_CB_CHANNEL);
    // channel_config_set_irq_quiet(&channel_config, true);
    // dma_channel_configure(DMA_CHANNEL,
    //                       &channel_config,
    //                       &pio->txf[sm],
    //                       NULL, // set by chain
    //                       8, // 8 words for 8 bit planes
    //                       false);