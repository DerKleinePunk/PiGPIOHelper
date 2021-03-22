#pragma once

#include <exception>
#include <string>

class ConfigErrorException: public std::exception
{
public:
	explicit ConfigErrorException(const char* message);
	explicit ConfigErrorException(const std::string& message);
	virtual ~ConfigErrorException() throw();

	const char* what() const throw() override
	{
		return mMessage.c_str();
	}

protected:
	std::string mMessage;

private:
	ConfigErrorException();
};
