#pragma once

namespace cat {

class IFileProvider
{
public:
	virtual ~IFileProvider() {}

	virtual void*		open		(const char* const filename) = 0;
	virtual int			read		(void* data, const int len, void* handle) = 0;
	//virtual void		write		(const void* data, const int len, void* handle) = 0;
	virtual const void* getBuffer	(void* handle) const = 0; // mmap a buffer to the file
	virtual int			getSize		(void* handle) const = 0; // get file size in byte
	virtual bool		exists		(const char* const filename) = 0;
	virtual void		close		(void* handle) = 0;
};

} //namespace ui
