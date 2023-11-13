#include <stdio.h>
#include <Windows.h>

int main(int argc, char const *argv[])
{

	char buff[256];
	scanf("%s", buff);
	
	char company[64] = {};
	char email[64] = {};
	char occupation[64] = {};
	
	int index = 0;
	int j = 0;
	bool start = false;
	
	for(int i=0;i<strlen(buff);i++)
	{
		if(buff[i]=='&')
		{
			index ++;
			j = 0;
			start = false;
			continue;
		}
		if(index==0)
		{
			if(buff[i]=='=')
			{
				start = true;
				continue;
			}
			if(start == true)
			{
				company[j] = buff[i];
				j ++;
			}
		}
		else if(index==1)
		{
			if(buff[i]=='=')
			{
				start = true;
				continue;
			}
			if(start == true)
			{
				email[j] = buff[i];
				j ++;
			}
		}
		else
		{
			if(buff[i]=='=')
			{
				start = true;
				continue;
			}
			if(start == true)
			{
				occupation[j] = buff[i];
				j ++;
			}
		}
	}
	//printf("company:%s\n", company);
	//printf("email:%s\n", email);
	//printf("occupation:%s\n", occupation);
	//printf("form:%s\n", buff);

	char cgi_out[4096] = {};
	char line[1024];	
	
	strcat(cgi_out, "<html>");
	strcat(cgi_out, "<head>");
	strcat(cgi_out, "<title>thanks for your time</title>");
	strcat(cgi_out, "</head>");
	strcat(cgi_out,"<body>");
	strcat(cgi_out,"<h1>your info has been recored</h1>");
	
	sprintf(line, "<h2>company = %s</h2>", company);
	strcat(cgi_out, line);	
	sprintf(line, "<h2>email = %s</h2>", email);
	strcat(cgi_out, line);
	sprintf(line, "<h2>occupation = %s</h2>", occupation);
	strcat(cgi_out, line);	
	strcat(cgi_out,"<img src=\"404.png\"/>");
	


	strcat(cgi_out, "</body>");
	strcat(cgi_out, "</html>");
	
	printf("%s", cgi_out);	
	return 0;
}
