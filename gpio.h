#ifndef GPIO_H
#define GPIO_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define EXPORT_PATH	"/sys/class/gpio/export"
#define UNEXPORT_PATH	"/sys/class/gpio/unexport"

class GPIO {
public:
	GPIO(int);
	~GPIO();
	
	int exportGPIO();
	int unexportGPIO();

	int setDirection(int);
	int value();
	int value(int);
	int enableMutex();
	int disableMutex();

protected:
	int getFd();

private:
	int _direction;
	int _gpio_no;
	char high[2];
	char low[2];
	char *_value_path;
	int _value_fd;

};

#endif
