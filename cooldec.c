/*
 * BMP image format decoder
 * Copyright (c) 2019 NHBB
 * Authors: Noah Harren and Bryan Bylsma
 *
 * This file aspires to be part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * NHBB, Inc., 47 S 300 E SLC UT, 84111.
 */


#include <inttypes.h>
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "msrledec.h"

/*
*   Main decoding method. Using the data-stream from FFmpeg, we will decode our cool file.
*/
static int cool_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
  static int a = 0;
  if(a==0){
    av_log(NULL, AV_LOG_INFO, "\n\n*** Opening a cool image. ***\n*** CS 3505 Spring 2019:\tCreated by Noah Harren and Bryan Bylsma ***\n\n");
    a++;
  }  
    int buffer_size       = avpkt->size; // size of the file

    const uint8_t *buffer = avpkt->data; // used to write from
    AVFrame *pframe         = data; //where to write to

    unsigned int headerSize;
    int width, height; // predefined for later use
    const int bitcount = 8; // this is constant(we always want a byte/pixel)
    int linecheck, linesize; 
    uint8_t *dataPointer;
    const uint8_t *buffer0 = buffer;


    //Insures this is a cool file by checking for a COOL signature.
    if (bytestream_get_byte(&buffer) != 'C' ||
        bytestream_get_byte(&buffer) != '0' ||
        bytestream_get_byte(&buffer) != '0' ||
        bytestream_get_byte(&buffer) != 'L') {
        av_log(avctx, AV_LOG_ERROR, "Looks likes this file was not cool enough. Look at header.\n");
        return AVERROR_INVALIDDATA;
    }
    

    headerSize  = 1036; //the total header size is always equal to a header + 1024 bits required to store palette color data. = 0; //  the size of info for the header. Characters "C00L".

    width  = bytestream_get_le32(&buffer); // gets the image width
    height = bytestream_get_le32(&buffer); // gets the image hight and moves the bufferfer
  

   ff_set_dimensions(avctx, width, height); // ffmpeg voodoo (assuming that this is setting the dimentions and context for the frame)

   avctx->pix_fmt = AV_PIX_FMT_PAL8; // set our pixel format to PAL8


    ff_get_buffer(avctx, pframe, 0); // voodoo magic (assuming this will get the start of the buffer)


    pframe->pict_type = AV_PICTURE_TYPE_I; // set our picture type to image
    pframe->key_frame = 1;  // one frame only since it is a still image

    buffer = buffer0 + headerSize;  // make the buffer point to right after the header

    /* Line size in file multiple of 4 */
    linecheck = ((avctx->width * bitcount + 31) / 8) & ~3;

    dataPointer = pframe->data[0];  // points to the start of our frame
    linesize = pframe->linesize[0]; //set linesize

    int colors = 1 << bitcount; // set the colors to 256 (1 << 8 = 256)

    memset(pframe->data[1], 0, 1024);   // pull the color data from the encoded palette

    buffer = buffer0 + 12 ; //palette location 

    for (int i = 0; i < colors; i++) // for every color
        ((uint32_t*)pframe->data[1])[i] = 0xFFU << 24 | bytestream_get_le32(&buffer); // add the correct color from the palette data to the correct location

    buffer = buffer0 + headerSize; // set the buffer to the frame

    for (int i = 0; i < avctx->height; i++) { 
        memcpy(dataPointer, buffer, linecheck); //create the image
        buffer += linecheck;                    // increment the buffer so we can get the next line
        dataPointer += linesize;        // increment the pointer to point at the next line
    }
    *got_frame = 1; //data confirmation
    return buffer_size; // return the image file size
}

AVCodec ff_cool_decoder = {
    .name           = "cool",
    .long_name      = NULL_IF_CONFIG_SMALL("Cool Image (CS 3505 Spring 2019)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_COOL,
    .decode         = cool_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
