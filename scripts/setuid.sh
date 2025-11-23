#!/bin/bash

# This script sets the setuid bit for the algo shell 
# For privileged commands execution 

sudo chown root:wheel algocli
sudo chmod u+s algocli

