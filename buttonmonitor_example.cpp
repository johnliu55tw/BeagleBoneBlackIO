#include "buttonmonitor.h"

int main(void)
{
	ButtonMonitor monitor;
	int btn1 = 66;
	int btn2 = 67;
	int ret;
	ret = monitor.setTimeout(500);
	ret = monitor.add(btn1, 1);
	ret = monitor.add(btn2, 1);

	while(1) {
		monitor.polling();
		if (monitor.pressed(66))
			printf("gpio 66 pressed!\n");
		if (monitor.pressed(67))
			printf("gpio 67 pressed!\n");
		printf("timeout\n");
	}
	return 0;
}
