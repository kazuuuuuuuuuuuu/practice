semaphore customer = 0;
semaphore barber = 2;
semaphore mutex = 1;
int wait = 0; 
const int MAX_WAIT = 3;// Maximum waiting queue length

void barber()
{
	p(customer);
	p(mutex);
	wait --;
	v(mutex);

	// cut hair

	v(barber);
}

void customer()
{
	p(mutex);
	if(wait<MAX_WAIT)
	{

		wait ++;
		v(mutex);

		p(barber);
		v(customer);

		//get haircut
	}
	else
	{
		v(mutex);
		// leave
	}
}
