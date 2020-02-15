#include "types.h"
#include "stat.h"
#include "user.h"

int
main()
{
	sh_var_write(0);

  int id = sem_create(1);	
	int pid = fork();
		
	for(int i = 0; i < 10000; i++) {
		sem_p(id);
		sh_var_write(sh_var_read()+1);
		sem_v(id);
	}
	if(pid > 0){ 
		wait();
		sem_free(id);
	}
	printf(1,"sum = %d\n", sh_var_read());
	exit();
}
