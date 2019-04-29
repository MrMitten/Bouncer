all: bouncer.cpp
	g++ bouncer.cpp -I..//ffmpeg/include -L..//ffmpeg/lib -lva-x11 -lva-drm -lbz2 -lX11 -lva -lswscale -lavdevice -lavformat -lavcodec -lavutil -lswresample -lm -lpthread -lz
clean: 
	rm *.o *.out *.cool *.mp4
movie: frame000.cool
	ffmpeg -i frame%03d.cool movie.mp4
play:
	ffplay movie.mp4



