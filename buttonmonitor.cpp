#include "buttonmonitor.h"

ButtonMonitor::ButtonMonitor()
{
	_timeout = -1; // Block forever
	_fdset = NULL;
	skipFirstTimeFlag = true;
}

ButtonMonitor::~ButtonMonitor()
{
	if (_fdset)
		free(_fdset);
}

int ButtonMonitor::add(int gpio_no, int edge)
{
	if (_searchGPIO(gpio_no) >= 0)
		return -1;
	if (_exportGPIO(gpio_no))
		return -2;
	if (_setEdge(gpio_no, edge))
		return -3;

	struct Button btn;
	btn.gpio_no = gpio_no;
	btn.pressed = false;
	btn.edge = edge;
	char path[PATH_MAX];
	sprintf(path, "/sys/class/gpio/gpio%d/value\0", gpio_no);
	btn.value_fd = open(path, O_RDONLY);
	if (btn.value_fd <= 0)
		return -4;

	_button_list.push_back(btn);
	return 0;
}

int ButtonMonitor::remove(int gpio_no)
{
	int index;
	if ((index = _searchGPIO(gpio_no)) < 0)
		return -1;

	_button_list.erase(_button_list.begin() + index);
	return 0;
}

int ButtonMonitor::setTimeout(int timeout)
{
	_timeout = timeout;
	return _timeout;
}

int ButtonMonitor::polling()
{
	_fdset = (struct pollfd*)calloc(sizeof(struct pollfd), _button_list.size());
	int i = 0;
	for (_it=_button_list.begin(); _it!=_button_list.end(); _it++) {
		_fdset[i].fd = _it->value_fd;
		_fdset[i].events = POLLPRI;
		i++;
	}

	i = poll(_fdset, _button_list.size(), _timeout);
	// Error Occurred
	if (i < 0) {
		return -1;
	}

	// Timeout Occurred
	if (i == 0) {
		for (_it=_button_list.begin(); _it!=_button_list.end(); _it++) {
			_it->pressed = false;
		}
		return 0;
	}

	i = 0;
	int pressed_counter = 0;
	for (_it=_button_list.begin(); _it!=_button_list.end(); _it++) {
		if (_fdset[i].revents & POLLPRI) {
			read(_fdset[i].fd, _buf, 10);
			if (!skipFirstTimeFlag) {
				_it->pressed = true;
				pressed_counter++;
			}
		} else {
			_it->pressed = false;
		}
		i++;
	}

	if (skipFirstTimeFlag)
		skipFirstTimeFlag = false;

	return pressed_counter;
}

int ButtonMonitor::pressed(int gpio_no)
{
	for (_it=_button_list.begin(); _it!=_button_list.end(); _it++) {
		if (_it->gpio_no == gpio_no) {
			if (_it->pressed)
				return 1;
			else
				return 0;
		}
	}
	return -1;
}

void ButtonMonitor::disableFisrtSkip(void)
{
	skipFirstTimeFlag = false;
}

int ButtonMonitor::_searchGPIO(int gpio_no)
{
	int index = 0;
	for(_it=_button_list.begin(); _it!=_button_list.end(); _it++) {
		if (_it->gpio_no == gpio_no)
			return index;
		else
			index++;
	}
	return -1;
}

int ButtonMonitor::_exportGPIO(int gpio_no)
{
	char GPIONum[3];
	sprintf(GPIONum, "%d", gpio_no);
	int wr;
	int export_fd = open(EXPORT_PATH, O_WRONLY);
	if (export_fd <= 0)
		return -1;

Export:
	if ((wr = write(export_fd, GPIONum, strlen(GPIONum))) == -1) {
		if (errno == EBUSY) {
			if (_unexportGPIO(gpio_no))
				return -2;
			goto Export;
		} else {
			return -2;
		}
	}
	close(export_fd);
	return 0;
}

int ButtonMonitor::_unexportGPIO(int gpio_no)
{
	int unexport_fd = open(UNEXPORT_PATH, O_WRONLY);
	if (unexport_fd == -1) {
		perror("open unexport");
		return -1;
	}
	int wr;
	char GPIONum[3];
	sprintf(GPIONum, "%d", gpio_no);
	
	if ((wr = write(unexport_fd, GPIONum, strlen(GPIONum))) == -1) {
		perror("open unexport");
		return -1;
	}

	close(unexport_fd);
	return 0;
}

int ButtonMonitor::_setEdge(int gpio_no, int edge)
{
	char path[PATH_MAX];
	char edgeName[8];
	int edge_fd;
	int wr;

	sprintf(path, "/sys/class/gpio/gpio%d/edge\0", gpio_no);
	if ((edge_fd = open(path, O_WRONLY)) <= 0) 
		return -1;
	switch(edge) {
	case 0:
		strcpy(edgeName, "none\0");
		wr = write(edge_fd, edgeName, strlen(edgeName));
		break;
	case 1:
		strcpy(edgeName, "falling\0");
		wr = write(edge_fd, edgeName, strlen(edgeName));
		break;
	case 2:
		strcpy(edgeName, "rising\0");
		wr = write(edge_fd, edgeName, strlen(edgeName));
		break;
	default:
		return -2;
	}

	if (wr <= 0)
		return -3;
	close(edge_fd);
	return 0;
}
