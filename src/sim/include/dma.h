#ifndef SRC_SIM_DMA_H
#define SRC_SIM_DMA_H

#define DMA_CHANNEL_GIF 0

void dma_channel_initialize(int, int, int);
void dma_channel_fast_waits(int);
void dma_wait_fast();

#endif
