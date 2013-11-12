#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>


#define EXPORT_PATH "/sys/class/gpio/export"
#define UNEXPORT_PATH "/sys/class/gpio/unexport"

class ButtonMonitor {
public:
	ButtonMonitor();
	~ButtonMonitor();

	int add(int, int);
	int remove(int);
	int setTimeout(int);
	int polling();
	int pressed(int);
	void disableFisrtSkip(void);

private:
	int _searchGPIO(int gpio_no);
	int _exportGPIO(int gpio_no);
	int _unexportGPIO(int gpio_no);
	int _setEdge(int gpio_no, int edge);
	bool skipFirstTimeFlag;

	struct Button {
		int gpio_no;
		bool pressed;
		/* Edge setting:
		0: none
		1: falling
		2: rising
		*/
		int edge;
		int value_fd;
	};
	std::vector<struct Button> _button_list;
	std::vector<struct Button>::iterator _it;
	struct pollfd *_fdset;
	char _buf[10];
	int _timeout;

};







	

