#ifndef READER_H
#define READER_H

#include <fstream>
#include <exception>

class Reader {
public:
	class EndException : public std::exception
	{
	public:
		EndException(int position) {
			mPosition = position;
		}

		int position() { return mPosition; }

	private:
		int mPosition;
	};

	Reader();
	virtual ~Reader();

	unsigned int readBits(int num) throw(EndException);
	void readBytes(unsigned char *buffer, int length) throw(EndException);

	void byteSync();
	bool empty();
	int position();

protected:
	void setBuffer(const unsigned char *buffer, int length);

private:
	void updateCurrent() throw(EndException);

	virtual int refillBuffer() = 0;

	const unsigned char *mBuffer;
	int mLength;
	int mOffset;
	int mPos;
	unsigned char mCurrent;
	int mBit;
};

class BufferReader : public Reader {
public:
	BufferReader(const unsigned char *buffer, int length);

private:

	virtual int refillBuffer();
};

class FileReader : public Reader {
public:
	FileReader(std::ifstream &file);
	virtual ~FileReader();

private:
	virtual int refillBuffer();

	std::ifstream &mFile;
	unsigned char *mMutableBuffer;
};

#endif
