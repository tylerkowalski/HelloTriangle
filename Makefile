CFLAGS = -std=c++17 -g

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# create list of all spv files and set as dependency
vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources)) 
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))

app : *.cpp *.hpp $(vertObjFiles) $(fragObjFiles)
	g++ $(CFLAGS) -o app *.cpp $(LDFLAGS)

#make shader targets
%.spv: %
	glslc $< -o $@

.PHONY: test clean

test: app
	./app

clean:
	rm -f app
	rm -f *.spv
