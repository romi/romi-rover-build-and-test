
romi_build_and_test_dir=../..

libr_incl=${romi_build_and_test_dir}/libr/include
libromi_incl=${romi_build_and_test_dir}/libromi/include

CXX_FLAGS=-O3 -g -Wall

all: elastic elastic_pp  
#weeder_eval

weeder_eval:
	g++ ${CXX_FLAGS} \
		ImageCropperFactory.cpp \
		ImageSegmentationFactory.cpp \
		PathPlannerFactory.cpp \
		ImageCropper.cpp \
		svm/SVMSegmentation.cpp \
		quincunx/Quincunx.cpp \
		quincunx/point.c \
		som/SOM.cpp \
		som/Superpixels.cpp \
		som/slic.cpp \
		som/fixed.c \
		Pipeline.cpp \
		DebugWeedingSession.cpp \
		weeder_eval.cpp \
		-lasan -lromi -lr -lm \
		-I ${libromi_incl} -I ${libr_incl} \
		-L${romi_build_and_test_dir}/build/lib \
		-o weeder_eval

elastic:
	gcc -O3 elastic.c dump.c test.c print.c -lm -o elastic

elastic_pp:
	g++ -O3 -g elastic.cpp som/fixed.c DebugWeedingSession.cpp test.c \
		-lasan -lromi -lr -lm \
		-I ${libromi_incl} -I ${libr_incl} \
		-L${romi_build_and_test_dir}/build/lib \
		-o elastic++

clean:
	rm -f elastic++ elastic weeder_eval

