CREATE TABLE backup_points(
name   TEXT,
createdon   TEXT);


CREATE TABLE nfs_shares (
bpname  TEXT,
host    TEXT,
params  TEXT,
PRIMARY KEY (bpname,host));


CREATE TABLE ssh_config (
Port    INT,
ListenAddress  TEXT,
Protocol   TEXT,
SyslogFacility    TEXT,
PasswordAuthentication   TEXT,
PubkeyAuthentication   TEXT,
MaxAuthTries   INT);


CREATE TABLE ssh_allowedhosts (
address TEXT PRIMARY KEY);

