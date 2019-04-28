/*
 * Noah Harren, Bryan Bylsma
 * April 24, 2019
 */

#include "bouncer.h"

int main(int argc, char const *argv[]){
    /*std::cout << "Starting up the Bouncer" << std::endl;
    std::string file_name = argv[1];
        if(file_name.substr(file_name.length()-4,file_name.length()) != ".jpg"){
                std::cout << "File must be a jpg!" << std::endl;
                return 1;
        }
    std::cout << "Got the right file type" <<std::endl;*/
}


/*
 * Returns an AVFrame taken from the command line jpg
 */
AVFrame* get_background_frame(char *filename)
{
    FF_DISABLE_DEPRECATION_WARNINGS
    av_register_all();

    //Using the filename, create an AVFormatContext
    AVFormatContext * jpgFormat = NULL;
    avformat_open_input( &jpgFormat, filename, NULL, NULL);
    avformat_find_stream_info(jpgFormat, NULL);
    av_dump_format(jpgFormat, 0, filename, 0);

    AVCodecContext *jpgCodec;

    
    jpgCodec = jpgFormat->streams[0]->codec; // get the stream at its start
    jpgCodec->pix_fmt = PIX_FMT_YUV; // Set the format to a standard JPG.

    AVCodec * jpgDec = avcodec_find_decoder(jpgCodec->codec_id);

    // Open the Codec
    avcodec_open2(jpgCodec, jpgDec, NULL);

    AVFrame * coolFrame;
    //allocate space for the new frame
    coolFrame = av_frame_alloc();
    //set demensions
    coolFrame->height = jpgCodec->height;
    coolFrame->width = jpgCodec->width;

    int done;
    AVPacket jpgPacket;
    int decode_result;
    int i = 5;

    while(av_read_frame(jpgFormat, &jpgPacket) >= 0)
    {
        if(jpgPacket.stream_index == 0)
        {
        //Decode the frame from the first stream index
        decode_result = avcodec_decode_video2(jpgCodec,coolFrame, &done, &jpgPacket);

        if(done)
        {
            // Convert the frame to cool format
            //coolFrame = coolConvert(coolFrame, PIX_FMT_RGB24);
        }
        }
        // Free the packet
        av_free_packet(&jpgPacket);
    }

    // Closes the codec
    avcodec_close(jpgCodec);

    // Cloes the format
    avformat_close_input(&jpgFormat);
    FF_ENABLE_DEPRECATION_WARNINGS
    return coolFrame;
}
