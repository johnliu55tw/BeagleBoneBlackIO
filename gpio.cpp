#include "gpio.h"

GPIO::GPIO(int gpio_no)
{
	_direction = 0;
	_gpio_no = gpio_no;
	_value_fd = -1;
	high[0] = '1';
	high[1] = 0;
	low[0] = '0';
	low[1] = 0;
}
GPIO::~GPIO()
{
	free(_value_path);
	unexportGPIO();
}

int GPIO::exportGPIO()
{
	/*
	   Error return value:
	   -1 : unable to open EXPORT_PATH
	   -2 : unable to export the GPIO
	   -3 : unable to open PathString Path
	   -4 : unable to set the direction
	   -5 : unable to open GPIO value file descriptor
	   -6 : unable to open edge _gpio_fd
	   -7 : unable to set edge rising
        */

	char path[PATH_MAX];
	char GPIONum[4];
	int export_fd;
	int wr;

	// Export the pin
	export_fd = open(EXPORT_PATH, O_WRONLY);
	if (export_fd == -1)
		return -1;
	if (_gpio_no == -1)
		return -2;

	sprintf(GPIONum, "%d", _gpio_no);

Export:
	if ((wr = write(export_fd, GPIONum, strlen(GPIONum))) == -1) {
		if (errno == EBUSY) {
			this->unexportGPIO();
			goto Export;
		} else {
			return -2;
		}
	}
	close(export_fd);
}

int GPIO::unexportGPIO()
{
	int unexport_fd = open(UNEXPORT_PATH, O_WRONLY);
	if (unexport_fd == -1) {
		perror("open unexport");
		return -1;
	}
	int wr;
	char GPIONum[4];
	sprintf(GPIONum, "%d", _gpio_no);
	
	if ((wr = write(unexport_fd, GPIONum, strlen(GPIONum))) == -1) {
		perror("open unexport");
		return -1;
	}

	close(unexport_fd);
	return 0;
}

int GPIO::setDirection(int dir)
{
	char path[PATH_MAX];
	char state[4];
	int dir_fd;
	int wr;
	sprintf(path, "/sys/class/gpio/gpio%d/direction", _gpio_no);
	dir_fd = open(path, O_WRONLY);
	if (dir_fd == -1) 
		return -3;

	if (dir) { // Output
		strcpy(state, "out");
		wr = write(dir_fd, state, strlen(state));
		if (wr == -1) 
			return -4;
		close(dir_fd);
		_direction = 1;

	} else { // Input
		strcpy(state, "in");
		wr = write(dir_fd, state, strlen(state));
		if (wr == -1) 
			return -4;
		close(dir_fd);
	
		sprintf(path, "/sys/class/gpio/gpio%d/edge", _gpio_no);
		int edge_fd = open(path, O_WRONLY);
		if (edge_fd == -1) 
			return -6;

		wr = write(edge_fd, "both", 4);
		if (wr == -1) 
			return -7;
		close(edge_fd);
		_direction = 0;
	}
	sprintf(path, "/sys/class/gpio/gpio%d/value", _gpio_no);
	_value_path = (char*) malloc(strlen(path));
	strcpy(_value_path, path);
	_value_fd = open(_value_path, O_RDWR);
	if (_value_fd == -1)
		return -5;
	
	if (_direction)
		this->value(0);

	return 0;
}

int GPIO::value()
{
	char buf[10];

	close(_value_fd);
	_value_fd = open(_value_path, O_RDWR);
	int rd = read(_value_fd, buf, 10);
	if (rd > 0) {
		switch (buf[0]) {
		case '0':
			return 0;
		case '1':
			return 1;
		default:
			return -1;
		}
	} else {
		perror("Reading Input State");
		return -1;
	}
}

int GPIO::value(int level)
{
	if (_direction == 0)
		return -1;
	int wr;

	if (level)
		wr = write(_value_fd, high, 2);
	else
		wr = write(_value_fd, low, 2);

	return wr;
}

int GPIO::getFd()
{
	return _value_fd;
}
