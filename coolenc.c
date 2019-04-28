/*
 * COOL image format encoder
 * Copyright (c) NHBB inc
 * Author: Noah Harren and Bryan Bylsma
 * Note: This file makes use of certain FFmpeg functons. 
 * We have commented them as needed.(labled them as voodoo and included our best guess at their functionality)
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
 * Foundation, Inc., 47 S 300 E SLC 
 */

#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

/*
* This method does nothing, it is only used to follow propper ffmpeg syntax.
*/
static av_cold int cool_encode_init(AVCodecContext *avctx){
    return 0;
}


/*
* Main encoder. This method takes in data from the FFmpeg Frame data-stream and properly encodes it
* Into a cool image.
*/
static int cool_encode(AVCodecContext *avctx, AVPacket *writePkt,
                            const AVFrame *pict, int *got_packet)
{
    const AVFrame * const pframe = pict; // a pointer the the frame of the picture we are given
    int n_bytes_image, n_bytes_per_row, totalBytes, i, headerSize; // predefining variables needed for encoding
    const uint32_t *palette = NULL; // creating a palette pointer to a const
    int pad_bytes_per_row, pal_entries = 0; // predefining variables needed to picture encoding
    int bit_count = 8; // the bitcount of the image we are encoding. This should always be 8.
    uint8_t *pointer, *buffer; //predefining a pointer to the data and our writefile bytestream.

// this simply makes it so the compiler doesnt throw warnings each time we make
FF_DISABLE_DEPRECATION_WARNINGS //disable warnings
    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I; // tells ffmpeg we are an image type (TYPE_I)
    avctx->coded_frame->key_frame = 1; // we only are encoding one frame (its an image)
FF_ENABLE_DEPRECATION_WARNINGS // re-enable them

    palette = (uint32_t *)pframe->data[1]; // set the color palette pointer
    pal_entries = 1 << bit_count; // set our number of palette entries to 256 ( 2^8)

    /*
    *calculates the number of bytes in each row using the supplied row width and bitcount,
    * it then accounts space for padding which makes sure row bytes per row %4 =0
    */
    n_bytes_per_row = ((int64_t)avctx->width * (int64_t)bit_count + 7LL) >> 3LL; 
    pad_bytes_per_row = (4 - n_bytes_per_row) & 3;

    n_bytes_image = avctx->height * (n_bytes_per_row + pad_bytes_per_row); // calcultes the total number of bytes in the image

#define COOLHEADER 12
    if(avctx->pix_fmt == AV_PIX_FMT_PAL8)
      headerSize = COOLHEADER  + (pal_entries << 2); // get the total header size, also take into accout palette size. We need 8 bits per entry so <<2.
    else
      headerSize = COOLHEADER;
    totalBytes = n_bytes_image + headerSize;
    
    ff_alloc_packet2(avctx,writePkt,totalBytes,0); //ffmpeg voodoo(Allocates space for our us to write too, according to the number of total calculated bytes)

    buffer = writePkt->data; //set the buffer at the start of our AVPacket pointer, used to write too. 

    //write out our COOL signature to the writePkt.
    bytestream_put_byte(&buffer, 'C');                   
    bytestream_put_byte(&buffer, '0');                   
    bytestream_put_byte(&buffer, '0');                   
    bytestream_put_byte(&buffer, 'L');                   

    //Some info we will send for later decoding.
    bytestream_put_le32(&buffer, avctx->width);          // width of COOL file
    bytestream_put_le32(&buffer, avctx->height);         // height of COOL file
    if(avctx->pix_fmt == AV_PIX_FMT_PAL8)
    for (i = 0; i < pal_entries; i++)
        bytestream_put_le32(&buffer, palette[i] & 0xFFFFFF); // put in our palette for us to code/decode with
        
    // Cool files are top to bottom so start at the top
    pointer = pframe->data[0] ; // pointer to first bit of the frame data
    buffer = writePkt->data + headerSize; // point to the buffer at the end of the header.


    //This is where the real magic happens. We Basically go row-by-row through the file, adding in
    // one row of pixels at a time, top-bottom. We then add padding to each row to insure that linesize % 4 = 0
    for(i = 0; i < avctx->height; i++) {
        memcpy(buffer, pointer, n_bytes_per_row); // in the current buffer location, copy in n_bytes_per_row of data from the pointer array
        buffer += n_bytes_per_row;
        memset(buffer, 0, pad_bytes_per_row); // used to make sure line size % 4 = 0, and insure proper sizing.
        buffer += pad_bytes_per_row;
        pointer += pframe->linesize[0]; // ... and go back
    }

    writePkt->flags |= AV_PKT_FLAG_KEY; //ffmpeg protocol
    *got_packet = 1;               //ffmpeg protocol :these confirm data recieved.
    return 0;
}

AVCodec ff_cool_encoder = {
    .name           = "cool",
    .long_name      = NULL_IF_CONFIG_SMALL("Cool Image(CS 3505 Spring 2019"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_COOL,
    .init           = cool_encode_init,
    .encode2        = cool_encode,
    .pix_fmts       = (const enum AVPixelFormat[]){
        AV_PIX_FMT_PAL8,
	AV_PIX_FMT_RGB24,
        AV_PIX_FMT_NONE
    },
};
