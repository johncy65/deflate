#ifndef GZIP_H
#define GZIP_H

#include <exception>

#include "Reader.hpp"
#include "Inflate.hpp"

class GZip {
public:
	class ReadException : public std::exception {
	public:
		ReadException(int position) {
			mPosition = position;
		}

		int position() { return mPosition; }

	private:
		int mPosition;
	};

	class InvalidFormatException : public std::exception {};

	GZip(Reader *reader) throw(ReadException, InvalidFormatException);

	int read(unsigned char *buffer, int length) throw(ReadException);
	bool empty();

private:
	Inflate *mInflate;	
};

#endif