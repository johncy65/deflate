#include <iostream>
#include <fstream>

#include "Reader.hpp"
#include "GZip.hpp"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char *argv[])
{
	if(argc < 2) {
		cerr << "No filename specified" << endl;
		exit(1);
	}

	const char *filename = argv[1];
	std::ifstream file(filename, std::ios_base::in | std::ios_base::binary);
	if(file.fail()) {
		cerr << "Error opening file " << filename << endl;
		exit(1);
	}

	FileReader reader(file);

	try {
		GZip gzip(reader);

		static const int BufferSize = 4096;
		unsigned char *buffer = new unsigned char[BufferSize];
		while(1) {
			int bytesRead = gzip.read(buffer, BufferSize);

			if(bytesRead == 0) {
				break;
			}

			cout.write((char*)buffer, bytesRead);
		}
	} catch(GZip::InvalidFormatException e) {
		cerr << "File " << filename << " is not a valid GZip file" << endl;
	} catch(GZip::ReadException e) {
		cerr << "*** Read error at position " << e.position() << " ***" << endl;
	}

	return 0;
}
