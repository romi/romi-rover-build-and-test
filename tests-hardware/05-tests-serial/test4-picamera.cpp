#include <unistd.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <raspicam/raspicam.h>
#include <raspicam/raspicam_still.h>
using namespace std;
 
int main()
{
	raspicam::RaspiCam_Still camera;

        camera.setEncoding(raspicam::RASPICAM_ENCODING_JPEG);
        camera.setWidth(1920);
        camera.setHeight(1080);
        
	cout << "Opening camera..." << endl;
	if (!camera.open()) {
                cerr << "Error opening camera" << endl;
                return -1;
        }

	cout<<"Sleeping for 3 secs"<<endl;
	sleep(3);

        size_t length = camera.getImageBufferSize();
	unsigned char *data = new unsigned char[length];
        
	camera.grab_retrieve(data, length);
        
	//save
	std::ofstream outFile("raspicam_image.jpg", std::ios::binary);
        
	outFile << "P6\n" << camera.getWidth() << " " << camera.getHeight() << " 255\n";
	outFile.write((char*) data, length);
        
	cout << "Image saved at raspicam_image.ppm" << endl;

	delete data;
	return 0;
}


 
int main_2()
{
	raspicam::RaspiCam camera;

        camera.setFormat(raspicam::RASPICAM_FORMAT_BGR);
        camera.setWidth(1920);
        camera.setHeight(1080);
        
	cout << "Opening camera..." << endl;
	if (!camera.open()) {
                cerr << "Error opening camera" << endl;
                return -1;
        }

	cout<<"Sleeping for 3 secs"<<endl;
	sleep(3);
        
	camera.grab();
        
	unsigned char *data = new unsigned char[camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB)];
        
	camera.retrieve(data,raspicam::RASPICAM_FORMAT_RGB);
        
	//save
	std::ofstream outFile("raspicam_image.ppm", std::ios::binary);
        
	outFile << "P6\n" << camera.getWidth() << " " << camera.getHeight() << " 255\n";
	outFile.write((char*) data, camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB));
        
	cout << "Image saved at raspicam_image.ppm" << endl;

	delete data;
	return 0;
}
