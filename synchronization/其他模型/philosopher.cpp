semaphore chopstick[5] = {1};


void philosopher_i(int i)
{
	while(1)
	{
		// thinking

		p(chopstick[i]);
		p(chopstick[(i+4)%5]);
		// eating
		v(chopstick[i]);
		v(chopstick[(i+4)%5]);		
	}
}