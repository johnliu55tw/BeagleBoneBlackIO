#include "gpio.h"

int main(void)
{
	GPIO out(66);
	out.exportGPIO();
	out.setDirection(1);
	
	GPIO button(67);
	button.exportGPIO();
	button.setDirection(0);

	while(1) {
		printf("Read value\n");
		if (button.value() == 1)
			printf("High\n");
		else
			printf("low\n");

		usleep(10000);
	}

	return 0;
}
