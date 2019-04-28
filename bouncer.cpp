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
    AVRational preTime = coolContext->time_base;
    AVRational preRate = coolContext->framerate;
    coolContext->time_base = (AVRational){1,25};
    coolContext->framerate = (AVRational){25,1};
    avcodec_open2(coolContext, coolCodec, NULL);
    coolContext->time_base = preTime;
    coolContext->framerate = preRate;
    std::cout << "4" << std::endl;
    std::string coolName = "cool";
    std::cout << "Projected Name: " << coolName << frameNum << std::endl;
    if(frameNum < 10)
    {
        coolName+="00";
        coolName+=frameNum;
    }
    else if(frameNum < 100)
    {
        coolName+="0";
        coolName+=frameNum;
    }
    else
    {
        coolName+= frameNum;
    }

    std::cout << "Actual Name: "<< coolName << std::endl;


    AVFrame *coolFrame = av_frame_alloc();
    std::cout <<"save" << std::endl;
    coolFrame->format = coolContext->pix_fmt;
    coolFrame->height = coolContext->height;
    coolFrame->width = coolContext->width;
    std::cout <<"save2" << std::endl;
    std::cout << background->data << std::endl;
    std::cout <<"_______________________________________________" << std::endl;
    coolFrame = coolConvert(background);
    std::cout << *coolFrame->data <<std::endl;
    std::cout <<"save5" << std::endl;
    AVPacket coolPacket;
    coolPacket.size = 0;
    coolPacket.data = NULL;
    av_init_packet(&coolPacket);
    int setYet = 0;
    std::cout <<"save6" << std::endl;
    //coolContext->codec = coolCodec;   // set the codec to spff 
    //avcodec_send_frame(coolContext, coolFrame);
    //avcodec_receive_packet(coolContext, &coolPacket);
    encode(coolContext,coolFrame, &coolPacket);//got rid of setYet for
    std::cout <<"save6.1" << std::endl;
    coolName += ".cool";
    std::cout <<coolPacket.data <<std::endl;
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

    int result = avformat_open_input( &jpgFormat, filename, NULL, NULL);
    std::cout << result <<std::endl;
    avformat_find_stream_info(jpgFormat, NULL);
    av_dump_format(jpgFormat, 0, filename, 0);
    

    //jpgCodec = jpgFormat->streams[0]->codec; // get the stream at its start

    //std::cout << jpgCodec << std::endl;
    //jpgCodec->pix_fmt = AV_PIX_FMT_YUV420P; // Set the coolFormat to a standard JPG.
    AVCodec * jpgDec = avcodec_find_decoder(jpgFormat->streams[0]->codecpar->codec_id);
    AVCodecContext *jpgCodec = avcodec_alloc_context3(jpgDec);
    int check = avcodec_parameters_to_context(jpgCodec, jpgFormat->streams[0]->codecpar);
    // Open the Codec
    avcodec_open2(jpgCodec, jpgDec, NULL);

    AVFrame * coolFrame;
    //allocate space for the new frame
    coolFrame = av_frame_alloc();
    //set demensions
    coolFrame->height = jpgCodec->height;
    coolFrame->width = jpgCodec->width;
    bool done;
    std::cout << "test" << std::endl;
    AVPacket *jpgPacket;
    jpgPacket = av_packet_alloc();
    int decode_result;
    int i = 5;
    FILE *f;
    f = fopen(filename, "rb");
    while (!feof(f)) {
        AVCodecParserContext * parser = av_parser_init(jpgCodec->codec_id);
        std::cout << "test2" << std::endl;
        uint8_t inbuf[4096];
        /* read raw data from the input file */
        size_t data_size = fread(inbuf, 1, 4096, f);
        if (!data_size)
            break;
        std::cout << "test3" << std::endl;
        /* use the parser to split the data into frames */
        uint8_t* data = inbuf;
        
        while (data_size > 0) {
            std::cout << "testloop1" << std::endl;
            int ret = av_parser_parse2(parser, jpgCodec, &jpgPacket->data, &jpgPacket->size,
                                   data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                exit(1);
            }
            std::cout << "testloop2" << std::endl;
            data      += ret;
            data_size -= ret;
            bool gotData;
            
            if (jpgPacket->size){
                std::cout << "testloopif3" << std::endl;
                decode(jpgCodec, coolFrame, gotData, jpgPacket);
            }
        }

    }
        fclose(f);
    /*while(av_read_frame(jpgFormat, &jpgPacket) >= 0)
    //{
        //if(jpgPacket.stream_index == 0)
        //{
        //Decode the frame from the first stream index
        //decode_result = decode(jpgCodec,coolFrame, done, &jpgPacket);

        av_read_frame(jpgFormat, &jpgPacket); // Read frames and save  - source: Dranger
        std::cout << jpgPacket.data << std::endl;
        // Decode video frames - source: all other deprecations
        avcodec_send_packet(jpgCodec, &jpgPacket);
        avcodec_receive_frame(jpgCodec, coolFrame);
        std::cout << *coolFrame->data << std::endl;

        //if(done)
        //{
            // Convert the frame to cool coolFormat
            std::cout <<"now converting" << std::endl;
            coolFrame = coolConvert(coolFrame);
            std::cout << *coolFrame->data << "Buggy bug?" <<std::endl;
        //}
        //}*/
        // Free the packet
        av_packet_unref(jpgPacket);
    //}
    // Closes the codec
    avcodec_close(jpgCodec);
    // Cloes the coolFormat
    avformat_close_input(&jpgFormat);
    //FF_ENABLE_DEPRECATION_WARNINGS
    return coolFrame;
}

// Used to decode a frame, given a context, frame, and passing a bool
// found at https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
int decode(AVCodecContext *context, AVFrame *frame, bool gotData, AVPacket *jpgPacket)
{
    int ret;

    gotData = false;

    if (jpgPacket) {
        ret = avcodec_send_packet(context, jpgPacket);
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
/*int encode(AVCodecContext *avctx, AVPacket *jpgPacket, int *got_packet, AVFrame *frame)
{
    int ret;

    *got_packet = 0;
    std::cout <<"enc1" << std::endl;
    std::cout <<avctx <<" : " << frame << std::endl;
    ret = av_frame_make_writable(frame);
    std::cout <<"enc1.5:" << ret << std::endl;
    ret = avcodec_send_frame(avctx, frame);

    std::cout <<"enc2" << std::endl;
    ret = avcodec_receive_packet(avctx, jpgPacket);
    if (!ret)
        *got_packet = 1;
        std::cout <<"enc3" << std::endl;
    if (ret == AVERROR(EAGAIN))
        return 0;

    return ret;
}*/

void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *jpgPacket)
{
    int ret;
    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3"PRId64"\n", frame->pts);
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, jpgPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        //printf("Write packet %3"PRId64" (size=%5d)\n", jpgPacket->pts, jpgPacket->size);
        //fwrite(jpgPacket->data, 1, jpgPacket->size, outfile);
        //av_packet_unref(jpgPacket);
    }
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
    num_bytes = av_image_get_buffer_size(coolFormat, input_frame->width, input_frame->height, 1);
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
