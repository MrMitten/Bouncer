all: bouncer.cpp
	g++ bouncer.cpp -I..//ffmpeg/include -L..//ffmpeg/lib -lva-x11 -lva-drm -lbz2 -lX11 -lva -lswscale -lavdevice -lavformat -lavcodec -lavutil -lswresample -lm -lpthread -lz
clean: 
	rm *.0 *.cool *.mp4
movie: Output/image-001.cool
	ffmpeg -i image-%02d.cool video_name.mp4



