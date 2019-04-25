all: bouncer.cpp
	g++ bouncer.cpp
clean: 
	rm *.0 *.cool *.mp4
movie: Output/image001.cool
	ffmpeg -i image-%02d.cool video_name.mp4



