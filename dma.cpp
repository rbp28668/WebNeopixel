

#include "dma.h"

Dma::Dma() : Dma(dma_claim_unused_channel(true)) {}

Dma::Dma(uint dma_channel)
: channel(dma_channel)
{
    dma_channel_claim(dma_channel);
}

Dma::~Dma(){
    dma_channel_unclaim	(channel);	
}


