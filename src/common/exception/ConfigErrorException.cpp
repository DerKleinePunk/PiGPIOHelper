#include "ConfigErrorException.hpp"
#include "../easylogging/easylogging++.h"

ConfigErrorException::ConfigErrorException() {
}

ConfigErrorException::ConfigErrorException(const char* message) {
	mMessage = message;
	LOG(ERROR) << "ConfigErrorException " << message;
}

ConfigErrorException::ConfigErrorException(const std::string& message) {
	mMessage = message;
	LOG(ERROR) << "ConfigErrorException " << message;
}


ConfigErrorException::~ConfigErrorException() throw() {
}
