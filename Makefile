#Makefile for lab7
CXX           = $(shell fltk-config --cxx)
CXXFLAGS      = $(shell fltk-config --cxxflags)
LINKFLTK_FLGS = $(shell fltk-config --ldflags --use-gl --use-images)
POSTBUILD     = fltk-config --post #build .app for osx. (does nothing on pc)

all: l7

%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS)

l7:  main.o MyGLCanvas.o ppm.o SceneObject.o 
	$(CXX) -o $@ main.o MyGLCanvas.o ppm.o SceneObject.o $(LINKFLTK_FLGS) && $(POSTBUILD) $@ && if test -d $@.app; then sed -i '' -e 's|<true/>|<false/>|' "$@.app/Contents/Info.plist"; fi

clean:
	@rm -rf l7.app l7 *.o *~
