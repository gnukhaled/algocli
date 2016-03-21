
#include "cmd-accessctrl.h"
#include "registry.h"

int accessctrl_ssh_enable(){

  int ret;
  ret = system("systemctl enable sshd.service &> /dev/null");
  if (!ret){
    ret = system("systemctl start sshd.service &> /dev/null");
    printf("SSH service enabled\n");
  }

  return ret;
}

int accessctrl_ssh_disable(){

  int ret;
  ret = system("systemctl stop sshd.service &> /dev/null");
  if (!ret){
    ret = system("systemctl disable sshd.service &> /dev/null");
    printf("SSH service disabled\n");
  }

  return ret;
}

int accessctrl_ftp_enable(){

  int ret;
  ret = system("systemctl enable vsftpd.service &> /dev/null");
  if (!ret) {
    ret = system("systemctl start vsftpd.service &> /dev/null");
    printf("FTP service enabled\n");
 }

  return ret;
}

int accessctrl_ftp_disable(){

  int ret;
  ret = system("systemctl stop vsftpd.service &> /dev/null");
  if (!ret){
    ret = system("systemctl disable vsftpd.service &> /dev/null");
    printf("FTP service disabled\n");
  }

  return ret;
}

int accessctrl_web_enable(){

  int ret;
  ret = system("systemctl enable httpd.service &> /dev/null");
  if (!ret){
    ret = system("systemctl start httpd.service &> /dev/null");
    printf("Web interface enabled\n");
  }

  return ret;
}

int accessctrl_web_disable(){

  int ret;
  ret = system("systemctl stop httpd.service &> /dev/null");
  if (!ret){
    ret = system("systemctl disable httpd.service &> /dev/null"); 
    printf("Web interface disabled\n");
  }

  return ret;
}

int ssh_set(int directive, char *val ){

	char **vals = calloc(1, sizeof(char*));
	vals[0] = val;
	int ret;
	char *table = ssh_config_tbl;
	char *col;


	switch(directive){
		case SSH_PORT:
			col = "Port";
			break;
		case SSH_LISTENADDR:
			col = "ListenAddress";
			break;
		case SSH_PROTO:
			col = "Protocol";
			break;
		default:
			return -1;
	}

	ret = registry(REG_UPDATE, table, col, vals);
	free(vals);
	return ret;

}


int ssh_add_host(char *host){

	char **vals = calloc(1,sizeof(char*));
	char *table = ssh_allowedhosts_tbl;
	char *col = "address";
	int ret;
	vals[0] = host;

	ret =  registry(REG_REPLACE, table, col, vals);
	free(vals);
	return ret;
}


int ssh_del_host(char *host){

	char **vals = calloc(1,sizeof(char*));
	char *table = ssh_allowedhosts_tbl;
	char *col = "address";
	int ret;
	vals[0] = host;

	ret = registry(REG_DELETE, table, col, vals);
	free(vals);
	return ret;
}
