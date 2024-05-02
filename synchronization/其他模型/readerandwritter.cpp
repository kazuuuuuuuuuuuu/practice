semaphore rw = 1;
int reader_count = 0;
semaphore r_mutex = 1;

void reader_i()
{
	p(r_mutex);
	if(reader_count==0)
		p(rw);
	reader_count ++;
	v(r_mutex);

	// read

	p(r_mutex);
	reader_count --;
	if(reader_count==0)
		v(rw);
	v(r_mutex);	
}

void writter_i()
{
	p(rw);

	// write

	v(rw);
}