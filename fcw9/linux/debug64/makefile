%.o : ../../source/%.cpp
	g++ -D_SYS_LINUX -D_OS64 -D_DEBUG -D_REENTRANT -D_USINGFWKSTATICLIB -traditional-cpp -m64 -c -g \
	-I../../../../fwkc/sdk -I../../../../fwkw/sdk -o $@ $<

include ../libobjs

fcw9: $(LIBOBJS)
	g++ -o fcw9 -D_OS64 -D_SYS_LINUX -D_DEBUG -D_REENTRANT -D_USINGFWKSTATICLIB -traditional-cpp -m64 -g $(LIBOBJS) \
		-L../../../../libs/linux/Sd64 \
		-lfwkw -lfwkc -lcups -lfreetype -lpng -ltiff -ljpeg -lzlib -lX11 -lXext -lXmu \
		-lrt -lpthread -ldl -lc

