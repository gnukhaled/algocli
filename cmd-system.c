
#include "cmd-system.h"

int algo_reboot(){

  char choice = 'N';
  printf("\n\tConfirm system reboot [y,N]: ");
  scanf("%c", &choice);

  if ((char)choice == 'N' || (char)choice == 'n'){
	printf("System reboot cancelled by user\n");
	return 0;
  }else if((char)choice == 'y' || (char)choice == 'Y'){
	printf("\nRebooting system\n");
	sync();
	fflush(stdout);
	return reboot(RB_AUTOBOOT);
  }

  return 0;
}


int algo_shutdown(){

  char choice = 'N';
  printf("\n\tConfirm system shutdown [y,N]: ");
  scanf("%c", &choice);

  if ((char)choice == 'N' || (char)choice == 'n'){
        printf("System shutdown cancelled by user\n");
        return 0;
  }else if((char)choice == 'y' || (char)choice == 'Y'){
        printf("\nShutting down the system\n");
        sync();
        fflush(stdout);
        return reboot(RB_POWER_OFF);
  }

  return 0;
}

int system_set_motd(){

  int ret;
  return  system("rvim -n --noplugin /etc/motd");
}
