/*
 * Noah Harren, Bryan Bylsma
 * April 24, 2019
 */


#include "bouncer.h"
#include <malloc.h>
int main(int argc, char const *argv[]){
    std::cout << "Starting up the Bouncer" << std::endl;
    std::string file_name = argv[1];
        if(file_name.substr(file_name.length()-4,file_name.length()) != ".jpg"){
                std::cout << "File must be a jpg!" << std::endl;
                return 1;
        }
    std::cout << "Got the right file type" <<std::endl;
    AVFrame * background = getBackground(file_name.c_str());
    std::cout << "Got the background image" <<std::endl;
    //Do stuff with it!

    saveCool(1,background);
    //Writing Loop
    for(int i = 0; i < 300; i++){
        //handle changes
        //write frame
    }
}

void saveCool(int frameNum, AVFrame * background){
    std::cout << "1" << std::endl;
    AVCodec *coolCodec = avcodec_find_encoder(AV_CODEC_ID_COOL);
    AVCodecContext *coolContext = avcodec_alloc_context3(coolCodec);
    std::cout << "2" << std::endl;
    coolContext->pix_fmt = AV_PIX_FMT_RGB24;
    coolContext->height = background->height;
    coolContext->width = background->width;
    int width = coolContext->width;
    int height = coolContext->height;
    std::cout << "3" << std::endl;

    coolContext->time_base = (AVRational){1,24};
    avcodec_open2(coolContext, coolCodec, NULL);
    std::cout << "4" << std::endl;
    std::string coolName = "cool";
    if(frameNum <10)
    {
        coolName+="00" +frameNum;
    }
    else if(frameNum <100)
    {
        coolName+="0" +frameNum;
    }
    else
    {
        coolName+= frameNum;
    }

    std::cout << coolName << std::endl;


    AVFrame *coolFrame = av_frame_alloc();

    coolFrame->format = coolContext->pix_fmt;
    coolFrame->height = coolContext->height;
    coolFrame->width = coolContext->width;

    coolFrame = coolConvert(background);
    std::cout <<"save5" << std::endl;
    AVPacket coolPacket;
    av_init_packet(&coolPacket);
    coolPacket.size = 0;
    coolPacket.data = NULL;
    int setYet = 0;
    std::cout <<"save6" << std::endl;
    encode(coolContext, &coolPacket, &setYet ,coolFrame);
    std::cout <<"save6.1" << std::endl;
    std::ofstream coolOutput;
    coolOutput.open(coolName.c_str());
    std::cout <<"save7" << std::endl;
    if( coolOutput.is_open() && setYet)
    {
        coolOutput << coolPacket.data;
        coolOutput.close();
        av_packet_unref(&coolPacket);
    }
        std::cout <<"save8" << std::endl;
    av_free(coolFrame);
    avcodec_close(coolContext);
    av_free(coolContext);

  }





/*
 * Returns an AVFrame taken from the command line jpg
 */
AVFrame* getBackground(const char *filename)
{
    //FF_DISABLE_DEPRECATION_WARNINGS
    //Using the filename, create an AVFormatContext
    AVFormatContext * jpgFormat = NULL;
    avformat_open_input( &jpgFormat, filename, NULL, NULL);
    avformat_find_stream_info(jpgFormat, NULL);
    av_dump_format(jpgFormat, 0, filename, 0);

    AVCodecContext *jpgCodec = avcodec_alloc_context3(NULL);

    //jpgCodec = jpgFormat->streams[0]->codec; // get the stream at its start
    int check = avcodec_parameters_to_context(jpgCodec, jpgFormat->streams[0]->codecpar);
    //std::cout << jpgCodec << std::endl;
    jpgCodec->pix_fmt = AV_PIX_FMT_YUV420P; // Set the coolFormat to a standard JPG.
    AVCodec * jpgDec = avcodec_find_decoder(jpgCodec->codec_id);

    // Open the Codec
    avcodec_open2(jpgCodec, jpgDec, NULL);

    AVFrame * coolFrame;
    //allocate space for the new frame
    coolFrame = av_frame_alloc();
    //set demensions
    coolFrame->height = jpgCodec->height;
    coolFrame->width = jpgCodec->width;
    bool done;
    AVPacket jpgPacket;
    int decode_result;
    int i = 5;
    while(av_read_frame(jpgFormat, &jpgPacket) >= 0)
    {
        if(jpgPacket.stream_index == 0)
        {
        //Decode the frame from the first stream index
        decode_result = decode(jpgCodec,coolFrame, done, &jpgPacket);

        if(done)
        {
            // Convert the frame to cool coolFormat
            std::cout <<"now converting" << std::endl;
            coolFrame = coolConvert(coolFrame);
        }
        }
        // Free the packet
        av_packet_unref(&jpgPacket);
    }
    // Closes the codec
    avcodec_close(jpgCodec);
    // Cloes the coolFormat
    avformat_close_input(&jpgFormat);
    //FF_ENABLE_DEPRECATION_WARNINGS
    return coolFrame;
}

// Used to decode a frame, given a context, frame, and passing a bool
// found at https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
int decode(AVCodecContext *context, AVFrame *frame, bool gotData, AVPacket *pkt)
{
    int ret;

    gotData = false;

    if (pkt) {
        ret = avcodec_send_packet(context, pkt);
        // In particular, we don't expect AVERROR(EAGAIN), because we read all
        // decoded frames with avcodec_receive_frame() until done.
        if (ret < 0)
            return ret == AVERROR_EOF ? 0 : ret;
    }

    ret = avcodec_receive_frame(context, frame);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return ret;
    if (ret >= 0)
        gotData = true;

    return 0;
}

// found at https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
int encode(AVCodecContext *avctx, AVPacket *pkt, int *got_packet, AVFrame *frame)
{
    int ret;

    *got_packet = 0;
    std::cout <<"enc1" << std::endl;
    std::cout <<avctx <<" : " << frame << std::endl;
    avcodec_send_frame(avctx, frame);

    std::cout <<"enc2" << std::endl;
    ret = avcodec_receive_packet(avctx, pkt);
    if (!ret)
        *got_packet = 1;
        std::cout <<"enc3" << std::endl;
    if (ret == AVERROR(EAGAIN))
        return 0;

    return ret;
}



/**
 * Converts the Frame into the coolest coolFormat
 * Returns an AVFrame 
 */
AVFrame* coolConvert(AVFrame * input_frame)
{
    AVPixelFormat coolFormat = AV_PIX_FMT_RGB24;
    // Convert our Frame to a specified color schmeme. 
    // Use a second Frame to store this coolFormat of the picture
    AVFrame * coolFrame;
    
    coolFrame = av_frame_alloc();

    // Use these buffers to store temp data
    const uint8_t *buffer;
    int num_bytes;
    std::cout <<"test1" << std::endl;
    // Determine required buffer size and allocate buffer
    num_bytes = av_image_get_buffer_size(coolFormat, input_frame->width, input_frame->height, 32);
    buffer = (uint8_t *)av_malloc(num_bytes);
    std::cout <<"test2" << std::endl;
    coolFrame->format = coolFormat;

    // set up the output width, height, coolFormat
    coolFrame->height = input_frame->height;
    coolFrame->width =  input_frame->width;
    std::cout <<"test3" << std::endl;
    //create a scaler context from the input frame and intended frame
    struct SwsContext *scaler = NULL;
    scaler = sws_getContext(input_frame->width,input_frame->height,AV_PIX_FMT_YUV420P,input_frame->width,input_frame->height,AV_PIX_FMT_RGB24,SWS_BILINEAR,NULL,NULL,NULL);
    //copy the image data into a buffer
    std::cout <<"test5" << std::endl;
    // push the buffer into our image
    av_image_fill_arrays(coolFrame->data,coolFrame->linesize, buffer, coolFormat, input_frame->width, input_frame->height, 1);
    //av_image_fill_arrays (picture->data, picture->linesize, picture_buf, pix_fmt, width, height, 1);
    std::cout <<"test6" << std::endl;
    //rescale the image
    sws_scale (scaler,(uint8_t const * const *)input_frame->data,input_frame->linesize,0,input_frame->height,coolFrame->data,coolFrame->linesize);
    std::cout <<"test7" << std::endl;
    // Free memory for original picture
    av_free(scaler);
    std::cout <<"test8" << std::endl;
    return coolFrame;
}
