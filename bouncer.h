/*
 * Noah Harren, Bryan Bylsma
 * April 24, 2019
 */

#ifndef NODE_H
#define NODE_H
#include <iostream>
#include <string>
#include <fstream>
#include <ostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}
//Place to define neccessary componenets of our bouncer class
int decode(AVCodecContext *avctx, AVFrame *frame, bool got, AVPacket *pkt);
int encode(AVCodecContext *avctx, AVPacket *pkt, int *got_packet, AVFrame *frame);
AVFrame* getBackground(const char *filename);
AVFrame* coolConvert(AVFrame * input_frame);
void saveCool(int frameNum, AVFrame * background);
#endif

