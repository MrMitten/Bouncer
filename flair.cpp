/*
 * Noah Harren and Bryan Bylsma
 * CS3505 FFMPEG Part 3
 * April 28, 2019
 */
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
//forward declare everything.
int decode(AVCodecContext *coolContext, AVFrame *coolFrame, int * gotData, AVPacket *jpgPacket);
int encode(AVCodecContext *avctx, AVPacket *jpgPacket, AVFrame *coolFrame, int *got_packet);
void makeBall(AVFrame *coolFrame, int Xcord, int Ycord, int ballSize);
AVFrame* makeRGB(AVFrame * inFrame, AVPixelFormat form);
AVFrame* getBackground(char *coolName);
void saveCool(AVFrame *inFrame, int frameNum);

/**
 * @brief main will take in a file name that ends in jpg, and make it into 300 frames that will
 * now have a glowing ball moving across them.
 * @param argc
 * @param argv is the file name
 * @return 0 if it ran successfully
 */
int main(int argc, char *argv[])
{
  std::cout << "Starting up the Bouncer" << std::endl;
  std::string file_name = argv[1]; // get the name of the file
      if(file_name.substr(file_name.length()-4,file_name.length()) != ".jpg"){ //check to make sure it is a jpg
              std::cout << "File must be a jpg!" << std::endl;
              return 1; //return error 1 since its not jpg
      }
  std::cout << "Got the right file type" <<std::endl;
  AVFrame * backFrame = getBackground(argv[1]);  // now get the frame of the jpg image

  int ballSize = sqrt(backFrame->width * backFrame->height) * .075; //using that frame, determine how big our ball should be

  for (int i = 0; i < 300; i++){ //create 300 frames
    std::cout<< "Creating frame: " << i << std::endl;
    AVFrame* tempFrame = makeRGB(backFrame, AV_PIX_FMT_RGB24); // make a temp frame from our background frame so we can save new on each time through the for loop.
    makeBall(tempFrame, ballSize, backFrame->height - backFrame->height/3, ballSize); // draw the ball on the tempframe
    saveCool(tempFrame, i); // save the file as a cool file
  }
  std::cout<< "All done! Type: \"make movie\" to create your movie." << std::endl;
  std::cout<< "After that type \"make play\" to play the movie!" << std::endl;
  return 0; //return 0 means success!
}

/**
 * Modified from https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decode_video.c
 * @brief getBackground is used to get the background frame we will copy from to create all the frames of our movie. Takes in a name to convert
 * */
AVFrame* getBackground(char *coolName)
{

  AVFormatContext * bgFormat = NULL;
  //Open the incoming image by filename
  avformat_open_input( &bgFormat, coolName, NULL, NULL);
  //Grab the jpg streams
  avformat_find_stream_info(bgFormat, NULL);
  //dump the info to console(used to see if it worked)
  av_dump_format(bgFormat, 0, coolName, 0);

  
  //Create our jpg decoder
  AVCodec * bgDecoder = avcodec_find_decoder(bgFormat->streams[0]->codecpar->codec_id);

  //Create an associated context
  AVCodecContext *bgContext = avcodec_alloc_context3(bgDecoder);
  //Get the codec params, used to update neccessary info
  avcodec_parameters_to_context(bgContext, bgFormat->streams[0]->codecpar);
  bgContext->pix_fmt = AV_PIX_FMT_YUV420P; //hardset(needed later in the convert)

  //open the decoder
  avcodec_open2(bgContext, bgDecoder, NULL);

  //allocate space for our return frame, set appropriate heights, width.
  AVFrame * bgFrame = av_frame_alloc();

  bgFrame->height = bgContext->height;
  bgFrame->width = bgContext->width; //fill the background frame with the appropriate data

  //used similar to a bool
  int done;
  // allocate the data packet
  AVPacket jpgPacket;
  //start read
  while(av_read_frame(bgFormat, &jpgPacket) >= 0) // this while loop modified from https://github.com/MurthyA/FFmpeg-tutorial-samples/blob/master/tutorial01.c lines 144 - 160
  {
    if(jpgPacket.stream_index == 0)
    {
      //Decode the packet into a frame
      decode(bgContext, bgFrame, &done, &jpgPacket);

      if(done)
      {
        //convert the frame to RGB24
        bgFrame = makeRGB(bgFrame, AV_PIX_FMT_YUV420P);
      }
    }

    av_packet_unref(&jpgPacket);
  }
  avformat_close_input(&bgFormat);
  avcodec_close(bgContext); // free up all the memory
  return bgFrame; //return our frame we just made
}


//Code below modified from https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/scaling_video.c 
//method is used to translate whatever format we get into rgb4 since that is what our cool files use.
AVFrame* makeRGB(AVFrame * inFrame, AVPixelFormat form)
{
  //Allocate the converted frame and set its params
  AVFrame * conFrame = av_frame_alloc();

  conFrame->height = inFrame->height; //fill the converted frame with the data.
  conFrame->format = AV_PIX_FMT_RGB24;
  conFrame->width =  inFrame->width;
 
  //Used for rescalling and converting between formats
  struct SwsContext *scalar = sws_getContext(inFrame->width,inFrame->height,form,conFrame->width,conFrame->height,AV_PIX_FMT_RGB24,SWS_BICUBIC,NULL,NULL,NULL);
  //calculate the size of our temporary buffer, and create it
  int buffyBities = av_image_get_buffer_size(AV_PIX_FMT_RGB24, inFrame->width, inFrame->height, 1);
  uint8_t *buffer = (uint8_t *)av_malloc(buffyBities * sizeof(uint8_t));

  //fill the conFrame with "empty" data
  av_image_fill_arrays(conFrame->data,conFrame->linesize, buffer, AV_PIX_FMT_RGB24, inFrame->width, inFrame->height, 1);
  //Scale the inFrame image into rgb24, send into conFrame.
  sws_scale(scalar,(const uint8_t * const *)inFrame->data,inFrame->linesize,0,inFrame->height,conFrame->data,conFrame->linesize);

  av_free(scalar);
  //return the converted frame
  return conFrame;
}


/**
 * @brief This method is used to draw a ball onto a frame. It takes in a "start" Y and X cord, a frame to draw on, and a ball Size. It
 * moves the ball in the frame using some simple math. The ball that is drawn is blue, with animated shading.
 **/
void makeBall(AVFrame *coolFrame, int Xcord, int Ycord, int ballSize)
{
  //static double used for storing the Y cord out of scope
  double static moveY = 0;
  moveY = moveY + .16; //move our Y by a set amount per frame
  moveY = fmod(moveY,(3 * 3.14159));//mod it by a the ends of a sin curve. We use 3 to produce a little "bump" at the midpoint of the anim.
  int yPos =  -1 *Ycord/2.5* sin(moveY);//set position using sin func
  Ycord -= yPos; //Adjust new Y acoord-ingly

  //Used to check if we are moving left or right
  bool static right = true;
  //Stores the current X modifier
  int static precord = 0;
  if(right){
  precord += coolFrame->width*.01; //move 1% of the screen width
  Xcord += precord;//Set as new cord
  if(Xcord > (coolFrame->width - ballSize/2))//if we have "hit" the end of the image
    right = false;
  }
  else{//going left
  precord -= coolFrame->width*.01;//move 1% of the screen width
  Xcord += precord;
  if(Xcord < ballSize/2)//if we have "hit" the start of the image
    right = true;
  }
  for (int i = 0; i < coolFrame->height; i++)
  {
    for (int j = 0; j < coolFrame->width * 3; j=j+3)
    {
      //Calculate the distance from midpoint of our ball
      int y = i;
      int x = j / 3;
      y = (Ycord - y)*(Ycord - y);
      x = (Xcord - x)*(Xcord - x);

      int dis = sqrt( x  + y );

      if (ballSize > dis)//only draw on pixels within the ball, so check pixel distance from center
      {
        // based off the pixel dis from center calculate the amount of green to simulate shading.
        //NOTE: we totally could have done a ball with normal shading, but we thought the animated pulsating looked way cooler
        double green = ((dis)*10 + yPos) % 255;
    
        coolFrame->data[0][i*coolFrame->linesize[0]+j] = 61; // put the red pixel data in the frame where we wanna draw our ball
        coolFrame->data[0][i*coolFrame->linesize[0]+j+1] = (int)green;// put in green with a modifier so it changes based off dis to the center (so it looks .cool)
        coolFrame->data[0][i*coolFrame->linesize[0]+j+2] = 239;  // put in the blue    
      }
    }
  }
}

/**
 * This code is modified from https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/encode_video.c
 * Takes in a frame and a frame number, and will create a file named framenum.cool that will be used to create a movie.
 */
void saveCool(AVFrame *inFrame, int frameNum)
{


  AVCodec *coolCodec = avcodec_find_encoder(AV_CODEC_ID_COOL);

  AVCodecContext *coolContext = avcodec_alloc_context3(coolCodec); //create a context for our cool encoder and allocate the appropriate memory

  AVFrame *coolFrame = av_frame_alloc(); // create a frame and allocate the appropriate memory for it

  coolContext->width = inFrame->width; // set the context with the inputed frame data
  coolFrame->width = coolContext->width; // also set the frame with this data

  coolContext->height = inFrame->height; // same as above
  coolFrame->height = coolContext->height;

  coolContext->time_base = (AVRational){1,25}; //since we are making a movie we need to set the playback and framerate
  coolContext->framerate = (AVRational){25,1};

  coolContext->pix_fmt = AV_PIX_FMT_RGB24; // set pixel data since cools only do RGB24 with Peters new code.
  coolFrame->format = coolContext->pix_fmt; // set the frame to the same format
  avcodec_open2(coolContext, coolCodec, NULL); //open up the encoder

  coolFrame = makeRGB(inFrame, coolContext->pix_fmt); // make sure the cool frame is RGB24


  AVPacket coolPacket; // declare our packet
  av_init_packet(&coolPacket); //allocate space for our packet

  int changed = 0; // used as a bool to tell if we have encoded the context.
  encode(coolContext, &coolPacket, coolFrame, &changed); //put the data into the packet using the encode method

  //this method was found on http://www.cplusplus.com/reference/cstdio/sprintf/
  char coolName[16]; // set a buffer for the name data of our file
  sprintf(coolName, "frame%03d.cool", frameNum); // set the name of our current frame using the framenumber.
  FILE *finishedFrame = fopen(coolName, "wb"); // create and open the file we are about to write to

  if(1 == changed) // if our encode worked
  {
    
    fwrite(coolPacket.data, 1, coolPacket.size, finishedFrame); // write the data to the file.
    av_packet_unref(&coolPacket); // free the data back up.
  }
  else
  {
   std::cout << "Encoding failed... Guess you were not .cool enough" << std::endl;
   return;
  }

  av_free(coolFrame); // deallocate all the memory we dont need anymore.
  avcodec_close(coolContext);
  av_free(coolContext);
  av_free(inFrame);
}


// Used to decode a coolFrame, given a coolContext, coolFrame, and an int used as a bool
// found at https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
int decode(AVCodecContext *coolContext, AVFrame *coolFrame, int * gotData, AVPacket *jpgPacket)
{
    int ret;

    *gotData = 0;

    if (jpgPacket) {
        ret = avcodec_send_packet(coolContext, jpgPacket);
        // In particular, we don't expect AVERROR(EAGAIN), because we read all
        // decoded frames with avcodec_receive_frame() until done.
        if (ret < 0)
            return ret == AVERROR_EOF ? 0 : ret;
    }

    ret = avcodec_receive_frame(coolContext, coolFrame);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return ret;
    if (ret >= 0)
        *gotData = 1;

    return 0;
}

// Used to encode a coolFrame, given a coolContext, coolFrame, and an int used as a bool
// found at https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
int encode(AVCodecContext *avctx, AVPacket *jpgPacket, AVFrame *coolFrame, int *got_packet)
{
    int ret;

    *got_packet = 0;
    ret = av_frame_make_writable(coolFrame);
    ret = avcodec_send_frame(avctx, coolFrame);

    ret = avcodec_receive_packet(avctx, jpgPacket);
    if (!ret)
        *got_packet = 1;
    if (ret == AVERROR(EAGAIN))
        return 0;

    return ret;
}
