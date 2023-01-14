#ifndef DMA_HPP
#define DMA_HPP

class Dma {
    unsigned int channel;
    public:
    Dma();
    Dma(unsigned int dma_channel);
    ~Dma();
};

#endif