
#include "hardware/dma.h"
#include "dma.hpp"

/// @brief Just grab the first unused channel
Dma::Dma() 
: channel(dma_claim_unused_channel(true)) 
{}

/// @brief Grab a specific DMA channel
/// @param dma_channel is the channel to claim.
Dma::Dma(uint dma_channel)
: channel(dma_channel)
{
    dma_channel_claim(dma_channel);
}

Dma::~Dma(){
    dma_channel_unclaim	(channel);	
}

